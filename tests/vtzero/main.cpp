#include "VkvgApp.hpp"

#include "argparse/argparse.hpp"

#include <sqlite3.h>
#include <vtzero/vector_tile.hpp>
#include <vector>
#include <stdexcept>
#include <zlib.h>
#include <string>
#include <iostream>

#include "icon_registry.h"

constexpr uint32_t fnv32(const char* str) {
    uint32_t hash = 2166136261U;
    while (*str) {
        hash ^= static_cast<uint32_t>(*str++);
        hash *= 16777619U;
    }
    return hash;
}

#define CreateRgbaf(r, g, b, a)                                                             \
((((uint32_t)(a * 255.0f) & 0xFF) << 24) | (((uint32_t)(b * a * 255.0f) & 0xFF) << 16) |    \
 (((uint32_t)(g * a * 255.0f) & 0xFF) << 8) | ((uint32_t)(r * a * 255.0f) & 0xFF))
/*#define CreateArgb(a, r, g, b)                                                              \
        ((0x##a & 0xFF) << 24) | ((((0x##b * 0x##a) >> 8) & 0xFF) << 16) |                  \
        ((((0x##g * 0x##a) >> 8) & 0xFF) << 8) | ((((0x##r * 0x##a) >> 8) & 0xFF))*/
#define CreateAbgr(a, r, g, b) \
    (((uint32_t)(a) & 0xFF) << 24) | \
    (((uint32_t)PREMULTIPLY(b, a) & 0xFF) << 16) | \
    (((uint32_t)PREMULTIPLY(g, a) & 0xFF) << 8)  | \
    ((uint32_t)PREMULTIPLY(r, a) & 0xFF)
static std::pair<int, int> latlon_to_mbtiles(float lon, float lat, int zoom) {
    // 1. Calculate Tile Column (X) - identical for XYZ and TMS
    int tile_column = static_cast<int>(std::floor((lon + 180.0f) / 360.0f * std::pow(2.0f, zoom)));
    float lat_rad = lat * M_PI / 180.0f;
    int tile_row = static_cast<int>(std::floor((1.0f - std::log(std::tan(lat_rad) + (1.0f / std::cos(lat_rad))) / M_PI) / 2.0f * std::pow(2.0f, zoom)));
    return {tile_column, tile_row};
}
static inline uint32_t _createAbgr(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t prod_b = (uint32_t)b * a + 128;
    uint32_t prod_g = (uint32_t)g * a + 128;
    uint32_t prod_r = (uint32_t)r * a + 128;

    return ((uint32_t)a << 24) |
           (((prod_b + (prod_b >> 8)) >> 8) << 16) |
           (((prod_g + (prod_g >> 8)) >> 8) << 8)  |
           ((prod_r + (prod_r >> 8)) >> 8);
}
#define CreateAbgr(a, r, g, b) _createAbgr(0x##a, 0x##r, 0x##g, 0x##b)
typedef struct point_t {
    float x;
    float y;
    point_t() = default;
    constexpr point_t(float x_, float y_) noexcept :
       x(x_),
       y(y_) { }
} Point;

struct RenderPolygon {
    std::vector<Point> rings; // Supports outer and inner hole rings
    float r, g, b;                                           // Pre-resolved colors
    int tx, ty;
};
struct TrackedLine {
    std::vector<Point> points;
    float width;
    float r, g, b;
    int tx, ty;
};
// The absolute correct polygon geometry template interface required by vtzero
struct hndGeometryDrawer {
    VkvgContext ctx;
    bool firstPoint;

    Point point;

    void ring_begin(uint32_t count) {
        firstPoint = true;
    }
    void ring_point(const vtzero::point p) {
        if (firstPoint) {
            vkvg_move_to(ctx, p.x, p.y);
            firstPoint = false;
        } else
            vkvg_line_to(ctx, p.x, p.y);
    }
    void ring_end(vtzero::ring_type type) {
        vkvg_close_path(ctx);
    }
    void linestring_begin(uint32_t count) {}
    void linestring_point(const vtzero::point p) {
        // We look at the internal vkvg path state to decide if we move or draw a segment line
        // vtzero tracks winding commands automatically under the hood
        static bool first_point_of_segment = true;

        if (first_point_of_segment) {
            vkvg_move_to(ctx, static_cast<float>(p.x), static_cast<float>(p.y));
            first_point_of_segment = false;
        } else {
            vkvg_line_to(ctx, static_cast<float>(p.x), static_cast<float>(p.y));
        }
    }
    void linestring_end() {}
    void points_begin(uint32_t count) {}
    void points_point(const vtzero::point p) {
        point = Point(static_cast<float>(p.x), static_cast<float>(p.y));
    }
    void points_end() {}
};
struct hndGeometryRecorder {
    bool firstPoint;

    Point point;
    std::vector<std::vector<Point>> lines;
    std::vector<std::vector<Point>> rings;

    std::vector<Point> points;

    void ring_begin(uint32_t count) {
        points.reserve(count);
    }
    void ring_point(const vtzero::point p) {
        points.push_back(Point{(float)p.x, (float)p.y});
    }
    void ring_end(vtzero::ring_type type) {
        rings.push_back(points);
    }
    void linestring_begin(uint32_t count) {
        points.reserve(count);
    }
    void linestring_point(const vtzero::point p) {
        points.push_back(Point{(float)p.x, (float)p.y});
    }
    void linestring_end() {
        lines.push_back(points);
    }
    void points_begin(uint32_t count) {}
    void points_point(const vtzero::point p) {
        point = Point(static_cast<float>(p.x), static_cast<float>(p.y));
    }
    void points_end() {}
};
struct Place {
    Point loc;
    std::string name;
};
using PoiIconMap = std::unordered_map<const SvgIcon*, std::vector<Place>>;

struct CachedTileSurface {
    VkvgSurface surface = nullptr; // The pre-rendered hardware texture layer
    int tx, ty;                    // Absolute tile coordinates
    bool fully_rendered = false;
    std::vector<Place> TownsCities;
    std::vector<Place> Vilages;
    PoiIconMap pois_by_icon;
};
class vtzeroApp : public VkvgApp {
    sqlite3* mbtiles_db;
    std::vector<CachedTileSurface> tile_surface_cache;

    static const uint32_t tileSize = 4096;
    static constexpr float tileSizeRatio = (float)4096 / (float)tileSize;

    int tile_x = 8388;
    int tile_y = 5495;
    int tile_z = 12;

    std::vector<RenderPolygon> background_landuse; // Forests, woods, parks, etc.
    std::vector<RenderPolygon> water_bodies;       // Lakes, canals, rivers
    std::vector<TrackedLine> road_network;

    std::vector<std::pair<int, int>> loaded_tiles;
    IconRegistry *icons;

public:
    VkvgFont font;
    VkvgFont fontPlace;
    VkvgFont fontPlaceM;
    VkvgFont fontPlaceS;
    VkvgFont fontRoads;
    void Init ()  override {
        VkvgApp::Init();
        icons = new IconRegistry(device);

        font = vkvg_font_create(device, "sans:bold", 20);
        fontPlace = vkvg_font_create(device, "sans:bold", 40);
        fontPlaceM = vkvg_font_create(device, "sans:bold", 30);
        fontPlaceS = vkvg_font_create(device, "sans:bold", 20);
        fontRoads = vkvg_font_create(device, "sans", 12);

        //if (sqlite3_open("/mnt/data/osm/bruxelles.mbtiles", &mbtiles_db) != SQLITE_OK) {
        if (sqlite3_open("/mnt/data/osm/ciney.mbtiles", &mbtiles_db) != SQLITE_OK) {
            std::cerr << "Failed to open map database file.\n";
            return;
        }

        offsetX = tileSize * 0.5;
        offsetY = tileSize * 0.5;

        Point center = fetch_center_from_mbtiles(mbtiles_db);
        std::pair<int, int> t = latlon_to_mbtiles(center.x, center.y, tile_z);

        tile_x = t.first;
        tile_y = t.second;

        UpdateVisibleTiles();
    }
    void CleanUp () override {
        for (auto& tile : tile_surface_cache) {
            if (tile.surface) vkvg_surface_destroy(tile.surface);
        }
        sqlite3_close(mbtiles_db);

        vkvg_font_destroy(font);
        vkvg_font_destroy(fontPlace);
        vkvg_font_destroy(fontPlaceM);
        vkvg_font_destroy(fontPlaceS);
        vkvg_font_destroy(fontRoads);

        delete(icons);

        VkvgApp::CleanUp();
    }
    void LoadTileSurfaceIntoCache(int z, int tx, int ty) {
        for (const auto& tile : tile_surface_cache) {
            if (tile.tx == tx && tile.ty == ty) return; // Already baked
        }
        CachedTileSurface new_tile;
        BakeTileToSurface(mbtiles_db, new_tile, z, tx, ty, device);
        tile_surface_cache.push_back(std::move(new_tile));
    }
    void UpdateVisibleTiles() {
        float localTileSize = static_cast<float>(tileSize);

        double screenLeft = -offsetX / zoom;
        double screenTop  = -offsetY / zoom;
        double screenRight  = (static_cast<double>(width) - offsetX) / zoom;
        double screenBottom = (static_cast<double>(height) - offsetY) / zoom;

        int min_tx = tile_x + static_cast<int>(floor(screenLeft / localTileSize));
        int max_tx = tile_x + static_cast<int>(floor(screenRight / localTileSize));

               // TOP-LEFT FIX: Screen-top means LOWER row indices, Screen-bottom means HIGHER row indices
        int min_ty = tile_y + static_cast<int>(floor(screenTop / localTileSize));
        int max_ty = tile_y + static_cast<int>(floor(screenBottom / localTileSize));

               // Pruning loop remains unchanged
        for (auto it = tile_surface_cache.begin(); it != tile_surface_cache.end(); ) {
            if (it->tx < min_tx || it->tx > max_tx || it->ty < min_ty || it->ty > max_ty) {
                if (it->surface) vkvg_surface_destroy(it->surface);
                it = tile_surface_cache.erase(it);
            } else {
                ++it;
            }
        }

               // Ingestion loop blocks remain sequential
        for (int tx = min_tx; tx <= max_tx; ++tx) {
            for (int ty = min_ty; ty <= max_ty; ++ty) {
                LoadTileSurfaceIntoCache(tile_z, tx, ty);
            }
        }
#if VKVG_DBG_STATS
        vkvg_debug_stats_t dbg = vkvg_device_get_stats(device);
        std::cout << "==== Debug statistics ====" << std::endl;
        std::cout << "sizePoints: " << dbg.sizePoints << std::endl;
        std::cout << "sizePathes: " << dbg.sizePathes << std::endl;
        std::cout << "sizeVertices: " << dbg.sizeVertices << std::endl;
        std::cout << "sizeIndices: " << dbg.sizeIndices << std::endl;
        std::cout << "sizeVBO: " << dbg.sizeVBO << std::endl;
        std::cout << "sizeIbo: " << dbg.sizeIBO << std::endl;
        std::cout << "==========================" << std::endl;
#endif
    }

    void BakeTileToSurface(sqlite3* mbtiles_db, CachedTileSurface& cache, int z, int tx, int ty, VkvgDevice device) {
        std::cout << "bake tile " << tx << ", " << ty << ", " << z << ", " << std::endl;
        cache.surface = vkvg_surface_create_ms(device, tileSize, tileSize, VK_SAMPLE_COUNT_8_BIT);
        cache.tx = tx;
        cache.ty = ty;

        VkvgContext ctx = vkvg_create(cache.surface);
        vkvg_set_source_rgba(ctx, 0.0f, 0.0f, 0.0f, 0.0f);
        vkvg_paint(ctx);

        vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);
        //vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
        vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

        // Scale down the internal MVT 0-4096 coordinate system natively inside the cache layout
        vkvg_scale(ctx, tileSizeRatio, tileSizeRatio);

        std::vector<char> compressed_blob = fetch_tile_from_mbtiles(mbtiles_db, z, tx, ty);
        if (compressed_blob.empty()) {
            vkvg_destroy(ctx);
            cache.fully_rendered = true;
            return;
        }

        std::vector<char> raw_mvt = decompress_gzip(compressed_blob);
        vtzero::vector_tile tile{raw_mvt.data(), raw_mvt.size()};

        while (auto layer = tile.next_layer()) {
            std::cout << "Layer Name: " << layer.name().to_string() << "\n";
            if (layer.name() != "poi")
                continue;


        }/*
        if (auto boundary = tile.get_layer_by_name("boundary")) {
            while (auto feature = boundary.next_feature()) {
                float r = 0.68f, g = 0.82f, b = 0.65f; // Pale forest green
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    switch (property.value().type()) {
                    case vtzero::property_value_type::string_value:
                        std::cout << "\t\tproperty:" << prop_name << " = " << property.value().string_value().to_string() << std::endl;
                        break;
                    default:
                        std::cout << "\t\tproperty:" << prop_name << " = ?" << std::endl;
                        break;
                    }
                }
            }
        }
        vkvg_flush(ctx);*/
        if (auto landuse = tile.get_layer_by_name("landuse")) {
            //std::cout << "landuse" << std::endl;
            while (auto feature = landuse.next_feature()) {
                uint32_t c = 0xffb0cc9a;
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    std::string prop_val{property.value().string_value()};
                    if (prop_name == "class") {
                        if (prop_val == "residential")
                            c = CreateAbgr (ff,cc,b1,8b);
                        else if (prop_val == "school" || prop_val == "college" || prop_val == "kindergarten" || prop_val == "university" || prop_val == "library")
                            c = CreateAbgr (ff,e3,9c,cf);
                        else if (prop_val == "hospital")
                            c = CreateAbgr (ff,df,9e,a5);
                        else if (prop_val == "commercial" || prop_val == "retail")
                            c = CreateAbgr (ff,ff,e2,85);
                        else if (prop_val == "track" || prop_val == "pitch")
                            c = CreateAbgr (ff,dd,dd,dd);
                        else if (prop_val == "playground" || prop_val == "stadium")
                            c = CreateAbgr (ff,8b,cc,b3);
                        else if (prop_val == "industrial" || prop_val == "railway")
                            c = CreateAbgr (ff,b7,b8,cc);
                        else if (prop_val == "cemetery")
                            c = CreateAbgr (ff,8a,cb,94);
                        else if (prop_val == "neighbourhood" || prop_val == "garages")
                            c = CreateAbgr (ff,00,00,ff);
                        else if (prop_val == "military")
                            c = CreateAbgr (ff,b7,b8,cc);
                        else if (prop_val == "zoo")
                            c = CreateAbgr (ff,cc,8b,bf);
                        /*else
                            std::cout << "\t\tproperty:" << prop_name << " = " << prop_val << std::endl;*/
                    }
                }
                vkvg_set_source_color(ctx, c);

                hndGeometryDrawer handler{ctx};
                vtzero::decode_geometry(feature.geometry(), handler);
                vkvg_fill(ctx);
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("landcover")) {
            //std::cout << "landcover" << std::endl;
            uint32_t c = 0xff0000ff;
            while (auto feature = layer.next_feature()) {
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    std::string prop_val{property.value().string_value()};
                    if (prop_name == "class") {
                        if (prop_val == "grass")
                            c = CreateAbgr (ff,9c,e6,9c);
                        else if (prop_val == "farmland")
                            c = CreateAbgr (ff,ee,f0,d6);
                        else if (prop_val == "wetland")
                            c = CreateAbgr (ff,96,f9,92);
                        else if (prop_val == "wood")
                            c = CreateAbgr (ff,8d,c5,6c);
                        else if (prop_val == "military")
                            c = CreateAbgr (ff,b7,b8,cc);
                        /*else
                            std::cout << "\t\tproperty:" << prop_name << " = " << prop_val << std::endl;*/
                    }
                }
                vkvg_set_source_color(ctx, c);
                hndGeometryDrawer handler{ctx};
                vtzero::decode_geometry(feature.geometry(), handler);
                vkvg_fill(ctx);
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("water")) {
            uint32_t c = 0xffb0cc9a;
            while (auto feature = layer.next_feature()) {
                vkvg_set_source_rgb(ctx, 0.71f, 0.83f, 0.91f); // Map Blue
                hndGeometryDrawer handler{ctx};
                vtzero::decode_polygon_geometry(feature.geometry(), handler);
                vkvg_fill(ctx);
            }
        }
        vkvg_flush(ctx);

        if (auto layer = tile.get_layer_by_name("transportation")) {
            //std::cout << "transportation" << std::endl;
            while (auto feature = layer.next_feature()) {
                std::string road_class = "";
                std::string road_subclass = "";
                std::string road_service = "";
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    switch (property.value().type()) {
                    case vtzero::property_value_type::string_value:
                        if (property.key() == "class") {
                            road_class = property.value().string_value().to_string();
                        } else if (property.key() == "subclass") {
                            road_subclass = property.value().string_value().to_string();
                        } else if (property.key() == "service") {
                            road_service = property.value().string_value().to_string();
                        } /*else
                            std::cout << "\t\tproperty:" << prop_name << std::endl;*/
                        break;
                    case vtzero::property_value_type::int_value:
                        std::cout << "\t\tproperty:" << prop_name << " = " << property.value().int_value() << std::endl;
                        break;
                    /*default:
                        std::cout << "\t\tproperty:" << prop_name << " = " << (int)property.value().type() << std::endl;
                        break;*/
                    }
                }

                float width = 4.0f;

                uint32_t fg = CreateAbgr (ff,FD,D7,A1);
                uint32_t bg = CreateAbgr (ff,33,33,33);

                const float dashes[] = {4.0f,2.0f};
                const float dashesRail[] = {16.0f,16.0f};
                const float dashesTrack[] = {16.0f,6.0f,6.0f,6.0f};
                int dashCount = 2;
                float* curDash = NULL;
                if (road_class == "motorway" || road_class == "trunk") {
                    width = 50.0f;
                    fg = CreateAbgr (ff,f9,b0,9a);
                } else if (road_class == "primary") {
                    width = 50.0f;
                    fg = CreateAbgr (ff,fc,d7,a6);
                } else if (road_class == "secondary") {
                    width = 40.0f;
                    fg = CreateAbgr (ff,f7,fa,bd);
                } else if (road_class == "tertiary" || road_class == "minor") {
                    //std::cout << "\t\troad:" << road_class << "  " << road_subclass << " srv: " << road_service << std::endl;
                    width = 24.0f;
                    fg = CreateAbgr (ff,F1,EE,E8);
                } else if (road_class == "path") {
                    if (road_subclass == "footway" || road_subclass == "path") {
                        width = 2.0f;
                        fg = CreateAbgr (ff,69,64,DF);
                        bg = 0;
                        curDash = (float*)dashes;
                    } else if (road_subclass == "pedestrian" || road_subclass == "steps" || road_subclass == "platform") {
                        width = 8.0f;
                        fg = CreateAbgr (ff,e7,dd,dd);
                        bg = CreateAbgr (ff,a9,55,55);
                    } else if (road_subclass == "cycleway" ) {
                        width = 2.0f;
                        fg = CreateAbgr (ff,ff,55,55);
                        bg = 0;
                        curDash = (float*)dashes;
                    } else {
                        width = 14.0f;
                        fg = CreateAbgr (ff,00,00,ff);
                        std::cout << "\t\troad:" << road_class << "  " << road_subclass << " srv: " << road_service << std::endl;
                    }
                } else if (road_class == "transit") {
                    if (road_subclass == "tram") {
                        width = 4.0f;
                        fg = CreateAbgr (ff,ee,dd,dd);
                        bg = 0;
                    }else if (road_subclass == "subway") {
                        width = 4.0f;
                        fg = CreateAbgr (ff,aa,aa,aa);
                        bg = 0;
                    } else {
                        width = 14.0f;
                        fg = CreateAbgr (ff,00,00,ff);
                        std::cout << "\t\troad:" << road_class << "  " << road_subclass << " srv: " << road_service << std::endl;
                    }
                } else if (road_class == "rail") {
                    width = 6.0f;
                    bg = CreateAbgr (ff,33,33,33);
                    fg = CreateAbgr (ff,CC,CC,CC);
                    curDash = (float*)dashesRail;
                } else if (road_class == "service") {
                    width = 8.0f;
                    fg = CreateAbgr (ff,e7,dd,dd);
                    bg = CreateAbgr (ff,a9,55,55);
                    //width = 16.0f;
                    //fg = 0xff00ff00;
                } else if (road_class == "track") {
                    width = 4.0f;
                    bg = 0;
                    fg = CreateAbgr (ff,27,83,ac);
                    curDash = (float*)dashesTrack;
                    dashCount = 4;
                } else {
                    std::cout << "\t\troad:" << road_class << "  " << road_subclass << " srv: " << road_service << std::endl;
                    width = 10.f;
                    fg = CreateAbgr (ff,00,00,ff);
                    bg = 0;
                }

                width /= pow(2, 14 - z);

                hndGeometryDrawer handler{ ctx };
                if (feature.geometry_type() == vtzero::GeomType::LINESTRING) {
                    vtzero::decode_geometry(feature.geometry(), handler);
                    if (bg) {
                        vkvg_set_source_color(ctx, bg);
                        vkvg_set_line_width(ctx, width + 2);
                        vkvg_stroke_preserve(ctx);
                    }
                    vkvg_set_line_width(ctx, width);
                    vkvg_set_source_color(ctx, fg);
                    if (curDash) {
                        vkvg_set_dash(ctx, curDash, dashCount, 0);
                        vkvg_stroke(ctx);
                        vkvg_set_dash(ctx, NULL, 0, 0);
                    } else {
                        vkvg_stroke(ctx);
                    }
                }
            }
        }
        vkvg_flush(ctx);
        vkvg_set_font(ctx, fontRoads);
        vkvg_font_extents_t fe;
        vkvg_font_extents(ctx, &fe);
        float offsetY = (fe.ascent - fe.descent) * 0.5f;
        vkvg_set_line_width(ctx, 2.0);
        vkvg_set_source_rgb(ctx, 0.1f, 0.1f, 0.1f); // Dark gray text
        if (auto layer = tile.get_layer_by_name("transportation_name")) {
            while (auto feature = layer.next_feature()) {
                std::string road_class = "";
                std::string road_subclass = "";
                std::string road_network = "";
                std::string road_name = "";
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    switch (property.value().type()) {
                    case vtzero::property_value_type::string_value:
                        if (property.key() == "class") {
                            road_class = property.value().string_value().to_string();
                        } else if (property.key() == "subclass") {
                            road_subclass = property.value().string_value().to_string();
                        } else if (property.key() == "network") {
                            road_network = property.value().string_value().to_string();
                        } else if (property.key() == "name:latin") {
                            road_name = property.value().string_value().to_string();
                        } else
                            std::cout << "\t\tproperty:" << prop_name << std::endl;
                        break;
                    default:
                        std::cout << "\t\tproperty:" << prop_name << std::endl;
                        break;
                    }
                }
                hndGeometryDrawer handler{ ctx };
                if (feature.geometry_type() == vtzero::GeomType::LINESTRING) {
                    vtzero::decode_geometry(feature.geometry(), handler);
                    VkvgText txt = vkvg_text_create (ctx, road_name.c_str());
                    //vkvg_set_source_rgb(ctx, 0.1f, 0.1f, 0.1f); // Dark gray text
                    vkvg_text_render_on_path (ctx, txt, 0, offsetY, false);
                    vkvg_text_destroy(txt);
                    /*vkvg_set_source_rgb(ctx, 1.f, 0.1f, 0.1f);
                    vkvg_stroke(ctx);*/
                }
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("building")) {
            uint32_t fg = CreateAbgr (ff,22,22,22);
            //uint32_t bg = CreateAbgr (ff,cc,87,23);
            uint32_t bg = CreateAbgr (ff,cc,cc,cc);
            vkvg_set_line_width(ctx, 1);
            while (auto feature = layer.next_feature()) {
                if (feature.geometry_type() != vtzero::GeomType::POLYGON)
                    continue;
                vkvg_set_source_color(ctx, bg);
                hndGeometryDrawer handler{ctx};
                vtzero::decode_geometry(feature.geometry(), handler);
                vkvg_fill_preserve(ctx);
                vkvg_set_source_color(ctx, fg);
                vkvg_stroke(ctx);
            }
        }
        vkvg_flush(ctx);
        if (z == 14) {
            if (auto layer = tile.get_layer_by_name("poi")) {
                while (auto feature = layer.next_feature()) {
                    std::string poi_class = "";
                    std::string poi_subclass = "";
                    std::string poi_name = "";
                    while (auto property = feature.next_property()) {
                        std::string prop_name{property.key()};
                        switch (property.value().type()) {
                        case vtzero::property_value_type::string_value:
                            if (property.key() == "class") {
                                poi_class = property.value().string_value().to_string();
                            } else if (property.key() == "subclass") {
                                poi_subclass = property.value().string_value().to_string();
                            } else if (property.key() == "name") {
                                poi_name = property.value().string_value().to_string();
                            }
                            break;
                        /*case vtzero::property_value_type::int_value:
                            std::cout << "\t\tproperty:" << prop_name << " = " << property.value().int_value() << std::endl;
                            break;
                        default:
                            std::cout << "\t\tproperty:" << prop_name << " = " << (int)property.value().type() << std::endl;
                            break;*/
                        }
                    }

                    if (feature.geometry_type() != vtzero::GeomType::POINT)
                        continue;

                    hndGeometryDrawer handler{ctx};
                    vtzero::decode_geometry(feature.geometry(), handler);

                    uint32_t current_class_hash = fnv32(poi_class.c_str());
                    uint32_t current_subclass_hash = fnv32(poi_subclass.c_str());

                    const SvgIcon* icon = icons->get_icon(current_class_hash, current_subclass_hash);
                    cache.pois_by_icon[icon].push_back(Place{handler.point, poi_name});
                    /*Point p{handler.point.x - icon_size / 2, handler.point.y - icon_size / 2};
                    if (icon && icon->pat) {
                        vkvg_save(ctx);
                        vkvg_translate(ctx, p.x, p.y);
                        //vkvg_set_source_with_offset(ctx, icon->pat, p.first, p.second);
                        vkvg_set_source(ctx, icon->pat);
                        vkvg_paint(ctx);
                        vkvg_restore(ctx);
                    } else {
                        vkvg_set_source_color(ctx, fg);
                        vkvg_rounded_rectangle(ctx, p.x, p.y, icon_size / 2, icon_size / 2, 5);
                        vkvg_fill(ctx);
                        //std::cout << poi_class << ", " << poi_subclass << ", " << poi_name << std::endl;
                    }*/
                }
            }
        }
        if (z > 11) {
            if (auto layer = tile.get_layer_by_name("place")) {
                vkvg_set_font(ctx, fontPlaceM);
                while (auto feature = layer.next_feature()) {
                    std::string poi_class = "";
                    std::string poi_subclass = "";
                    std::string poi_name = "";
                    while (auto property = feature.next_property()) {
                        std::string prop_name{property.key()};
                        switch (property.value().type()) {
                        case vtzero::property_value_type::string_value:
                            if (property.key() == "class") {
                                poi_class = property.value().string_value().to_string();
                            } else if (property.key() == "subclass") {
                                poi_subclass = property.value().string_value().to_string();
                            } else if (property.key() == "name:latin") {
                                poi_name = property.value().string_value().to_string();
                            }
                            break;
                            /*case vtzero::property_value_type::int_value:
                                std::cout << "\t\tproperty:" << prop_name << " = " << property.value().int_value() << std::endl;
                                break;
                            default:
                                std::cout << "\t\tproperty:" << prop_name << " = " << (int)property.value().type() << std::endl;
                                break;*/
                        }
                    }
                    hndGeometryDrawer handler{ctx};
                    vtzero::decode_geometry(feature.geometry(), handler);

                    if (feature.geometry_type() == vtzero::GeomType::POINT) {
                        if (poi_class == "town" || poi_class == "city") {
                            if (tile_z < 13)
                                cache.TownsCities.push_back(Place{handler.point, poi_name});
                        } else if (poi_class == "village") {
                            if (tile_z < 13)
                                cache.Vilages.push_back(Place{handler.point, poi_name});
                        } /*else if (poi_class == "square") {
                            if (tile_z < 14)
                                continue;
                        } else if (poi_class == "quarter") {
                            if (tile_z < 13)
                                continue;
                        }*/
                            //poi_class == "neighbourhood" || poi_class == "locality" || || poi_class == "quarter"
                        /*    VkvgText txt = vkvg_text_create(ctx, poi_name.c_str());
                            vkvg_text_extents_t te = {0};
                            vkvg_text_get_extents(txt, &te);
                            Point p{handler.point.x - te.width / 2, handler.point.y};
                            vkvg_move_to(ctx, p.x, p.y);
                            vkvg_set_source_rgb(ctx, 0.5,0.5,1);
                            vkvg_text_render(ctx, txt);
                            vkvg_set_source_rgb(ctx, 1,0,0);
                            vkvg_arc(ctx, handler.point.x, handler.point.y, 20, 0, 2*M_PI);
                            vkvg_fill(ctx);*/
                        /*} else
                            std::cout << poi_class << ", " << poi_subclass << ", " << poi_name << std::endl;*/
                    }
                }
            }
        }

        vkvg_identity_matrix(ctx);
        vkvg_set_line_width(ctx,2);
        vkvg_set_source_rgba(ctx, 1.0f, 0.0f, 0.0f, 0.8f);
        vkvg_rectangle(ctx, 0, 0, tileSize, tileSize);
        vkvg_stroke(ctx);

        vkvg_set_font(ctx, font);
        std::stringstream tilestr;
        tilestr << "(" << tx << ", " << ty << ", " << z << ")";
        vkvg_move_to(ctx, tileSize / 2, tileSize / 2);
        vkvg_show_text(ctx, tilestr.str().c_str());

        vkvg_destroy(ctx);
        cache.fully_rendered = true;
    }

    void OnMouseMove(double xpos, double ypos) override {
        VkvgApp::OnMouseMove(xpos, ypos);
        if (isDragging) {
            float scaledTileSize = static_cast<float>(tileSize) * zoom;
            if (offsetX < -scaledTileSize) {
                tile_x++;   offsetX += scaledTileSize;
            } else if (offsetX > scaledTileSize) {
                tile_x--;   offsetX -= scaledTileSize;
            }

                   // TOP-LEFT FIX: Standard tracking direction for Y axis wrapping
            if (offsetY < -scaledTileSize) {
                tile_y++;   offsetY += scaledTileSize;
            } else if (offsetY > scaledTileSize) {
                tile_y--;   offsetY -= scaledTileSize;
            }
            UpdateVisibleTiles();
        }
        /*double worldX = (xpos - offsetX) / zoom;
        double worldY = (ypos - offsetY) / zoom;
        std::cout << "mouse(" << xpos << "," << ypos << ") world(" << worldX << "," << worldY << ")" << std::endl;*/
    }
    void OnScroll(double xoffset, double yoffset) override {
        // 1. Capture exactly where the mouse pointer is sitting on the window right now
        double mouseX, mouseY;
        glfwGetCursorPos(win, &mouseX, &mouseY);

        // 2. Calculate the global pixel coordinate relative to the current layout center
        double worldX = (mouseX - offsetX) / zoom;
        double worldY = (mouseY - offsetY) / zoom;

        // 3. Apply smooth scrolling step multipliers
        double zoomFactor = (yoffset > 0) ? 1.15 : (1.0 / 1.15);
        zoom *= zoomFactor;

        bool zoom_changed = false;

        // --- SWAP TILE RESOLUTION LAYERS ---
        if (zoom > 2.0 && tile_z < 14) {
            tile_z++;
            zoom /= 2.0;
            worldX *= 2.0;
            worldY *= 2.0;
            tile_x *= 2;
            tile_y *= 2;
            zoom_changed = true;
        } else if (zoom < 0.5 && tile_z > 10) {
            tile_z--;
            worldX /= 2.0;
            worldY /= 2.0;
            if (tile_x % 2)
                worldX += tileSize * zoom;
            if (tile_y % 2)
                worldY += tileSize * zoom;
            tile_x /= 2;
            tile_y /= 2;
            zoom *= 2.0;
            zoom_changed = true;
        }

        if (zoom_changed) {
            for (auto& tile : tile_surface_cache) {
                if (tile.surface) vkvg_surface_destroy(tile.surface);
            }
            tile_surface_cache.clear();
        }

        offsetX = mouseX - (worldX * zoom);
        offsetY = mouseY - (worldY * zoom);

        UpdateVisibleTiles();
        refresh = true;
    }
    void drawText(VkvgContext ctx, std::string text, Point point) {
        VkvgText txt = vkvg_text_create(ctx, text.c_str());
        vkvg_text_extents_t te = {0};
        vkvg_text_get_extents(txt, &te);
        Point p{point.x - te.width / 2, point.y};
        vkvg_move_to(ctx, p.x + 2, p.y + 2);
        vkvg_set_source_rgb(ctx, 0.3,0.3,0.3);
        vkvg_text_render(ctx, txt);
        vkvg_move_to(ctx, p.x, p.y);
        vkvg_set_source_rgb(ctx, 0.9,0.9,0.9);
        vkvg_text_render(ctx, txt);
    }
    void Render() override {
        VkvgContext ctx = vkvg_create(surf);

        vkvg_set_source_rgba(ctx, 0.96f, 0.95f, 0.94f, 1.0f);
        vkvg_paint(ctx);

        vkvg_translate(ctx, offsetX, offsetY);
        vkvg_scale(ctx, zoom, zoom);

        float localTileSize = static_cast<float> (tileSize);

        for (const auto& tile : tile_surface_cache) {
            if (!tile.surface) continue;
            float slot_x = (tile.tx - tile_x) * localTileSize;
            float slot_y = (tile.ty - tile_y) * localTileSize;

            vkvg_save(ctx);
            vkvg_translate(ctx, slot_x, slot_y);

            vkvg_set_source_surface(ctx, tile.surface, 0, 0);
            vkvg_paint(ctx);

            vkvg_identity_matrix(ctx);
            vkvg_translate(ctx, offsetX, offsetY);
            vkvg_translate(ctx, slot_x*zoom, slot_y*zoom);

            vkvg_set_font(ctx, fontPlace);
            //vkvg_set_font(ctx, fontPlaceM);
            for (const auto& town : tile.TownsCities) {
                Point screen_pos;
                screen_pos.x = town.loc.x * tileSizeRatio * zoom;
                screen_pos.y = town.loc.y * tileSizeRatio * zoom;
                drawText(ctx, town.name, screen_pos);

                /*vkvg_set_source_rgb(ctx, 0,1,0);
                vkvg_arc(ctx, screen_pos.x, screen_pos.y, 5, 0, 2*M_PI);
                vkvg_fill(ctx);*/
            }
            //vkvg_flush(ctx);
            //vkvg_set_font(ctx, fontPlaceM);
            vkvg_set_font(ctx, fontPlaceM);
            for (const auto& village : tile.Vilages) {
                Point screen_pos;
                screen_pos.x = village.loc.x * tileSizeRatio * zoom;
                screen_pos.y = village.loc.y * tileSizeRatio * zoom;
                drawText(ctx, village.name, screen_pos);
                /*vkvg_set_source_rgb(ctx, 0,0,1);
                vkvg_arc(ctx, screen_pos.x, screen_pos.y, 5, 0, 2*M_PI);
                vkvg_fill(ctx);*/
            }
            for (const auto& pair : tile.pois_by_icon) {
                const SvgIcon* icon = pair.first;
                const auto& instances = pair.second;
                if (icon && icon->pat) {
                    vkvg_set_source(ctx, icon->pat);
                } else {
                    uint32_t fallback_fg = CreateAbgr(ff, ff, 22, 22);
                    vkvg_set_source_color(ctx, fallback_fg);
                }
                for (const auto& poi : instances) {
                    Point screen_pos;
                    screen_pos.x = poi.loc.x * tileSizeRatio * zoom - icon_size / 2.0f;
                    screen_pos.y = poi.loc.y * tileSizeRatio * zoom - icon_size / 2.0f;
                    vkvg_translate(ctx, screen_pos.x, screen_pos.y);

                    if (icon && icon->pat) {
                        vkvg_paint(ctx);
                    } else {
                        vkvg_set_line_width(ctx, 1);
                        vkvg_rounded_rectangle(ctx, 0, 0, icon_size / 2.0f, icon_size / 2.0f, 5);
                        vkvg_fill(ctx);
                    }

                    vkvg_translate(ctx, -screen_pos.x, -screen_pos.y);
                }
            }

            vkvg_restore(ctx);
        }

        vkvg_identity_matrix(ctx);
        vkvg_set_font(ctx, font);
        std::stringstream str;
        str << "zoom=" << zoom << " tile_xyz=(" << tile_x << "," << tile_y << "," << tile_z << ")" << " offsets=(" << offsetX << "," << offsetY << ")";
        vkvg_move_to(ctx, 5, 25);
        vkvg_set_source_rgb(ctx, 0, 0, 0);
        vkvg_show_text(ctx, str.str().c_str());

        vkvg_destroy(ctx);
        refresh = false;
    }

    std::vector<char> decompress_gzip(const std::vector<char>& compressed_data) {
        if (compressed_data.empty()) {
            return {};
        }

        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = static_cast<uInt>(compressed_data.size());
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data.data()));

               // 16 + MAX_WBITS forces zlib to automatically detect and decode GZIP headers
        if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib inflate.");
        }

        std::vector<char> decompressed_data;
        // Chunk size matching standard operating system page allocations
        char buffer[4096];

        int ret;
        do {
            strm.avail_out = sizeof(buffer);
            strm.next_out = reinterpret_cast<Bytef*>(buffer);

            ret = inflate(&strm, Z_NO_FLUSH);

            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                throw std::runtime_error("GZIP decompression failed: Data corruption or memory error.");
            }

                   // Calculate how many bytes were decoded in this iteration pass
            size_t have = sizeof(buffer) - strm.avail_out;
            decompressed_data.insert(decompressed_data.end(), buffer, buffer + have);

        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);
        return decompressed_data;
    }
    std::vector<char> fetch_tile_from_mbtiles(sqlite3* db, int z, int x, int y) {
        std::vector<char> tile_data;

        const char* sql = "SELECT tile_data FROM tiles WHERE zoom_level = ? AND tile_column = ? AND tile_row = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tile_data;
        }

        int tms_y = ((1 << z) - 1) - y;

        sqlite3_bind_int(stmt, 1, z);
        sqlite3_bind_int(stmt, 2, x);
        sqlite3_bind_int(stmt, 3, tms_y);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Access the pointer to the binary blob inside SQLite memory
            const char* blob = static_cast<const char*>(sqlite3_column_blob(stmt, 0));
            int blob_size = sqlite3_column_bytes(stmt, 0);

            if (blob && blob_size > 0) {
                tile_data.assign(blob, blob + blob_size);
            }
        }

        sqlite3_finalize(stmt);
        return tile_data;
    }
    Point  fetch_center_from_mbtiles(sqlite3* db) {
        Point center = {0.0f, 0.0f};
        const char* sql = "SELECT value FROM metadata WHERE name = 'center';";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return center;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Retrieve raw string pointer (e.g., "-74.006,40.7128,12")
            const unsigned char* raw_text = sqlite3_column_text(stmt, 0);
            if (raw_text) {
                std::string center_str(reinterpret_cast<const char*>(raw_text));

                // 4. Parse the "lon,lat,zoom" string using stringstream
                std::stringstream ss(center_str);
                std::string lon_s, lat_s;

                if (std::getline(ss, lon_s, ',') && std::getline(ss, lat_s, ',')) {
                    try {
                        center.x = std::stof(lon_s);  // Longitude
                        center.y = std::stof(lat_s); // Latitude
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing float values: " << e.what() << std::endl;
                    }
                }
            }
        }
        sqlite3_finalize(stmt);
        return center;
    }
};

int main(int argc, char** argv) {
    vtzeroApp app{};

    argparse::ArgumentParser program("vtzero");
    program.add_argument("-w", "--width")
        .help("Output width")
        .default_value<uint32_t>(1024)
        .store_into<uint32_t>(app.width);
    program.add_argument("-h", "--height")
        .help("Output height")
        .default_value<uint32_t>(1024)
        .store_into<uint32_t>(app.height);
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    program.add_argument("--severity")
        .help("Debug messenger severity: 3:Err 2:Warn 1:Info 0:Verbose")
        .default_value<std::vector<uint32_t>>({0})
        .scan<'u', uint32_t>()
        .nargs(argparse::nargs_pattern::at_least_one);
    program.add_argument("--logtype")
        .help("Log type bits: 1:General 2:Validation 4:Perf 8:DevAddrBind")
        .default_value<uint32_t>(2)
        .store_into<uint32_t>(app.logType);
#endif
    program.add_argument("--png")
        .help("save last image to png")
        .default_value(false)
        .implicit_value(true)
        .store_into(app.save_img);

    program.add_argument("--list-gpus")
        .help("Get available GPU's, select with option '-g'")
        .default_value(false)
        .implicit_value(true)
        .store_into(app.listGpus);
    program.add_argument("-g", "--gpu").help("GPU index").default_value<int32_t>(-1).store_into<int32_t>(app.gpuIndex);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    app.logSeverity = program.get<std::vector<uint32_t>>("--severity");
#endif

    //vkvg_log_level = VKVG_LOG_ERR | VKVG_LOG_WARN | VKVG_LOG_INFO;

    app.Run();
}
