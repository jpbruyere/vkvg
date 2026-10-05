#include <ft2build.h>
//#include "freetype/freetype.h"
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H


#include <fontconfig/fontconfig.h>

/*
 serif          Liberation Serif, DejaVu Serif, Times
 sans-serif     Liberation Sans, DejaVu Sans, Arial
 monospace      Liberation Mono, DejaVu Sans Mono
 cursive        Comic Neue, URW Chancery
 fantasy        Impact, Western
--------
 system-ui
 emoji          noto color emoji
 math
 fangsong       traditional chinese
*/
typedef enum {

    OldstyleSerifs                      = 0x0100,
    NoClassification                    = 0x0100,
    IBMRoundedLegibility                = 0x0101,
    Garalde                             = 0x0102,
    Venetian                            = 0x0103,
    ModifiedGaralde                     = 0x0104,
    DutchModern                         = 0x0105,
    Centaur                             = 0x0106,
    NoStyle                             = 0x0107,
    Contemporary                        = 0x0108,
    TransitionalSerifs                  = 0x0200,
    DirectLine                          = 0x0201,
    ScriptStyle                         = 0x0202,
    Neoclassical                        = 0x0203,
    Modern                              = 0x0204,
    Clarendon                           = 0x0205,
    SlabSerif                           = 0x0206,
    Typewriter                          = 0x0207,
    ModernSerifs                        = 0x0300,
    Italian                             = 0x0301,
    ModernSerifsScriptStyle             = 0x0302,
    ClarendonSerifs                     = 0x0400,
    ModernSerifsClarendon               = 0x0401,
    ModernClarendon                     = 0x0402,
    TraditionalClarendon                = 0x0403,
    NewspaperClarendon                  = 0x0404,
    StubSerif                           = 0x0405,
    Monospaced                          = 0x0406,
    FreeformSerifs                      = 0x0500,
    FreeformSerifsModern                = 0x0501,
    Traditional                         = 0x0502,
    SansSerif                           = 0x0700,
    IBMNeogrotesqueGothic               = 0x0701,
    Humanist                            = 0x0702,
    LowcontrastGrotesque                = 0x0703,
    HighcontrastGrotesque               = 0x0704,
    NeogrotesqueGothic                  = 0x0705,
    ModifiedNeogrotesqueGothic          = 0x0706,
    GeometricGrotesque                  = 0x0707,
    GeometricHumanist                   = 0x0708,
    GrotesqueGothic                     = 0x0709,
    Ornamentals                         = 0x0800,

    Engraved                            = 0x0801,
    Novelty                             = 0x0802,
    Retro                               = 0x0803,
    Synthetic                           = 0x0804,

    Scripts                             = 0x0900,
    Uncial                              = 0x0901,
    BrushCalligraphy                    = 0x0902,
    FormalCalligraphy                   = 0x0903,
    MonolineCalligraphy                 = 0x0904,
    TraditionalCalligraphy              = 0x0905,
    ContemporaryCalligraphy             = 0x0906,
    Cursive                             = 0x0907,
    Calligraphic                        = 0x0908,
    InlineOutline                       = 0x0909,
    Symbolic                            = 0x0A00,
    MixedSerif                          = 0x0A01,
    OldstyleSerif                       = 0x0A02,
    NeoclassicalSerif                   = 0x0A03,
    ModernSerif                         = 0x0A04,
    ClarendonSerif                      = 0x0A05,
    SymbolicSansSerif                   = 0x0A07,
    Ornamental                          = 0x0A08,
    Script                              = 0x0A09,
    Texture                             = 0x0A0A,
} font_class;

/*
Class 0x01 — Oldstyle Serifs
 0x0100 (Subclass 0x00): No Classification
 0x0101 (Subclass 0x01): IBM Rounded Legibility
 0x0102 (Subclass 0x02): Garalde
 0x0103 (Subclass 0x03): Venetian
 0x0104 (Subclass 0x04): Modified Garalde
 0x0105 (Subclass 0x05): Dutch Modern
 0x0106 (Subclass 0x06): Centaur
 0x0107 (Subclass 0x07): No Style
 0x0108 (Subclass 0x08): Contemporary

Class 0x02 — Transitional Serifs
 0x0200 (Subclass 0x00): No Classification
 0x0201 (Subclass 0x01): Direct Line
 0x0202 (Subclass 0x02): Script Style
 0x0203 (Subclass 0x03): Neo-classical
 0x0204 (Subclass 0x04): Modern
 0x0205 (Subclass 0x05): Clarendon
 0x0206 (Subclass 0x06): Slab Serif
 0x0207 (Subclass 0x07): Typewriter

Class 0x03 — Modern Serifs
 0x0300 (Subclass 0x00): No Classification
 0x0301 (Subclass 0x01): Italian
 0x0302 (Subclass 0x02): Script Style

Class 0x04 — Clarendon Serifs
 0x0400 (Subclass 0x00): No Classification
 0x0401 (Subclass 0x01): Clarendon
 0x0402 (Subclass 0x02): Modern Clarendon
 0x0403 (Subclass 0x03): Traditional Clarendon
 0x0404 (Subclass 0x04): Newspaper Clarendon
 0x0405 (Subclass 0x05): Stub Serif
 0x0406 (Subclass 0x06): Monospaced

Class 0x05 — Freeform Serifs
 0x0500 (Subclass 0x00): No Classification
 0x0501 (Subclass 0x01): Modern
 0x0502 (Subclass 0x02): Traditional

Class 0x07 — Sans Serif
 0x0700 (Subclass 0x00): No Classification
 0x0701 (Subclass 0x01): IBM Neo-grotesque Gothic
 0x0702 (Subclass 0x02): Humanist
 0x0703 (Subclass 0x03): Low-contrast Grotesque
 0x0704 (Subclass 0x04): High-contrast Grotesque
 0x0705 (Subclass 0x05): Neo-grotesque Gothic
 0x0706 (Subclass 0x06): Modified Neo-grotesque Gothic
 0x0707 (Subclass 0x07): Geometric Grotesque
 0x0708 (Subclass 0x08): Geometric Humanist
 0x0709 (Subclass 0x09): Grotesque Gothic

Class 0x08 — Ornamentals
 0x0800 (Subclass 0x00): No Classification
 0x0801 (Subclass 0x01): Engraved
 0x0802 (Subclass 0x02): Novelty
 0x0803 (Subclass 0x03): Retro
 0x0804 (Subclass 0x04): Synthetic

Class 0x09 — Scripts
 0x0900 (Subclass 0x00): No Classification
 0x0901 (Subclass 0x01): Uncial
 0x0902 (Subclass 0x02): Brush Calligraphy
 0x0903 (Subclass 0x03): Formal Calligraphy
 0x0904 (Subclass 0x04): Monoline Calligraphy
 0x0905 (Subclass 0x05): Traditional Calligraphy
 0x0906 (Subclass 0x06): Contemporary Calligraphy
 0x0907 (Subclass 0x07): Cursive
 0x0908 (Subclass 0x08): Calligraphic
 0x0909 (Subclass 0x09): Inline / Outline

Class 0x0A — Symbolic
 0x0A00 (Subclass 0x00): No Classification
 0x0A01 (Subclass 0x01): Mixed Serif
 0x0A02 (Subclass 0x02): Oldstyle Serif
 0x0A03 (Subclass 0x03): Neo-classical Serif
 0x0A04 (Subclass 0x04): Modern Serif
 0x0A05 (Subclass 0x05): Clarendon Serif
 0x0A07 (Subclass 0x07): Sans Serif
 0x0A08 (Subclass 0x08): Ornamental
 0x0A09 (Subclass 0x09): Script
 0x0A0A (Subclass 0x0A): Texture
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>

#include <stdarg.h>
#include <string.h>

#define MAX_PATH_LENGTH 1024
#define MAX_FAM_LENGTH  64

typedef struct {
    uint32_t    hash;
    char        string[MAX_FAM_LENGTH];
} hashed_string_t;

#define ARRAY_INIT          16
#define ARRAY_EXP_GROWTH    2
#define ARRAY_ELEMENT_TYPE  hashed_string_t
#define ARRAY_ID(x)         ((x).hash)

#define ARRAY_IMPLEMENTATION
#include "array.h"

static char*              filename    = NULL;
static char*              directory   = NULL;
struct dirent*            dir         = NULL;

struct stat file_stat;


#define NORMAL_COLOR "\x1B[0m"
#define GREEN        "\x1B[32m"
#define BLUE         "\x1B[34m"

static int   svg_file_count;
static float maxScroll;

/* not defined in c11, ok to define here only for sample */
#ifndef DT_DIR
#define DT_DIR 4
#endif

/**
 * @brief Computes the 64-bit FNV-1a hash of a ascii string lowering case.
 *
 * @param str Pointer to the start of the null terminated ascii string to hash.
 * @return uint64_t The 64-bit hash value.
 */
static inline uint64_t fnv1a_64_str(const char *const restrict str) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    const uint8_t *restrict data = (const uint8_t *)str;

    while (*data) {
        uint8_t c = *data;
        // Fast branchless lowercase for ASCII font names/paths
        if (c >= 'A' && c <= 'Z') {
            c |= 0x20;
        }
        hash ^= c;
        hash *= 0x00000100000001B3ULL;
        data++;
    }

    return hash;
}
#define FNV_32_PRIME 0x01000193
#define FNV_32_OFFSET 0x811C9DC5

static inline uint32_t fnv1a_32_str(const char *const restrict str) {
    const uint8_t *restrict data = (const uint8_t *)str;
    uint32_t hash = FNV_32_OFFSET;
    while (*data) {
        uint8_t c = *data;
        // Fast branchless lowercase for ASCII font names/paths
        if (c >= 'A' && c <= 'Z') {
            c |= 0x20;
        }
        hash ^= c;
        hash *= FNV_32_PRIME;
        data++;
    }
    return hash;
}

void convert_utf16be_to_ascii(const FT_Byte* src, FT_UInt src_len, char* dest, FT_UInt dest_max_len) {
    FT_UInt d_idx = 0;
    // Iterate 2 bytes at a time
    for (FT_UInt s_idx = 0; s_idx < src_len && d_idx < (dest_max_len - 1); s_idx += 2) {
        // In UTF-16BE, the second byte contains the ASCII value for basic English
        FT_Byte high_byte = src[s_idx];
        FT_Byte low_byte  = src[s_idx + 1];

        if (high_byte == 0) {
            dest[d_idx++] = (char)low_byte; // Standard English character
        } else {
            dest[d_idx++] = '?'; // Non-ASCII character fallback
        }
    }
    dest[d_idx] = '\0'; // Properly null-terminate the string
}


void print_help_and_exit() {
    printf("\nUsage: fonttest [options] [path]\n\n");
    exit(-1);
}
#define CHECK_FACE_FLAG(flags, flag, buffer) \
    do { \
            if ((flags) & (flag)) { \
                if (buffer[0] != '\0') strcat(buffer, " | "); \
                strcat(buffer, #flag); \
        } \
    } while (0)

/**
 * Converts face_flags bitmask to a human-readable string.
 * Make sure the output buffer is large enough (e.g., 512 bytes).
 */
void ft_face_flags_to_string(FT_Long face_flags, char* out_str, size_t max_len) {
    if (!out_str || max_len == 0) return;
    out_str[0] = '\0';

    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_SCALABLE, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_FIXED_SIZES, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_FIXED_WIDTH, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_SFNT, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_HORIZONTAL, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_VERTICAL, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_KERNING, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_MULTIPLE_MASTERS, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_GLYPH_NAMES, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_EXTERNAL_STREAM, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_HINTER, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_CID_KEYED, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_TRICKY, out_str);
    CHECK_FACE_FLAG(face_flags, FT_FACE_FLAG_COLOR, out_str);

    if (out_str[0] == '\0') {
        strncpy(out_str, "NONE", max_len);
    }
}

// Define explicit variation axis tags using FreeType's macro wrapper
#define AXIS_TAG_WGHT  FT_MAKE_TAG('w', 'g', 'h', 't')
#define AXIS_TAG_WDTH  FT_MAKE_TAG('w', 'd', 't', 'h')
#define AXIS_TAG_SLNT  FT_MAKE_TAG('s', 'l', 'n', 't')
#define AXIS_TAG_ITAL  FT_MAKE_TAG('i', 't', 'a', 'l')

typedef enum {
    font_weight_unset       = 0,
    font_weight_thin        = 100,
    font_weight_extra_light = 200,
    font_weight_light       = 300,
    font_weight_normal      = 400,
    font_weight_medium      = 500,
    font_weight_semi_bold   = 600,
    font_weight_bold        = 700,
    font_weight_extra_bold  = 800,
    font_weight_black       = 900,
} font_weight;

typedef enum {
    font_width_unset            = 0,
    font_width_ultra_condensed  = 1,//50%
    font_width_extra_condensed  = 2,
    font_width_condensed        = 3,
    font_width_semi_condensed   = 4,
    font_width_normal           = 5,
    font_width_semi_expanded    = 6,
    font_width_expanded         = 7,
    font_width_extra_expanded   = 8,
    font_width_ultra_expanded   = 9,//150%
} font_width;

typedef enum {
    font_slant_unset    = 0,
    font_slant_italic   = 0b0000000000000001,
    font_slant_regular  = 0b0000000001000000,
    font_slant_oblic    = 0b0000001000000000
} font_slant;


FT_Library  library;

typedef struct {
    uint64_t    pathHash;
    uint32_t    nameHash;
    uint32_t    styleHash;
} font_id_t;

typedef struct {
    const char *txtFontPattern; // font config textual representation of pattern
    const char *family;
    const char *style;
    font_weight weight;
    font_width  width;
    font_slant  slant;
    uint32_t    flags;
} vkvg_font_create_info_t;

font_id_t font_id_cache[7000];
array_t* font_names;
array_t* font_sub_names;

uint32_t fontIndex = 0;


const char* nameid_trimmed(const char* str) {
    return str + 11;
}



#define SFNT_NAME_VASE(value)   \
case value :                   \
    return nameid_trimmed(#value);

const char* sfnt_nameid_to_string(int id) {
    switch(id) {
        SFNT_NAME_VASE(TT_NAME_ID_COPYRIGHT)
        SFNT_NAME_VASE(TT_NAME_ID_FONT_FAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_FONT_SUBFAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_UNIQUE_ID)
        SFNT_NAME_VASE(TT_NAME_ID_FULL_NAME)
        SFNT_NAME_VASE(TT_NAME_ID_VERSION_STRING)
        SFNT_NAME_VASE(TT_NAME_ID_PS_NAME)
        SFNT_NAME_VASE(TT_NAME_ID_TRADEMARK)
        SFNT_NAME_VASE(TT_NAME_ID_MANUFACTURER)
        SFNT_NAME_VASE(TT_NAME_ID_DESIGNER)
        SFNT_NAME_VASE(TT_NAME_ID_DESCRIPTION)
        SFNT_NAME_VASE(TT_NAME_ID_VENDOR_URL)
        SFNT_NAME_VASE(TT_NAME_ID_DESIGNER_URL)
        SFNT_NAME_VASE(TT_NAME_ID_LICENSE)
        SFNT_NAME_VASE(TT_NAME_ID_LICENSE_URL)
        SFNT_NAME_VASE(TT_NAME_ID_TYPOGRAPHIC_FAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_TYPOGRAPHIC_SUBFAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_MAC_FULL_NAME)
        SFNT_NAME_VASE(TT_NAME_ID_SAMPLE_TEXT)
        SFNT_NAME_VASE(TT_NAME_ID_CID_FINDFONT_NAME)
        SFNT_NAME_VASE(TT_NAME_ID_WWS_FAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_WWS_SUBFAMILY)
        SFNT_NAME_VASE(TT_NAME_ID_LIGHT_BACKGROUND)
        SFNT_NAME_VASE(TT_NAME_ID_DARK_BACKGROUND)
        SFNT_NAME_VASE(TT_NAME_ID_VARIATIONS_PREFIX)

    }
    return "\0";
}

static inline void strn_tolower_copy(char *restrict dest, const char *restrict src, size_t len) {
    if (!dest || !src) return;

    for (size_t i = 0; i < len; ++i) {
        // cast to unsigned char is required by standard to prevent undefined behavior with negative char values
        dest[i] = (char)tolower((unsigned char)src[i]);
    }

    dest[len] = '\0'; // Explicitly enforce null-termination
}
static inline bool sfnt_name_is_english(const FT_Short platform_id, const FT_Short language_id) {
    return (platform_id == 3 && language_id == 0x0409) ||
           (platform_id == 0 && language_id == 0) ||
           (platform_id == 1 && language_id == 0);
}

void process_font_file (const char* const fontDir, const char* fontFile) {
    char path[MAX_PATH_LENGTH];
    sprintf(path, "%s/%s", fontDir, fontFile);
    FT_Face face;
    font_weight weight = font_weight_normal;
    font_width  width  = font_width_normal;
    font_slant  slant  = font_slant_regular;
    FT_Int16  fntClass = 0;

    FT_Error error = FT_New_Face(library, path, 0, &face );
    if (error) {
        fprintf(stdout, "error opening font: %s\n%s\n", path, FT_Error_String(error));
        return;
    }

    printf("%-100s:", path);
    printf("\tnum faces: %ld (%ld):\t%-40s%-20s#:%ld\n", face->num_faces, face->face_index, face->family_name, face->style_name,face->num_glyphs);
    char flags[1024];
    ft_face_flags_to_string(face->face_flags, flags, 1024);
    printf("\tFlags: %s\n", flags);

    if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
        FT_MM_Var* mm_var;
        FT_Get_MM_Var(face, &mm_var);
        printf("\tAxis: %4d Named Instances: %4d\n", mm_var->num_axis, mm_var->num_namedstyles);
        for (int i = 0; i < mm_var->num_namedstyles; ++i) {
            printf("\t%4d %u\n", i, mm_var->namedstyle[i].strid);
        }
        //mm_var->axis[0].;
        FT_Done_MM_Var(library, mm_var);
    }

    TT_OS2* os2_table = (TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
    if (os2_table) {
        // 1. Get the real numeric weight (e.g., 400, 700)
        weight = os2_table->usWeightClass;
        width = os2_table->usWidthClass;
        slant = os2_table->fsSelection;
        fntClass = os2_table->sFamilyClass;
    }

    printf("\tClass %04x Weight: %4d width: %4d slant: %4d\n",fntClass, weight, width, slant);


    FT_UInt sfntCount = FT_Get_Sfnt_Name_Count(face);
    FT_SfntName name;
    char stringAscii[1024];

    hashed_string_t famName = {0};
    hashed_string_t subFamName = {0};
    hashed_string_t typoFamName = {0};
    hashed_string_t typoSubFamName = {0};

    for (int i = 0; i < sfntCount; ++i) {
        error = FT_Get_Sfnt_Name(face, i, &name);
        if (error) {
            fprintf(stdout, "error reading sfnt name #%d, %s\n", i, FT_Error_String(error));
            continue;
        }
        /*FT_SfntLangTag langTag = {0};
        error = FT_Get_Sfnt_LangTag(face, name.language_id, &langTag);
        if (langTag.string_len < 1) {
            continue;
        }
        convert_utf16be_to_ascii(langTag.string, langTag.string_len, langTagAscii, 64);*/
        if (name.string_len < 1 || !sfnt_name_is_english(name.platform_id, name.language_id))
            continue;
        if (name.name_id == TT_NAME_ID_FONT_FAMILY ||
            name.name_id == TT_NAME_ID_FONT_SUBFAMILY ||
            name.name_id == TT_NAME_ID_FULL_NAME ||
            name.name_id == TT_NAME_ID_TYPOGRAPHIC_FAMILY ||
            name.name_id == TT_NAME_ID_TYPOGRAPHIC_SUBFAMILY ||
            //name.name_id == TT_NAME_ID_WWS_FAMILY ||
            //name.name_id == TT_NAME_ID_WWS_SUBFAMILY ||
            name.name_id == TT_NAME_ID_UNIQUE_ID) {

            /*fprintf(stdout, "sfntName(%1d:%3d:%4d:%6d): %-20s = ", name.platform_id, name.encoding_id, name.language_id, name.name_id, sfnt_nameid_to_string(name.name_id));
            if (name.platform_id == 3) {
                convert_utf16be_to_ascii(name.string, name.string_len, stringAscii, 1024);
                fprintf(stdout, "%s\n", stringAscii);
            } else {
                fprintf(stdout, "%.*s\n", name.string_len, name.string);
            }*/

            fflush(stdout);
        } else
            continue;

        if (name.platform_id == 3) {
            convert_utf16be_to_ascii(name.string, name.string_len, stringAscii, 1024);
        } else {
            strncpy(stringAscii, (char*)name.string, name.string_len < 1023 ? name.string_len : 1023);
            stringAscii[name.string_len < 1023 ? name.string_len : 1023] = '\0';
        }

        switch (name.name_id) {
        case TT_NAME_ID_TYPOGRAPHIC_FAMILY:
            if (typoFamName.hash)
                continue;
            strn_tolower_copy(typoFamName.string, stringAscii, strlen(stringAscii));
            typoFamName.hash = fnv1a_32_str(typoFamName.string);
            break;
        case TT_NAME_ID_TYPOGRAPHIC_SUBFAMILY:
            if (typoSubFamName.hash)
                continue;
            strn_tolower_copy(typoSubFamName.string, stringAscii, strlen(stringAscii));
            typoSubFamName.hash = fnv1a_32_str(typoSubFamName.string);
            break;
        case TT_NAME_ID_FONT_FAMILY:
            if (famName.hash)
                continue;
            strn_tolower_copy(famName.string, stringAscii, strlen(stringAscii));
            famName.hash = fnv1a_32_str(famName.string);
            break;
        case TT_NAME_ID_FONT_SUBFAMILY:
            if (subFamName.hash)
                continue;
            strn_tolower_copy(subFamName.string, stringAscii, strlen(stringAscii));
            subFamName.hash = fnv1a_32_str(subFamName.string);
            break;
        default:
            break;
        }



    }

    fflush(stdout);

    uint64_t pathHash = fnv1a_64_str(path);
    if (pathHash) {
        font_id_cache[fontIndex] = (font_id_t) { pathHash, fnv1a_32_str(face->family_name), fnv1a_32_str(face->style_name)};
        fontIndex++;
    } else {
        fprintf(stdout, "Path hash = 0 for %s\n", path);
    }

    if (typoFamName.hash)
        array_add_unique(font_names, typoFamName);
    else if (famName.hash)
        array_add_unique(font_names, famName);
    if (typoSubFamName.hash)
        array_add_unique(font_sub_names, typoSubFamName);
    else if (subFamName.hash)
        array_add_unique(font_sub_names, subFamName);

    FT_Done_Face(face);
}
void parse_font_dir (const char* const dir_path) {
    char path[MAX_PATH_LENGTH];
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("error opening directory: %s\n", dir_path);
        return;
    }
    rewinddir(dir);

    struct dirent* de;
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == DT_DIR) {
            if (de->d_name[0] == '.')
                continue;
            sprintf(path, "%s/%s", dir_path, de->d_name);
            parse_font_dir(path);
        } else {
            if (!strcasecmp(strrchr(de->d_name, '\0') - 4, ".otf") ||
                !strcasecmp(strrchr(de->d_name, '\0') - 4, ".ttf") ||
                !strcasecmp(strrchr(de->d_name, '\0') - 5, ".woff") ||
                !strcasecmp(strrchr(de->d_name, '\0') - 6, ".woff2")) {

                process_font_file(dir_path, de->d_name);
            }
        }
    }
    closedir(dir);
}

void sort_fonts_by_path_hash(font_id_t *array, size_t n) {
    if (n < 2) return;

           // Allocate a temporary buffer for the 16-byte structs
    font_id_t *buffer = (font_id_t *)malloc(n * sizeof(font_id_t));
    if (!buffer) return;

    font_id_t *src = array;
    font_id_t *dst = buffer;

           // 8 passes of Radix Sort (8 bits per pass = 64 bits total)
    for (int pass = 0; pass < 8; pass++) {
        size_t count[256] = {0}; // 256 buckets for an 8-bit chunk
        int shift = pass * 8;

               // 1. Calculate histograms directly using the unsigned pathHash
        for (size_t i = 0; i < n; i++) {
            uint8_t byte = (src[i].pathHash >> shift) & 0xFF;
            count[byte]++;
        }

               // 2. Transform counts into exclusive prefix sums (offsets)
        size_t total = 0;
        for (int i = 0; i < 256; i++) {
            size_t old_count = count[i];
            count[i] = total;
            total += old_count;
        }

               // 3. Scatter entire structs safely into the destination buffer
        for (size_t i = 0; i < n; i++) {
            uint8_t byte = (src[i].pathHash >> shift) & 0xFF;
            dst[count[byte]++] = src[i]; // Copies the full 16 bytes cleanly
        }

               // Ping-pong buffer pointers to avoid unnecessary copying
        font_id_t *temp = src;
        src = dst;
        dst = temp;
    }

           // If the final sorted data landed in the temporary buffer, copy it back
    if (src != array) {
        memcpy(array, buffer, n * sizeof(font_id_t));
    }

    free(buffer);
}
void radix_sort_structs(font_id_t *array, size_t n) {
    if (n < 2) return;

           // Allocate pointer arrays for the radix passes
    font_id_t **src = (font_id_t **)malloc(n * sizeof(font_id_t *));
    font_id_t **dst = (font_id_t **)malloc(n * sizeof(font_id_t *));
    if (!src || !dst) { free(src); free(dst); return; }

           // Initialize the source pointer array
    for (size_t i = 0; i < n; i++) {
        src[i] = &array[i];
    }

           // 8 passes of Radix Sort (8 bits per pass)
    for (int pass = 0; pass < 8; pass++) {
        size_t count[256] = {0};
        int shift = pass * 8;

               // 1. Histogram (Extract the byte from the 64-bit ID)
               // For signed int64_t, we flip the sign bit on the final pass (pass 7)
        for (size_t i = 0; i < n; i++) {
            uint64_t key = (uint64_t)src[i]->pathHash;
            if (pass == 7) key ^= 0x8000000000000000ULL; // Handle signed values
            uint8_t byte = (key >> shift) & 0xFF;
            count[byte]++;
        }

               // 2. Prefix Sums
        size_t total = 0;
        for (int i = 0; i < 256; i++) {
            size_t old_count = count[i];
            count[i] = total;
            total += old_count;
        }

               // 3. Scatter pointers
        for (size_t i = 0; i < n; i++) {
            uint64_t key = (uint64_t)src[i]->pathHash;
            if (pass == 7) key ^= 0x8000000000000000ULL;
            uint8_t byte = (key >> shift) & 0xFF;
            dst[count[byte]++] = src[i];
        }

       // Ping-pong pointer swap
        font_id_t **temp = src;
        src = dst;
        dst = temp;
    }

   // Reorder the original array in-place using cycle-walking (O(N) time, O(1) extra space)
   // 'src' now holds the correct final pointers in sorted order.
    for (size_t i = 0; i < n; i++) {
        if (src[i] != &array[i]) {
            font_id_t temp_struct = array[i];
            size_t j = i;
            while (1) {
                // Find where the element currently at array[j] belongs
                size_t k = (src[j] - array);
                if (k == i) {
                    array[j] = temp_struct;
                    src[j] = &array[j];
                    break;
                }
                array[j] = array[k];
                src[j] = &array[j];
                j = k;
            }
        }
    }

    free(src);
    free(dst);
}

void font_config_tests () {
    if (!FcInit()) {
        fprintf(stderr, "Fatal Error: Failed to initialize Fontconfig.\n");
        return;
    }

           // Load the system configuration and default user fonts
    FcConfig* config = FcConfigGetCurrent();

           // 2. Create an empty pattern filter (empty means "Match everything")
    FcPattern* blank_pattern = FcPatternCreate();

           // 3. Declare exactly what metadata elements you want to extract
           // Passing trailing 0/NULL tells the builder array arguments are complete
    FcObjectSet* requested_elements = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, (char *)0);

           // 4. Generate the sorted unique array list of matching fonts
    FcFontSet* font_database = FcFontList(config, blank_pattern, requested_elements);

    if (font_database) {
        printf("Successfully indexed %d font variants.\n\n", font_database->nfont);

        for (int i = 0; i < font_database->nfont; i++) {
            FcPattern* font_entry = font_database->fonts[i];

            FcChar8* family_name = NULL;
            FcChar8* style_variant = NULL;
            FcChar8* file_path = NULL;

                   // Extract the strings cleanly out of the font structure
            if (FcPatternGetString(font_entry, FC_FAMILY, 0, &family_name) == FcResultMatch &&
                FcPatternGetString(font_entry, FC_STYLE, 0, &style_variant) == FcResultMatch) {

                // Optional: Also fetch the file system location if you need to open it in FreeType
                FcPatternGetString(font_entry, FC_FILE, 0, &file_path);

                printf("[%d] Family: %s \n    Style:  %s\n    Path:   %s\n\n",
                       i + 1, family_name, style_variant, file_path ? (char*)file_path : "Unknown");
            }
        }

               // 5. Always free memory assets to prevent memory leaks
        FcFontSetDestroy(font_database);
    }

    FcObjectSetDestroy(requested_elements);
    FcPatternDestroy(blank_pattern);
    FcFini(); // Clean up environment space
}

void font_config_sandboxing() {
    setenv("FONTCONFIG_SYSROOT", "/tmp", 1);

    // 1. Create a brand new, completely blank configuration instance
    FcConfig* local_config = FcConfigCreate();
    if (!local_config) {
        fprintf(stderr, "Failed to create custom Fontconfig configuration.\n");
        return;
    }

           // 2. Define your local asset path
    const FcChar8* font_dir = (const FcChar8*)"/usr/local/share/fonts";

           // 3. Force Fontconfig to scan this directory and add it to the local config instance
           // This parses the font binaries in this directory into memory.
    if (!FcConfigAppFontAddDir(local_config, font_dir)) {
        fprintf(stderr, "Failed to register application font directory: %s\n", font_dir);
        FcConfigDestroy(local_config);
        return;
    }

           // 4. (Optional but Recommended) Bind this config as the active thread configuration.
           // This ensures subsequent generic Fc calls use your lightweight sandbox.
    FcConfigSetCurrent(local_config);

    printf("Fontconfig sandboxed successfully. Scanning only: %s\n\n", font_dir);

           // =========================================================================
           // Example: Let's query our sandbox for a font to prove it works
           // =========================================================================

    // Create a request pattern
    FcPattern* request = FcPatternCreate();
    FcPatternAddString(request, FC_FAMILY, (const FcChar8*)"sans-serif");

           // Apply substitutions using our custom config object instead of NULL (system default)
    FcConfigSubstitute(local_config, request, FcMatchPattern);
    FcDefaultSubstitute(request);

    FcResult result;
    // Match against our clean, local configuration file list
    FcPattern* matched_font = FcFontMatch(local_config, request, &result);

    if (matched_font) {
        FcChar8* file_path = NULL;
        FcChar8* family_name = NULL;

        if (FcPatternGetString(matched_font, FC_FILE, 0, &file_path) == FcResultMatch &&
            FcPatternGetString(matched_font, FC_FAMILY, 0, &family_name) == FcResultMatch) {

            printf("Successfully matched!\n");
            printf("Family: %s\n", family_name);
            printf("File Path: %s\n", file_path);
        }
        FcPatternDestroy(matched_font);
    } else {
        printf("No matching font found in the local asset folder.\n");
    }

           // 5. Cleanup
    FcPatternDestroy(request);

    // This unloads the memory assets allocated by FcConfigCreate
    FcConfigDestroy(local_config);

    // Clean up global libraries state
    FcFini();
}

/* requesting variable font
 *
FcPattern* request = FcPatternCreate();
FcPatternAddString(request, FC_FAMILY, (const FcChar8*)"Roboto Flex");
FcPatternAddString(request, FC_FONT_VARIATIONS, (const FcChar8*)"wght=625,wdth=80");
*/
/* Fallback chain
 // 1. Initialize Fontconfig library and load system configuration
    FcConfig* config = FcInitLoadConfigAndFonts();
    if (!config) return 1;

 // 2. Create an empty pattern
 FcPattern* pat = FcPatternCreate();

 // 3. Programmatically define the fallback chain (Order: Highest -> Lowest)
 // The last argument 'FcTrue' appends the font to the end of the list
 FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)"CustomWebFont", FcTrue);
 FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)"Liberation Sans", FcTrue);
 FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)"DejaVu Sans", FcTrue);
 FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)"sans-serif", FcTrue); // Generic system fallback

 // Add other attributes if needed (e.g., matching bold)
 FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_BOLD);

 // 4. Perform standard configuration-defined substitutions
 // This hooks into system/user XML configurations (~/.config/fontconfig/fonts.conf)
 // to fill in missing properties or alias maps.
 FcConfigSubstitute(config, pat,
*/

void setup_variable_font(FcPattern* matched_font) {
    // 1. Verify if the font handles open vector variables
    FcBool is_variable = FcFalse;
    if (FcPatternGetBool(matched_font, FC_VARIABLE, 0, &is_variable) == FcResultMatch && is_variable) {
        printf("This matched font file is variable-capable!\n");
    }

           // 2. Extract specific axis configuration parameters requested by a pattern string
           // This returns a string formatted exactly like CSS: "wght=525.0,wdth=85.5"
    FcChar8* variations_string = NULL;
    if (FcPatternGetString(matched_font, FC_FONT_VARIATIONS, 0, &variations_string) == FcResultMatch) {
        printf("Requested layout coordinates: %s\n", variations_string);

        // 3. You can pass this string directly to your rendering subsystem.
        // Inside FreeType, you convert this string into an array of FT_Fixed coordinates
        // and apply it directly via:
        // FT_Set_Var_Design_Coordinates(face, num_coords, coords);
    }
}

int main(int argc, char* argv[]) {
    int   i      = 1;
    char* output = NULL;

    while (i < argc) {
        int argLen = strlen(argv[i]);
        if (argv[i][0] == '-') {

            if (argLen < 2)
                print_help_and_exit();

            switch (argv[i][1]) {
            case 'f':
                if (argc < ++i + 1)
                    print_help_and_exit();
                filename = argv[i];
                break;
            default:
                print_help_and_exit();
            }
        } else
            directory = argv[i];
        i++;
    }
    if (!directory)
        print_help_and_exit();

    FT_Error error = FT_Init_FreeType( &library );
    if (error) {
        perror("error initializing freetype library\n");
        exit(-1);
    }

    //font_config_tests();
    font_config_sandboxing();




    //font_id_cache = array_create();

/*
    font_names = array_create();
    font_sub_names = array_create();

    if (directory) {
        parse_font_dir(directory);
    }
    printf("Total font files: %d\n", fontIndex);
    radix_sort_structs(font_id_cache, fontIndex);
*/


    /*for (int i = 0; i < fontIndex; ++i) {
        printf("%lu %u %u\n", font_id_cache[i].pathHash, font_id_cache[i].nameHash, font_id_cache[i].styleHash);
    }

    for (int i = 0; i < fontIndex - 1; ++i) {
        if (font_id_cache[i].pathHash == font_id_cache[i+1].pathHash)
            printf("Collision: %lu %u %u\n", font_id_cache[i].pathHash, font_id_cache[i].nameHash, font_id_cache[i].styleHash);
    }*/
    /*printf("=== Font Names: %-5d ===\n", font_names->count);
    for (int i = 0; i < font_names->count; ++i) {
        printf("%s\n", font_names->elements[i].string);
    }
    printf("=============================\n");
    printf("=== Font Sub Names: %-5d ===\n", font_sub_names->count);
    for (int i = 0; i < font_sub_names->count; ++i) {
        printf("%s\n", font_sub_names->elements[i].string);
    }
    printf("==============================\n");*/

 //   array_destroy(font_names);
 //   array_destroy(font_sub_names);

}
