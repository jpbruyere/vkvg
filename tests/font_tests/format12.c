#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H // Required for SFNT structure macros

/* --- OpenType Binary Byte Order Swapping Helpers --- */
static uint16_t read_u16(const uint8_t* p) {
    return (p[0] << 8) | p[1];
}

static uint32_t read_u32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

/* --- Format 12 Structural Group Definition --- */
typedef struct {
    uint32_t startCharCode;
    uint32_t endCharCode;
    uint32_t startGlyphID;
} Format12Group;

void parse_raw_sfnt_cmap(FT_Face face) {
    // 1. Ensure the loaded font is an SFNT-based format (TrueType/OpenType)
    if (!FT_IS_SFNT(face)) {
        printf("Error: Loaded font face is not an SFNT (TrueType/OpenType) font.\n");
        return;
    }

    // 2. Query FreeType to check if the 'cmap' table exists and find its length
    FT_ULong table_len = 0;
    FT_Error err = FT_Load_Sfnt_Table(face, TTAG_cmap, 0, NULL, &table_len);
    if (err || table_len == 0) {
        printf("Error: Could not read 'cmap' table metrics.\n");
        return;
    }

    // 3. Allocate a memory buffer and download the raw 'cmap' bytes
    uint8_t* cmap_raw = (uint8_t*)malloc(table_len);
    err = FT_Load_Sfnt_Table(face, TTAG_cmap, 0, cmap_raw, &table_len);
    if (err) {
        printf("Error: Failed downloading raw 'cmap' block.\n");
        free(cmap_raw);
        return;
    }

    printf("Successfully extracted raw 'cmap' table. Size: %lu bytes.\n", table_len);

    /* --- Parse the CMap Index Header --- */
    // Bytes 0-1: version (must be 0)
    // Bytes 2-3: numSubtables (number of encoding tables)
    uint16_t numSubtables = read_u16(cmap_raw + 2);
    printf("Number of encoding subtables found: %d\n\n", numSubtables);

    uint32_t format12_offset = 0;

    /* --- Loop Through Subtables to Find Format 12 --- */
    // The subtable dictionary begins at byte 4. Each entry is 8 bytes.
    for (uint16_t i = 0; i < numSubtables; i++) {
        uint32_t entry_offset = 4 + (i * 8);
        uint16_t platform_id  = read_u16(cmap_raw + entry_offset);
        uint16_t encoding_id  = read_u16(cmap_raw + entry_offset + 2);
        uint32_t sub_offset   = read_u32(cmap_raw + entry_offset + 4);

        // Peek at the first 2 bytes of the subtable to check its Format ID
        uint16_t format = read_u16(cmap_raw + sub_offset);

        printf("  Subtable [%d]: Platform %d, Encoding %d, Format %d -> Offset 0x%X\n", 
               i, platform_id, encoding_id, format, sub_offset);

        if (format == 12) {
            format12_offset = sub_offset;
            // Stop looping if we find a standard Unicode/Windows full-plane table
            if (platform_id == 3 && encoding_id == 10) { 
                break;
            }
        }
    }

    if (format12_offset == 0) {
        printf("\nThis font does not contain a Format 12 subtable (likely BMP-only/Format 4).\n");
        free(cmap_raw);
        return;
    }

    /* --- Parse the Format 12 Subtable --- */
    printf("\n--- Parsing Format 12 Subtable at Offset 0x%X ---\n", format12_offset);
    const uint8_t* f12_ptr = cmap_raw + format12_offset;

    // Header Structure for Format 12:
    // uint16 format (12)      -> bytes 0-1
    // uint16 reserved        -> bytes 2-3
    // uint32 length          -> bytes 4-7
    // uint32 language        -> bytes 8-11
    // uint32 numGroups       -> bytes 12-15
    uint32_t f12_length   = read_u32(f12_ptr + 4);
    uint32_t numGroups    = read_u32(f12_ptr + 12);

    printf("  Subtable Byte Length: %u\n", f12_length);
    printf("  Total Compressed Segmented Groups: %u\n\n", numGroups);

    // Group Array begins at byte 16. Each group is 12 bytes.
    const uint8_t* groups_ptr = f12_ptr + 16;
    
    // Print out the structural boundaries mapped by the font designer
    for (uint32_t g = 0; g < numGroups; g++) {
        uint32_t g_offset = g * 12;
        Format12Group group;
        group.startCharCode = read_u32(groups_ptr + g_offset);
        group.endCharCode   = read_u32(groups_ptr + g_offset + 4);
        group.startGlyphID  = read_u32(groups_ptr + g_offset + 8);

        printf("  Group %-4u: Unicode [U+%06X to U+%06X] -> maps to starting Glyph ID: %u\n", 
               g, group.startCharCode, group.endCharCode, group.startGlyphID);
        
        // Safety break to prevent console flooding on giant fonts like Noto Sans
        if (g >= 15) {
            printf("  ... (%u more groups omitted) ...\n", numGroups - 16);
            break;
        }
    }

    // Clean up local buffers
    free(cmap_raw);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <path_to_font.ttf>\n", argv[0]);
        return 1;
    }

    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        printf("Could not init FreeType.\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(library, argv[1], 0, &face)) {
        printf("Could not load font file %s\n", argv[1]);
        FT_Done_FreeType(library);
        return 1;
    }

    parse_raw_sfnt_cmap(face);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return 0;
}

