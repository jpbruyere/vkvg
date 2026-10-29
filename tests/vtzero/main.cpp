#include "VkvgApp.hpp"

#include "argparse/argparse.hpp"

#include <sqlite3.h>
#include <vtzero/vector_tile.hpp>
#include <vector>
#include <stdexcept>
#include <zlib.h>
#include <string>
#include <iostream>

struct RenderPolygon {
    std::vector<std::vector<std::pair<float, float>>> rings; // Supports outer and inner hole rings
    float r, g, b;                                           // Pre-resolved colors
    int tx, ty;
};
struct TrackedLine {
    std::vector<std::pair<float, float>> points;
    float width;
    float r, g, b;
    int tx, ty;
};
// The absolute correct polygon geometry template interface required by vtzero
struct VkvgGeometryHandler {
    std::vector<std::pair<float, float>> current_ring;
    std::vector<std::vector<std::pair<float, float>>>& out_rings;

           // Called at the start of every ring layer (outer boundary or inner hole)
    void ring_begin(uint32_t count) {
        current_ring.clear();
        current_ring.reserve(count);
    }

           // Called for every coordinate pair inside the polygon ring
    void ring_point(const vtzero::point p) {
        current_ring.push_back({static_cast<float>(p.x), static_cast<float>(p.y)});
    }

           // Called at the end of every ring layer
    void ring_end(vtzero::ring_type type) {
        if (!current_ring.empty()) {
            out_rings.push_back(std::move(current_ring));
        }
    }
};
struct VkvgLineHandler {
    VkvgContext ctx;

           // Called at the start of the entire feature parsing pass
    void linestring_begin(uint32_t count) {}

           // 1. CRITICAL: Called every time a new, separate road stroke starts!
    void line_begin(uint32_t count) {}

           // 2. CRITICAL: Handles points sequentially, lifting the pen if it's the start
           // of a brand new disconnected road segment block
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

           // 3. CRITICAL: Called when a separate sub-line ends
    void line_end() {
        // Reset the segment tracker state flag so the next line segment issues a fresh move_to
        // instead of attaching a webbing string back across the forest!
        // We will call the actual stroke command at the feature level
    }

    void linestring_end() {}
};

struct CachedTileSurface {
    VkvgSurface surface = nullptr; // The pre-rendered hardware texture layer
    int tx, ty;                    // Absolute tile coordinates
    bool fully_rendered = false;
};
class vtzeroApp : public VkvgApp {
    sqlite3* mbtiles_db;
    std::vector<CachedTileSurface> tile_surface_cache;

    static const uint32_t tileSize = 2048;

    int tile_x = 8388;
    int tile_y = 5495;
    int tile_z = 14;

    std::vector<RenderPolygon> background_landuse; // Forests, woods, parks, etc.
    std::vector<RenderPolygon> water_bodies;       // Lakes, canals, rivers
    std::vector<TrackedLine> road_network;

    std::vector<std::pair<int, int>> loaded_tiles;

public:
    VkvgFont font;

    void Init ()  override {
        VkvgApp::Init();

        font = vkvg_font_create(device, "sans:bold", 20);

        if (sqlite3_open("/mnt/data/osm/bruxelles.mbtiles", &mbtiles_db) != SQLITE_OK) {
        //if (sqlite3_open("/mnt/data/osm/ciney.mbtiles", &mbtiles_db) != SQLITE_OK) {
            std::cerr << "Failed to open map database file.\n";
            return;
        }

        offsetX = width * 0.5;
        offsetY = height * 0.5;

        /*tile_x = 8424;
        tile_y = 5535;*/
        /*tile_z = 14;*/
        //LoadTileGeometry(tile_z, tile_x, tile_y);
        UpdateVisibleTiles();
    }
    void CleanUp () override {
        for (auto& tile : tile_surface_cache) {
            if (tile.surface) vkvg_surface_destroy(tile.surface);
        }
        sqlite3_close(mbtiles_db);

        vkvg_font_destroy(font);

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
    }

    void BakeTileToSurface(sqlite3* mbtiles_db, CachedTileSurface& cache, int z, int tx, int ty, VkvgDevice device) {
        std::cout << "bake tile " << tx << ", " << ty << ", " << z << ", " << std::endl;
        cache.surface = vkvg_surface_create_ms(device, tileSize, tileSize, VK_SAMPLE_COUNT_8_BIT);
        cache.tx = tx;
        cache.ty = ty;

        VkvgContext ctx = vkvg_create(cache.surface);

               // Transparent baseline canvas so layers blend over background cleanly
        vkvg_set_source_rgba(ctx, 0.0f, 0.0f, 0.0f, 0.0f);
        vkvg_paint(ctx);

               // Set caps and joins to round for smooth connection intersections
        /*vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);
        vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);*/
        vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

        // Scale down the internal MVT 0-4096 coordinate system natively inside the cache layout
        vkvg_scale(ctx, 0.5f, 0.5f);

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
        }
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
        vkvg_flush(ctx);
        if (auto landuse = tile.get_layer_by_name("landuse")) {
            std::cout << "landuse" << std::endl;
            while (auto feature = landuse.next_feature()) {
                uint32_t c = 0xffb0cc9a;
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    std::string prop_val{property.value().string_value()};
                    if (prop_name == "class") {
                        if (prop_val == "residential")
                            c = 0xffccb18b;
                        else if (prop_val == "school" || prop_val == "college" || prop_val == "kindergarten" || prop_val == "university" || prop_val == "library")
                            c = 0xffe39ccf;
                        else if (prop_val == "hospital")
                            c = 0xffdf9ea5;
                        else if (prop_val == "commercial" || prop_val == "retail")
                            c = 0xffffe285;
                        else if (prop_val == "track" || prop_val == "pitch")
                            c = 0xffdddddd;
                        else if (prop_val == "playground" || prop_val == "stadium")
                            c = 0xff8bccb3;
                        else if (prop_val == "industrial" || prop_val == "railway")
                            c = 0xffb7b8cc;
                        else if (prop_val == "cemetery")
                            c = 0xff8acb94;
                        else if (prop_val == "neighbourhood" || prop_val == "garages")
                            c = 0xff0000ff;
                        else if (prop_val == "military")
                            c = 0xffb7b8cc;
                        else if (prop_val == "zoo")
                            c = 0xffcc8bbf;
                        else
                            std::cout << "\t\tproperty:" << prop_name << " = " << prop_val << std::endl;
                    }
                }
                std::vector<std::vector<std::pair<float, float>>> rings;
                VkvgGeometryHandler handler{{}, rings};
                vtzero::decode_polygon_geometry(feature.geometry(), handler);

                       //vkvg_set_source_rgb(ctx, r, g, b);
                vkvg_set_source_color(ctx, c);
                for (const auto& ring : rings) {
                    if (ring.empty()) continue;
                    vkvg_move_to(ctx, ring[0].first, ring[0].second);
                    for (size_t i = 1; i < ring.size(); ++i) vkvg_line_to(ctx, ring[i].first, ring[i].second);
                    vkvg_close_path(ctx);
                }
                vkvg_fill(ctx);
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("water")) {
            uint32_t c = 0xffb0cc9a;
            while (auto feature = layer.next_feature()) {
                std::vector<std::vector<std::pair<float, float>>> rings;
                VkvgGeometryHandler handler{{}, rings};
                vtzero::decode_polygon_geometry(feature.geometry(), handler);

                vkvg_set_source_rgb(ctx, 0.71f, 0.83f, 0.91f); // Map Blue
                for (const auto& ring : rings) {
                    if (ring.empty()) continue;
                    vkvg_move_to(ctx, ring[0].first, ring[0].second);
                    for (size_t i = 1; i < ring.size(); ++i) vkvg_line_to(ctx, ring[i].first, ring[i].second);
                    vkvg_close_path(ctx);
                }
                vkvg_fill(ctx);
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("transportation")) {
            std::cout << "transportation" << std::endl;
            uint32_t c = 0xffb0cc9a;
            while (auto feature = layer.next_feature()) {
                std::string road_class = "";
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    switch (property.value().type()) {
                    case vtzero::property_value_type::string_value:
                        if (property.key() == "class") {
                            road_class = property.value().string_value().to_string();
                        }
                        //std::cout << "\t\tproperty:" << prop_name << " = " << property.value().string_value().to_string() << std::endl;
                        break;
                    /*case vtzero::property_value_type::int_value:
                        std::cout << "\t\tproperty:" << prop_name << " = " << property.value().int_value() << std::endl;
                        break;
                    default:
                        std::cout << "\t\tproperty:" << prop_name << " = " << (int)property.value().type() << std::endl;
                        break;*/
                    }
                }

                float width = 4.0f;
                float r = 0.90f, g = 0.45f, b = 0.20f; // Highway Orange

                if (road_class == "motorway" || road_class == "trunk") {
                    width = (z <= 11) ? 2.5f : (z <= 13) ? 4.0f : 8.0f;
                    r = 0.90f; g = 0.45f; b = 0.20f;
                } else if (road_class == "primary" || road_class == "secondary") {
                    width = (z <= 11) ? 1.5f : (z <= 13) ? 3.0f : 6.0f;
                    r = 0.95f; g = 0.70f; b = 0.30f;
                } else {
                    width = (z <= 13) ? 1.0f : 10.f;
                    r = 0.9f; g = 0.9f; b = 0.7f;
                }

                       // Configure vector states directly on the master context
                vkvg_set_source_rgb(ctx, r, g, b);
                vkvg_set_line_width(ctx, width);

                       // Instantiate our new, direct pipeline handler
                VkvgLineHandler handler{ ctx };

                if (feature.geometry_type() == vtzero::GeomType::LINESTRING) {
                    vtzero::decode_linestring_geometry(feature.geometry(), handler);
                    vkvg_stroke(ctx);
                }
            }
        }
        vkvg_flush(ctx);
        if (auto layer = tile.get_layer_by_name("landcover")) {
            std::cout << "landcover" << std::endl;
            uint32_t c = 0xff0000ff;
            while (auto feature = layer.next_feature()) {
                while (auto property = feature.next_property()) {
                    std::string prop_name{property.key()};
                    std::string prop_val{property.value().string_value()};
                    if (prop_name == "class") {
                        if (prop_val == "grass")
                            c = 0xff9ce69c;
                        else if (prop_val == "farmland")
                            c = 0xffcc8bbf;
                        else if (prop_val == "wetland")
                            c = 0xff96f992;
                        else if (prop_val == "wood")
                            c = 0xff8dc56c;
                        else if (prop_val == "military")
                            c = 0xffb7b8cc;
                        else
                            std::cout << "\t\tproperty:" << prop_name << " = " << prop_val << std::endl;
                    }
                }
                std::vector<std::vector<std::pair<float, float>>> rings;
                VkvgGeometryHandler handler{{}, rings};
                vtzero::decode_polygon_geometry(feature.geometry(), handler);

                vkvg_set_source_color(ctx, c);
                for (const auto& ring : rings) {
                    if (ring.empty()) continue;
                    vkvg_move_to(ctx, ring[0].first, ring[0].second);
                    for (size_t i = 1; i < ring.size(); ++i) vkvg_line_to(ctx, ring[i].first, ring[i].second);
                    vkvg_close_path(ctx);
                }
                vkvg_fill(ctx);
            }
        }
        if (auto layer = tile.get_layer_by_name("building")) {
            uint32_t c = 0xff222222;
            vkvg_set_line_width(ctx, 1);
            while (auto feature = layer.next_feature()) {
                std::vector<std::vector<std::pair<float, float>>> rings;
                VkvgGeometryHandler handler{{}, rings};
                if (feature.geometry_type() == vtzero::GeomType::POLYGON)
                    vtzero::decode_polygon_geometry(feature.geometry(), handler);
                else
                    continue;

                vkvg_set_source_color(ctx, 0xff999999);
                for (const auto& ring : rings) {
                    if (ring.empty()) continue;
                    vkvg_move_to(ctx, ring[0].first, ring[0].second);
                    for (size_t i = 1; i < ring.size(); ++i) vkvg_line_to(ctx, ring[i].first, ring[i].second);
                    vkvg_close_path(ctx);
                }
                vkvg_fill_preserve(ctx);
                vkvg_set_source_color(ctx, 0xff222222);
                vkvg_stroke(ctx);
            }
        }
        vkvg_flush(ctx);

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
        int old_z = tile_z;

               // --- SWAP TILE RESOLUTION LAYERS ---
        if (zoom > 2.0 && tile_z < 14) {
            tile_z++;
            zoom /= 2.0; // Reset local scale factor to prevent jumping
            worldX *= 2.0;
            worldY *= 2.0;

            tile_x *= 2;
            tile_y *= 2;
            zoom_changed = true;
        }
        else if (zoom < 0.5 && tile_z > 10) {
            tile_z--;
            worldX /= 2.0;
            worldY /= 2.0;
            if (tile_x % 2)
                worldX += tileSize * zoom;
            if (tile_y % 2)
                worldY += tileSize * zoom;
            tile_x /= 2;
            tile_y /= 2;
            zoom *= 2.0; // Double local scale factor to keep things smooth
            zoom_changed = true;
        }

        if (zoom_changed) {
            offsetX = mouseX - (worldX * zoom);
            offsetY = mouseY - (worldY * zoom);

            for (auto& tile : tile_surface_cache) {
                if (tile.surface) vkvg_surface_destroy(tile.surface);
            }
            tile_surface_cache.clear();
        } else {
            offsetX = mouseX - (worldX * zoom);
            offsetY = mouseY - (worldY * zoom);
        }

        UpdateVisibleTiles();
        refresh = true;
    }


    void Render() override {
        VkvgContext ctx = vkvg_create(surf);

        vkvg_set_source_rgba(ctx, 0.96f, 0.95f, 0.94f, 1.0f);
        vkvg_paint(ctx);

        vkvg_translate(ctx, offsetX, offsetY);
        vkvg_scale(ctx, zoom, zoom);

               // Render each texture block side-by-side using the fast texture blit engine
        //float localTileSize = 819.2f; // Corresponding exactly to your 0.2 scale bounds
        float localTileSize = static_cast<float> (tileSize);

        for (const auto& tile : tile_surface_cache) {
            if (!tile.surface) continue;

                   // TOP-LEFT FIX: Simple delta spacing step
            float slot_x = (tile.tx - tile_x) * localTileSize;
            float slot_y = (tile.ty - tile_y) * localTileSize;

            vkvg_save(ctx);
            vkvg_translate(ctx, slot_x, slot_y);

            vkvg_set_source_surface(ctx, tile.surface, 0, 0);
            vkvg_paint(ctx);

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
};

int main(int argc, char** argv) {
    vtzeroApp app{};

    argparse::ArgumentParser program("samples");
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

    program.add_argument("-t", "--test-indices")
        .default_value<std::vector<uint32_t>>({})
        .help("test indices to run")
        .scan<'u', uint32_t>()
        .nargs(argparse::nargs_pattern::at_least_one);

    program.add_argument("--png")
        .help("save last image to png")
        .default_value(false)
        .implicit_value(true)
        .store_into(app.save_img);

    program.add_argument("--list-tests")
        .help("Get tests list")
        .default_value(false)
        .implicit_value(true);

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
