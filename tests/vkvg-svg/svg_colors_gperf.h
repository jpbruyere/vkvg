/* ANSI-C code produced by gperf version 3.2.1 */
/* Command-line: gperf --output-file=build/svg_colors_gperf.h src/svg_colors_names.gperf  */
/* Computed positions: -k'3-4,6-8,12-13' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "src/svg_colors_names.gperf"

#include <string.h>

/* Enum representing our supported SVG elements and attributes */
typedef enum {
    SVG_COLOR_UNKNOWN,
    SVG_COLOR_ALICEBLUE,
	SVG_COLOR_ANTIQUEWHITE,
	SVG_COLOR_AQUA,
	SVG_COLOR_AQUAMARINE,
	SVG_COLOR_AZURE,
	SVG_COLOR_BEIGE,
	SVG_COLOR_BISQUE,
	SVG_COLOR_BLACK,
	SVG_COLOR_BLANCHEDALMOND,
	SVG_COLOR_BLUE,
	SVG_COLOR_BLUEVIOLET,
	SVG_COLOR_BROWN,
	SVG_COLOR_BURLYWOOD,
	SVG_COLOR_CADETBLUE,
	SVG_COLOR_CHARTREUSE,
	SVG_COLOR_CHOCOLATE,
	SVG_COLOR_CORAL,
	SVG_COLOR_CORNFLOWERBLUE,
	SVG_COLOR_CORNSILK,
	SVG_COLOR_CRIMSON,
	SVG_COLOR_CYAN,
	SVG_COLOR_DARKBLUE,
	SVG_COLOR_DARKCYAN,
	SVG_COLOR_DARKGOLDENROD,
	SVG_COLOR_DARKGRAY,
	SVG_COLOR_DARKGREEN,
	SVG_COLOR_DARKGREY,
	SVG_COLOR_DARKKHAKI,
	SVG_COLOR_DARKMAGENTA,
	SVG_COLOR_DARKOLIVEGREEN,
	SVG_COLOR_DARKORANGE,
	SVG_COLOR_DARKORCHID,
	SVG_COLOR_DARKRED,
	SVG_COLOR_DARKSALMON,
	SVG_COLOR_DARKSEAGREEN,
	SVG_COLOR_DARKSLATEBLUE,
	SVG_COLOR_DARKSLATEGRAY,
	SVG_COLOR_DARKSLATEGREY,
	SVG_COLOR_DARKTURQUOISE,
	SVG_COLOR_DARKVIOLET,
	SVG_COLOR_DEEPPINK,
	SVG_COLOR_DEEPSKYBLUE,
	SVG_COLOR_DIMGRAY,
	SVG_COLOR_DIMGREY,
	SVG_COLOR_DODGERBLUE,
	SVG_COLOR_FIREBRICK,
	SVG_COLOR_FLORALWHITE,
	SVG_COLOR_FORESTGREEN,
	SVG_COLOR_FUCHSIA,
	SVG_COLOR_GAINSBORO,
	SVG_COLOR_GHOSTWHITE,
	SVG_COLOR_GOLD,
	SVG_COLOR_GOLDENROD,
	SVG_COLOR_GRAY,
	SVG_COLOR_GREEN,
	SVG_COLOR_GREENYELLOW,
	SVG_COLOR_GREY,
	SVG_COLOR_HONEYDEW,
	SVG_COLOR_HOTPINK,
	SVG_COLOR_INDIANRED,
	SVG_COLOR_INDIGO,
	SVG_COLOR_IVORY,
	SVG_COLOR_KHAKI,
	SVG_COLOR_LAVENDER,
	SVG_COLOR_LAVENDERBLUSH,
	SVG_COLOR_LAWNGREEN,
	SVG_COLOR_LEMONCHIFFON,
	SVG_COLOR_LIGHTBLUE,
	SVG_COLOR_LIGHTCORAL,
	SVG_COLOR_LIGHTCYAN,
	SVG_COLOR_LIGHTGOLDENRODYELLOW,
	SVG_COLOR_LIGHTGRAY,
	SVG_COLOR_LIGHTGREEN,
	SVG_COLOR_LIGHTGREY,
	SVG_COLOR_LIGHTPINK,
	SVG_COLOR_LIGHTSALMON,
	SVG_COLOR_LIGHTSEAGREEN,
	SVG_COLOR_LIGHTSKYBLUE,
	SVG_COLOR_LIGHTSLATEGRAY,
	SVG_COLOR_LIGHTSLATEGREY,
	SVG_COLOR_LIGHTSTEELBLUE,
	SVG_COLOR_LIGHTYELLOW,
	SVG_COLOR_LIME,
	SVG_COLOR_LIMEGREEN,
	SVG_COLOR_LINEN,
	SVG_COLOR_MAGENTA,
	SVG_COLOR_MAROON,
	SVG_COLOR_MEDIUMAQUAMARINE,
	SVG_COLOR_MEDIUMBLUE,
	SVG_COLOR_MEDIUMORCHID,
	SVG_COLOR_MEDIUMPURPLE,
	SVG_COLOR_MEDIUMSEAGREEN,
	SVG_COLOR_MEDIUMSLATEBLUE,
	SVG_COLOR_MEDIUMSPRINGGREEN,
	SVG_COLOR_MEDIUMTURQUOISE,
	SVG_COLOR_MEDIUMVIOLETRED,
	SVG_COLOR_MIDNIGHTBLUE,
	SVG_COLOR_MINTCREAM,
	SVG_COLOR_MISTYROSE,
	SVG_COLOR_MOCCASIN,
	SVG_COLOR_NAVAJOWHITE,
	SVG_COLOR_NAVY,
	SVG_COLOR_OLDLACE,
	SVG_COLOR_OLIVE,
	SVG_COLOR_OLIVEDRAB,
	SVG_COLOR_ORANGE,
	SVG_COLOR_ORANGERED,
	SVG_COLOR_ORCHID,
	SVG_COLOR_PALEGOLDENROD,
	SVG_COLOR_PALEGREEN,
	SVG_COLOR_PALETURQUOISE,
	SVG_COLOR_PALEVIOLETRED,
	SVG_COLOR_PAPAYAWHIP,
	SVG_COLOR_PEACHPUFF,
	SVG_COLOR_PERU,
	SVG_COLOR_PINK,
	SVG_COLOR_PLUM,
	SVG_COLOR_POWDERBLUE,
	SVG_COLOR_PURPLE,
	SVG_COLOR_RED,
	SVG_COLOR_ROSYBROWN,
	SVG_COLOR_ROYALBLUE,
	SVG_COLOR_SADDLEBROWN,
	SVG_COLOR_SALMON,
	SVG_COLOR_SANDYBROWN,
	SVG_COLOR_SEAGREEN,
	SVG_COLOR_SEASHELL,
	SVG_COLOR_SIENNA,
	SVG_COLOR_SILVER,
	SVG_COLOR_SKYBLUE,
	SVG_COLOR_SLATEBLUE,
	SVG_COLOR_SLATEGRAY,
	SVG_COLOR_SLATEGREY,
	SVG_COLOR_SNOW,
	SVG_COLOR_SPRINGGREEN,
	SVG_COLOR_STEELBLUE,
	SVG_COLOR_TAN,
	SVG_COLOR_TEAL,
	SVG_COLOR_THISTLE,
	SVG_COLOR_TOMATO,
	SVG_COLOR_TURQUOISE,
	SVG_COLOR_VIOLET,
	SVG_COLOR_WHEAT,
	SVG_COLOR_WHITE,
	SVG_COLOR_WHITESMOKE,
	SVG_COLOR_YELLOW,
	SVG_COLOR_YELLOWGREEN
} SvgColorTokId;

/* Structure returned by gperf */
struct SvgColorName {
    const char   *name;
    SvgColorTokId id;
    uint32_t      value;
};
#line 174 "src/svg_colors_names.gperf"
struct SvgColorName;

#define SVG_COL__TOTAL_KEYWORDS 147
#define SVG_COL__MIN_WORD_LENGTH 28
#define SVG_COL__MAX_WORD_LENGTH 62
#define SVG_COL__MIN_HASH_VALUE 83
#define SVG_COL__MAX_HASH_VALUE 531
/* maximum key range = 449, duplicates = 0 */

#ifndef GPERF_DOWNCASE
#define GPERF_DOWNCASE 1
static const unsigned char gperf_downcase[256] =
  {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
  };
#endif

#ifndef GPERF_CASE_STRCMP
#define GPERF_CASE_STRCMP 1
static int
gperf_case_strcmp (register const char *s1, register const char *s2)
{
  for (;;)
    {
      unsigned char c1 = gperf_downcase[(unsigned char)*s1++];
      unsigned char c2 = gperf_downcase[(unsigned char)*s2++];
      if (c1 != 0 && c1 == c2)
        continue;
      return (int)c1 - (int)c2;
    }
}
#endif

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_svg_color (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532,   5,   5, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532,   5,  60,  60,  55,   0,
      100,  25,   5,  35,  20, 100,  10,  95,  50,   0,
       50, 130,  10,  30,   0, 105,  10, 185, 170, 210,
       30, 532, 532, 532, 532,   0, 532,   5,  60,  60,
       55,   0, 100,  25,   5,  35,  20, 100,  10,  95,
       50,   0,  50, 130,  10,  30,   0, 105,  10, 185,
      170, 210,  30, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532, 532, 532, 532,
      532, 532, 532, 532, 532, 532, 532
    };
  return len + asso_values[(unsigned char)str[12]] + asso_values[(unsigned char)str[11]] + asso_values[(unsigned char)str[7]] + asso_values[(unsigned char)str[6]] + asso_values[(unsigned char)str[5]+1] + asso_values[(unsigned char)str[3]] + asso_values[(unsigned char)str[2]];
}

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static const struct SvgColorName svg_color_wordlist[] =
  {
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""},
#line 240 "src/svg_colors_names.gperf"
    {"lavender SVG_COLOR_LAVENDER 0xe6e6faff"},
    {""}, {""}, {""},
#line 231 "src/svg_colors_names.gperf"
    {"green SVG_COLOR_GREEN 0x008000ff"},
    {""}, {""},
#line 314 "src/svg_colors_names.gperf"
    {"teal SVG_COLOR_TEAL 0x008080ff"},
    {""},
#line 319 "src/svg_colors_names.gperf"
    {"wheat SVG_COLOR_WHEAT 0xf5deb3ff"},
    {""}, {""},
#line 309 "src/svg_colors_names.gperf"
    {"slategrey SVG_COLOR_SLATEGREY 0x708090ff"},
    {""},
#line 238 "src/svg_colors_names.gperf"
    {"ivory SVG_COLOR_IVORY 0xfffff0ff"},
    {""}, {""},
#line 308 "src/svg_colors_names.gperf"
    {"slategray SVG_COLOR_SLATEGRAY 0x708090ff"},
    {""},
#line 193 "src/svg_colors_names.gperf"
    {"coral SVG_COLOR_CORAL 0xff7f50ff"},
#line 241 "src/svg_colors_names.gperf"
    {"lavenderblush SVG_COLOR_LAVENDERBLUSH 0xfff0f5ff"},
    {""},
#line 256 "src/svg_colors_names.gperf"
    {"lightslategrey SVG_COLOR_LIGHTSLATEGREY 0x778899ff"},
    {""}, {""}, {""}, {""},
#line 255 "src/svg_colors_names.gperf"
    {"lightslategray SVG_COLOR_LIGHTSLATEGRAY 0x778899ff"},
    {""}, {""}, {""}, {""},
#line 286 "src/svg_colors_names.gperf"
    {"palegreen SVG_COLOR_PALEGREEN 0x98fb98ff"},
    {""},
#line 247 "src/svg_colors_names.gperf"
    {"lightgoldenrodyellow SVG_COLOR_LIGHTGOLDENRODYELLOW 0xfafad2ff"},
    {""},
#line 232 "src/svg_colors_names.gperf"
    {"greenyellow SVG_COLOR_GREENYELLOW 0xadff2fff"},
#line 250 "src/svg_colors_names.gperf"
    {"lightgrey SVG_COLOR_LIGHTGREY 0xd3d3d3ff"},
    {""},
#line 320 "src/svg_colors_names.gperf"
    {"white SVG_COLOR_WHITE 0xffffffff"},
    {""},
#line 252 "src/svg_colors_names.gperf"
    {"lightsalmon SVG_COLOR_LIGHTSALMON 0xffa07aff"},
#line 248 "src/svg_colors_names.gperf"
    {"lightgray SVG_COLOR_LIGHTGRAY 0xd3d3d3ff"},
    {""},
#line 249 "src/svg_colors_names.gperf"
    {"lightgreen SVG_COLOR_LIGHTGREEN 0x90ee90ff"},
    {""}, {""},
#line 197 "src/svg_colors_names.gperf"
    {"cyan SVG_COLOR_CYAN 0x00ffffff"},
    {""},
#line 280 "src/svg_colors_names.gperf"
    {"olive SVG_COLOR_OLIVE 0x808000ff"},
#line 253 "src/svg_colors_names.gperf"
    {"lightseagreen SVG_COLOR_LIGHTSEAGREEN 0x20b2aaff"},
    {""},
#line 281 "src/svg_colors_names.gperf"
    {"olivedrab SVG_COLOR_OLIVEDRAB 0x6b8e23ff"},
    {""},
#line 261 "src/svg_colors_names.gperf"
    {"linen SVG_COLOR_LINEN 0xfaf0e6ff"},
    {""},
#line 263 "src/svg_colors_names.gperf"
    {"maroon SVG_COLOR_MAROON 0x800000ff"},
#line 228 "src/svg_colors_names.gperf"
    {"gold SVG_COLOR_GOLD 0xffd700ff"},
    {""}, {""},
#line 288 "src/svg_colors_names.gperf"
    {"palevioletred SVG_COLOR_PALEVIOLETRED 0xdb7093ff"},
    {""}, {""}, {""},
#line 182 "src/svg_colors_names.gperf"
    {"beige SVG_COLOR_BEIGE 0xf5f5dcff"},
    {""},
#line 258 "src/svg_colors_names.gperf"
    {"lightyellow SVG_COLOR_LIGHTYELLOW 0xffffeeff"},
#line 229 "src/svg_colors_names.gperf"
    {"goldenrod SVG_COLOR_GOLDENROD 0xdaa520ff"},
    {""},
#line 184 "src/svg_colors_names.gperf"
    {"black SVG_COLOR_BLACK 0x000000ff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 274 "src/svg_colors_names.gperf"
    {"mintcream SVG_COLOR_MINTCREAM 0xf5fffaff"},
    {""}, {""}, {""},
#line 311 "src/svg_colors_names.gperf"
    {"springgreen SVG_COLOR_SPRINGGREEN 0x00ff7fff"},
#line 275 "src/svg_colors_names.gperf"
    {"mistyrose SVG_COLOR_MISTYROSE 0xffe4e1ff"},
#line 273 "src/svg_colors_names.gperf"
    {"midnightblue SVG_COLOR_MIDNIGHTBLUE 0x191970ff"},
    {""}, {""}, {""},
#line 259 "src/svg_colors_names.gperf"
    {"lime SVG_COLOR_LIME 0x00ff00ff"},
    {""}, {""}, {""}, {""},
#line 236 "src/svg_colors_names.gperf"
    {"indianred SVG_COLOR_INDIANRED 0xcd5c5cff"},
    {""},
#line 245 "src/svg_colors_names.gperf"
    {"lightcoral SVG_COLOR_LIGHTCORAL 0xf08080ff"},
    {""},
#line 305 "src/svg_colors_names.gperf"
    {"silver SVG_COLOR_SILVER 0xc0c0c0ff"},
#line 186 "src/svg_colors_names.gperf"
    {"blue SVG_COLOR_BLUE 0x0000ffff"},
    {""}, {""}, {""}, {""},
#line 179 "src/svg_colors_names.gperf"
    {"aqua SVG_COLOR_AQUA 0x00ffffff"},
    {""}, {""}, {""}, {""},
#line 291 "src/svg_colors_names.gperf"
    {"peru SVG_COLOR_PERU 0xcd853fff"},
#line 225 "src/svg_colors_names.gperf"
    {"fuchsia SVG_COLOR_FUCHSIA 0xff00ffff"},
#line 239 "src/svg_colors_names.gperf"
    {"khaki SVG_COLOR_KHAKI 0xf0e68cff"},
    {""},
#line 284 "src/svg_colors_names.gperf"
    {"orchid SVG_COLOR_ORCHID 0xda70d6ff"},
#line 257 "src/svg_colors_names.gperf"
    {"lightsteelblue SVG_COLOR_LIGHTSTEELBLUE 0xb0c4deff"},
    {""}, {""}, {""}, {""},
#line 260 "src/svg_colors_names.gperf"
    {"limegreen SVG_COLOR_LIMEGREEN 0x32cd32ff"},
    {""},
#line 181 "src/svg_colors_names.gperf"
    {"azure SVG_COLOR_AZURE 0xf0ffffff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 222 "src/svg_colors_names.gperf"
    {"firebrick SVG_COLOR_FIREBRICK 0xb22222ff"},
    {""},
#line 321 "src/svg_colors_names.gperf"
    {"whitesmoke SVG_COLOR_WHITESMOKE 0xf5f5f5ff"},
    {""}, {""},
#line 202 "src/svg_colors_names.gperf"
    {"darkgreen SVG_COLOR_DARKGREEN 0x006400ff"},
    {""},
#line 187 "src/svg_colors_names.gperf"
    {"blueviolet SVG_COLOR_BLUEVIOLET 0x8a2be2ff"},
#line 303 "src/svg_colors_names.gperf"
    {"seashell SVG_COLOR_SEASHELL 0xfff5eeff"},
    {""},
#line 268 "src/svg_colors_names.gperf"
    {"mediumseagreen SVG_COLOR_MEDIUMSEAGREEN 0x3cb371ff"},
#line 279 "src/svg_colors_names.gperf"
    {"oldlace SVG_COLOR_OLDLACE 0xfdf5e6ff"},
#line 216 "src/svg_colors_names.gperf"
    {"darkviolet SVG_COLOR_DARKVIOLET 0x9400d3ff"},
    {""}, {""},
#line 292 "src/svg_colors_names.gperf"
    {"pink SVG_COLOR_PINK 0xffc0cbff"},
    {""}, {""},
#line 285 "src/svg_colors_names.gperf"
    {"palegoldenrod SVG_COLOR_PALEGOLDENROD 0xeee8aaff"},
#line 224 "src/svg_colors_names.gperf"
    {"forestgreen SVG_COLOR_FORESTGREEN 0x228b22ff"},
#line 226 "src/svg_colors_names.gperf"
    {"gainsboro SVG_COLOR_GAINSBORO 0xdcdcdcff"},
    {""},
#line 191 "src/svg_colors_names.gperf"
    {"chartreuse SVG_COLOR_CHARTREUSE 0x7fff00ff"},
    {""},
#line 300 "src/svg_colors_names.gperf"
    {"salmon SVG_COLOR_SALMON 0xfa8072ff"},
#line 192 "src/svg_colors_names.gperf"
    {"chocolate SVG_COLOR_CHOCOLATE 0xd2691eff"},
#line 262 "src/svg_colors_names.gperf"
    {"magenta SVG_COLOR_MAGENTA 0xff00ffff"},
    {""},
#line 287 "src/svg_colors_names.gperf"
    {"paleturquoise SVG_COLOR_PALETURQUOISE 0xafeeeeff"},
#line 304 "src/svg_colors_names.gperf"
    {"sienna SVG_COLOR_SIENNA 0xa0522dff"},
#line 283 "src/svg_colors_names.gperf"
    {"orangered SVG_COLOR_ORANGERED 0xff4500ff"},
    {""}, {""},
#line 302 "src/svg_colors_names.gperf"
    {"seagreen SVG_COLOR_SEAGREEN 0x2e8b57ff"},
#line 318 "src/svg_colors_names.gperf"
    {"violet SVG_COLOR_VIOLET 0xee82eeff"},
#line 185 "src/svg_colors_names.gperf"
    {"blanchedalmond SVG_COLOR_BLANCHEDALMOND 0xffebcdff"},
    {""},
#line 272 "src/svg_colors_names.gperf"
    {"mediumvioletred SVG_COLOR_MEDIUMVIOLETRED 0xc71585ff"},
    {""}, {""},
#line 312 "src/svg_colors_names.gperf"
    {"steelblue SVG_COLOR_STEELBLUE 0x4682b4ff"},
#line 235 "src/svg_colors_names.gperf"
    {"hotpink SVG_COLOR_HOTPINK 0xff69b4ff"},
    {""},
#line 195 "src/svg_colors_names.gperf"
    {"cornsilk SVG_COLOR_CORNSILK 0xfff8dcff"},
    {""},
#line 307 "src/svg_colors_names.gperf"
    {"slateblue SVG_COLOR_SLATEBLUE 0x6a5acdff"},
#line 266 "src/svg_colors_names.gperf"
    {"mediumorchid SVG_COLOR_MEDIUMORCHID 0xba55d3ff"},
#line 301 "src/svg_colors_names.gperf"
    {"sandybrown SVG_COLOR_SANDYBROWN 0xf4a460ff"},
#line 199 "src/svg_colors_names.gperf"
    {"darkcyan SVG_COLOR_DARKCYAN 0x008b8bff"},
    {""},
#line 310 "src/svg_colors_names.gperf"
    {"snow SVG_COLOR_SNOW 0xfffafaff"},
#line 315 "src/svg_colors_names.gperf"
    {"thistle SVG_COLOR_THISTLE 0xd8bfd8ff"},
#line 221 "src/svg_colors_names.gperf"
    {"dodgerblue SVG_COLOR_DODGERBLUE 0x1e90ffff"},
    {""}, {""},
#line 189 "src/svg_colors_names.gperf"
    {"burlywood SVG_COLOR_BURLYWOOD 0xdeb887ff"},
    {""}, {""},
#line 276 "src/svg_colors_names.gperf"
    {"moccasin SVG_COLOR_MOCCASIN 0xffe4b5ff"},
#line 237 "src/svg_colors_names.gperf"
    {"indigo SVG_COLOR_INDIGO 0x4b0082ff"},
    {""}, {""},
#line 188 "src/svg_colors_names.gperf"
    {"brown SVG_COLOR_BROWN 0xa52a2aff"},
    {""},
#line 205 "src/svg_colors_names.gperf"
    {"darkmagenta SVG_COLOR_DARKMAGENTA 0x8b008bff"},
#line 293 "src/svg_colors_names.gperf"
    {"plum SVG_COLOR_PLUM 0xdda0ddff"},
    {""},
#line 207 "src/svg_colors_names.gperf"
    {"darkorange SVG_COLOR_DARKORANGE 0xff8c00ff"},
    {""},
#line 316 "src/svg_colors_names.gperf"
    {"tomato SVG_COLOR_TOMATO 0xff6347ff"},
#line 244 "src/svg_colors_names.gperf"
    {"lightblue SVG_COLOR_LIGHTBLUE 0xadd8e6ff"},
#line 178 "src/svg_colors_names.gperf"
    {"antiquewhite SVG_COLOR_ANTIQUEWHITE 0xfaebd7ff"},
    {""},
#line 217 "src/svg_colors_names.gperf"
    {"deeppink SVG_COLOR_DEEPPINK 0xff1493ff"},
#line 282 "src/svg_colors_names.gperf"
    {"orange SVG_COLOR_ORANGE 0xffa500ff"},
#line 233 "src/svg_colors_names.gperf"
    {"grey SVG_COLOR_GREY 0x808080ff"},
    {""},
#line 208 "src/svg_colors_names.gperf"
    {"darkorchid SVG_COLOR_DARKORCHID 0x9932ccff"},
    {""},
#line 295 "src/svg_colors_names.gperf"
    {"purple SVG_COLOR_PURPLE 0x800080ff"},
#line 230 "src/svg_colors_names.gperf"
    {"gray SVG_COLOR_GRAY 0x808080ff"},
#line 243 "src/svg_colors_names.gperf"
    {"lemonchiffon SVG_COLOR_LEMONCHIFFON 0xfffacdff"},
#line 265 "src/svg_colors_names.gperf"
    {"mediumblue SVG_COLOR_MEDIUMBLUE 0x0000cdff"},
    {""}, {""},
#line 278 "src/svg_colors_names.gperf"
    {"navy SVG_COLOR_NAVY 0x000080ff"},
    {""},
#line 180 "src/svg_colors_names.gperf"
    {"aquamarine SVG_COLOR_AQUAMARINE 0x7fffd4ff"},
#line 234 "src/svg_colors_names.gperf"
    {"honeydew SVG_COLOR_HONEYDEW 0xf0fff0ff"},
    {""},
#line 206 "src/svg_colors_names.gperf"
    {"darkolivegreen SVG_COLOR_DARKOLIVEGREEN 0x556b2fff"},
    {""},
#line 269 "src/svg_colors_names.gperf"
    {"mediumslateblue SVG_COLOR_MEDIUMSLATEBLUE 0x7b68eeff"},
#line 313 "src/svg_colors_names.gperf"
    {"tan SVG_COLOR_TAN 0xd2b48cff"},
#line 323 "src/svg_colors_names.gperf"
    {"yellowgreen SVG_COLOR_YELLOWGREEN 0x9acd32ff"},
#line 190 "src/svg_colors_names.gperf"
    {"cadetblue SVG_COLOR_CADETBLUE 0x5f9ea0ff"},
    {""}, {""},
#line 296 "src/svg_colors_names.gperf"
    {"red SVG_COLOR_RED 0xff0000ff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
#line 322 "src/svg_colors_names.gperf"
    {"yellow SVG_COLOR_YELLOW 0xffff00ff"},
#line 251 "src/svg_colors_names.gperf"
    {"lightpink SVG_COLOR_LIGHTPINK 0xffb6c1ff"},
    {""},
#line 227 "src/svg_colors_names.gperf"
    {"ghostwhite SVG_COLOR_GHOSTWHITE 0xf8f8ffff"},
    {""}, {""},
#line 204 "src/svg_colors_names.gperf"
    {"darkkhaki SVG_COLOR_DARKKHAKI 0xbdb76bff"},
#line 270 "src/svg_colors_names.gperf"
    {"mediumspringgreen SVG_COLOR_MEDIUMSPRINGGREEN 0x00fa9aff"},
    {""},
#line 200 "src/svg_colors_names.gperf"
    {"darkgoldenrod SVG_COLOR_DARKGOLDENROD 0xb8860bff"},
    {""},
#line 317 "src/svg_colors_names.gperf"
    {"turquoise SVG_COLOR_TURQUOISE 0x40e0d0ff"},
#line 196 "src/svg_colors_names.gperf"
    {"crimson SVG_COLOR_CRIMSON 0xdc143cff"},
#line 271 "src/svg_colors_names.gperf"
    {"mediumturquoise SVG_COLOR_MEDIUMTURQUOISE 0x48d1ccff"},
    {""},
#line 277 "src/svg_colors_names.gperf"
    {"navajowhite SVG_COLOR_NAVAJOWHITE 0xffdeadff"},
    {""}, {""}, {""},
#line 215 "src/svg_colors_names.gperf"
    {"darkturquoise SVG_COLOR_DARKTURQUOISE 0x00ced1ff"},
    {""},
#line 242 "src/svg_colors_names.gperf"
    {"lawngreen SVG_COLOR_LAWNGREEN 0x7cfc00ff"},
#line 211 "src/svg_colors_names.gperf"
    {"darkseagreen SVG_COLOR_DARKSEAGREEN 0x8fbc8fff"},
    {""}, {""},
#line 264 "src/svg_colors_names.gperf"
    {"mediumaquamarine SVG_COLOR_MEDIUMAQUAMARINE 0x66cdaaff"},
#line 177 "src/svg_colors_names.gperf"
    {"aliceblue SVG_COLOR_ALICEBLUE 0xf0f8ffff"},
#line 267 "src/svg_colors_names.gperf"
    {"mediumpurple SVG_COLOR_MEDIUMPURPLE 0x9370dbff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
#line 210 "src/svg_colors_names.gperf"
    {"darksalmon SVG_COLOR_DARKSALMON 0xe9967aff"},
    {""},
#line 299 "src/svg_colors_names.gperf"
    {"saddlebrown SVG_COLOR_SADDLEBROWN 0x8b4513ff"},
    {""}, {""}, {""},
#line 212 "src/svg_colors_names.gperf"
    {"darkslateblue SVG_COLOR_DARKSLATEBLUE 0x483d8bff"},
    {""}, {""},
#line 209 "src/svg_colors_names.gperf"
    {"darkred SVG_COLOR_DARKRED 0x8b0000ff"},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 198 "src/svg_colors_names.gperf"
    {"darkblue SVG_COLOR_DARKBLUE 0x00008bff"},
#line 223 "src/svg_colors_names.gperf"
    {"floralwhite SVG_COLOR_FLORALWHITE 0xfffaf0ff"},
#line 246 "src/svg_colors_names.gperf"
    {"lightcyan SVG_COLOR_LIGHTCYAN 0xe0ffffff"},
    {""}, {""}, {""}, {""}, {""},
#line 306 "src/svg_colors_names.gperf"
    {"skyblue SVG_COLOR_SKYBLUE 0x87ceebff"},
    {""}, {""}, {""}, {""}, {""},
#line 289 "src/svg_colors_names.gperf"
    {"papayawhip SVG_COLOR_PAPAYAWHIP 0xffefd5ff"},
    {""},
#line 183 "src/svg_colors_names.gperf"
    {"bisque SVG_COLOR_BISQUE 0xffe4c4ff"},
    {""},
#line 254 "src/svg_colors_names.gperf"
    {"lightskyblue SVG_COLOR_LIGHTSKYBLUE 0x87cefaff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 218 "src/svg_colors_names.gperf"
    {"deepskyblue SVG_COLOR_DEEPSKYBLUE 0x00bfffff"},
    {""}, {""}, {""},
#line 203 "src/svg_colors_names.gperf"
    {"darkgrey SVG_COLOR_DARKGREY 0xa9a9a9ff"},
    {""}, {""}, {""}, {""},
#line 201 "src/svg_colors_names.gperf"
    {"darkgray SVG_COLOR_DARKGRAY 0xa9a9a9ff"},
    {""}, {""}, {""},
#line 294 "src/svg_colors_names.gperf"
    {"powderblue SVG_COLOR_POWDERBLUE 0xb0e0e6ff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 298 "src/svg_colors_names.gperf"
    {"royalblue SVG_COLOR_ROYALBLUE 0x4169e1ff"},
    {""}, {""},
#line 214 "src/svg_colors_names.gperf"
    {"darkslategrey SVG_COLOR_DARKSLATEGREY 0x2f4f4fff"},
    {""}, {""}, {""}, {""},
#line 213 "src/svg_colors_names.gperf"
    {"darkslategray SVG_COLOR_DARKSLATEGRAY 0x2f4f4fff"},
    {""},
#line 290 "src/svg_colors_names.gperf"
    {"peachpuff SVG_COLOR_PEACHPUFF 0xffdab9ff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 219 "src/svg_colors_names.gperf"
    {"dimgray SVG_COLOR_DIMGRAY 0x696969ff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""},
#line 194 "src/svg_colors_names.gperf"
    {"cornflowerblue SVG_COLOR_CORNFLOWERBLUE 0x6495edff"},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 297 "src/svg_colors_names.gperf"
    {"rosybrown SVG_COLOR_ROSYBROWN 0xbc8f8fff"},
#line 220 "src/svg_colors_names.gperf"
    {"dimgrey SVG_COLOR_DIMGREY 0x696969ff"}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

const struct SvgColorName *
lookup_svg_color_names (register const char *str, register size_t len)
{
  if (len <= SVG_COL__MAX_WORD_LENGTH && len >= SVG_COL__MIN_WORD_LENGTH)
    {
      register unsigned int key = hash_svg_color (str, len);

      if (key <= SVG_COL__MAX_HASH_VALUE)
        {
          register const char *s = svg_color_wordlist[key].name;

          if ((((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !gperf_case_strcmp (str, s))
            return &svg_color_wordlist[key];
        }
    }
  return (struct SvgColorName *) 0;
}
#line 324 "src/svg_colors_names.gperf"

