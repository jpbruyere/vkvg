// Auto-generated mapping dictionary using pre-calculated hashes
#include <stdint.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <filesystem>
#include "vkvg.h"
#include "vkvg-svg.h"

namespace fs = std::filesystem;

const int icon_size = 16;

// Structure matching your SVG texture or data buffer payload
struct SvgIcon {
    std::string filename;
    VkvgSurface surf;
    VkvgPattern pat;
};

class IconRegistry {
    VkvgDevice dev;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, SvgIcon>> icon_map;
    void initialize_icons() {
        #include "IconRegistryData.inc"
    }
  public:
    IconRegistry(VkvgDevice device) {
        initialize_icons();
        dev = device;
        vkvg_device_reference(dev);
        fs::path root = fs::path(OSM_DATA_ROOT);
        for (auto& [class_hash, sub_map] : icon_map) {
            for (auto& [sub_hash, icon] : sub_map) {
                fs::path icon_path = root / icon.filename;

                if (!fs::exists(icon_path)) {
                    std::cout << "icon file not found: " << icon_path << std::endl;
                    continue;
                }
                icon.surf = vkvg_surface_create_from_svg(dev, icon_size, icon_size, icon_path.c_str());
                icon.pat = vkvg_pattern_create_for_surface(icon.surf);
            }
        }
    }
    ~IconRegistry() {
        for (auto& class_pair : icon_map) {
            for (auto& sub_pair : class_pair.second) {
                if (sub_pair.second.surf) {
                    vkvg_pattern_destroy(sub_pair.second.pat);
                    vkvg_surface_destroy(sub_pair.second.surf);
                }
            }
        }
        vkvg_device_destroy(dev);
    }
    const SvgIcon* get_icon(uint32_t class_hash, uint32_t subclass_hash) const {
        auto class_it = icon_map.find(class_hash);
        if (class_it != icon_map.end()) {
            auto sub_it = class_it->second.find(subclass_hash);
            if (sub_it != class_it->second.end()) {
                return &sub_it->second;
            }
        }
        return nullptr;
    }
};