/* ANSI-C code produced by gperf version 3.2.1 */
/* Command-line: gperf --output-file=build/svg_elt_gperf.h src/svg_elt_keywords.gperf  */
/* Computed positions: -k'1,3,$' */

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

#line 1 "src/svg_elt_keywords.gperf"

#include <string.h>

/* Enum representing our supported SVG elements and attributes */
typedef enum {
    SVG_TOK_SVG,
    SVG_TOK_G,
    SVG_TOK_DEFS,
    SVG_TOK_DESC,
    SVG_TOK_TITLE,
    SVG_TOK_SYMBOL,
    SVG_TOK_USE,
    SVG_TOK_SWITCH,
    SVG_TOK_IMAGE,
    SVG_TOK_STYLE,
    SVG_TOK_PATH,
    SVG_TOK_RECT,
    SVG_TOK_CIRCLE,
    SVG_TOK_LINE,
    SVG_TOK_ELLIPSE,
    SVG_TOK_POLYLINE,
    SVG_TOK_POLYGON,
    SVG_TOK_TEXT,
    SVG_TOK_TSPAN,
    SVG_TOK_TREF,
    SVG_TOK_TEXTPATH,
    SVG_TOK_ALTGLYPH,
    SVG_TOK_GLYPHREF,
    SVG_TOK_MARKER,
    SVG_TOK_COLOR_PROFILE,
    SVG_TOK_LINEARGRADIENT,
    SVG_TOK_RADIALGRADIENT,
    SVG_TOK_STOP,
    SVG_TOK_PATTERN,
    SVG_TOK_CLIPPATH,
    SVG_TOK_MASK,
    SVG_TOK_FILTER,
    SVG_TOK_FEBLEND,
    SVG_TOK_FECOLORMATRIX,
    SVG_TOK_FECOMPONENTTRANSFER,
    SVG_TOK_FECOMPOSITE,
    SVG_TOK_FECONVOLVEMATRIX,
    SVG_TOK_FEDIFFUSELIGHTING,
    SVG_TOK_FEDISPLACEMENTMAP,
    SVG_TOK_FEFLOOD,
    SVG_TOK_FEGAUSSIANBLUR,
    SVG_TOK_FEIMAGE,
    SVG_TOK_FEMERGE,
    SVG_TOK_FEMERGENODE,
    SVG_TOK_FEMORPHOLOGY,
    SVG_TOK_FEOFFSET,
    SVG_TOK_FESPECULARLIGHTING,
    SVG_TOK_FETILE,
    SVG_TOK_FETURBULENCE,
    SVG_TOK_FEDISTANTLIGHT,
    SVG_TOK_FEPOINTLIGHT,
    SVG_TOK_FESPOTLIGHT,
    SVG_TOK_FEFUNCR,
    SVG_TOK_FEFUNCG,
    SVG_TOK_FEFUNCB,
    SVG_TOK_FEFUNCA,
    SVG_TOK_CURSOR,
    SVG_TOK_A,
    SVG_TOK_VIEW,
    SVG_TOK_SCRIPT,
    SVG_TOK_ANIMATE,
    SVG_TOK_SET,
    SVG_TOK_ANIMATEMOTION,
    SVG_TOK_ANIMATECOLOR,
    SVG_TOK_ANIMATETRANSFORM,
    SVG_TOK_MPATH,
    SVG_TOK_FONT,
    SVG_TOK_FONT_FACE,
    SVG_TOK_GLYPH,
    SVG_TOK_MISSING_GLYPH,
    SVG_TOK_HKERN,
    SVG_TOK_VKERN,
    SVG_TOK_FONT_FACE_URI,
    SVG_TOK_FONT_FACE_FORMAT,
    SVG_TOK_FONT_FACE_NAME,
    SVG_TOK_FOREIGNOBJECT,
} SvgEltTokId;

/* Structure returned by gperf */
struct SvgEltKeyword {
    const char *name;
    SvgEltTokId id;
};
#line 96 "src/svg_elt_keywords.gperf"
struct SvgEltKeyword;

#define TOTAL_KEYWORDS 76
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 123
/* maximum key range = 123, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124,  20,  25,  15,  35, 124,
        0,  45, 124,  15, 124, 124, 124,  35, 124,   0,
        5, 124,   0,  40,  55, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124,   0, 124,  10,
       40,   0,  25,  40,  40,  20, 124,  60,   5,  20,
        5,  70,   5, 124,   5,   0,   0,   5,   0,   0,
        5,  15, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
      124, 124, 124, 124, 124, 124
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[2]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static const struct SvgEltKeyword wordlist[] =
  {
    {""},
#line 156 "src/svg_elt_keywords.gperf"
    {"a", SVG_TOK_A},
    {""},
#line 160 "src/svg_elt_keywords.gperf"
    {"set", SVG_TOK_SET},
#line 157 "src/svg_elt_keywords.gperf"
    {"view", SVG_TOK_VIEW},
#line 103 "src/svg_elt_keywords.gperf"
    {"title", SVG_TOK_TITLE},
    {""}, {""},
#line 105 "src/svg_elt_keywords.gperf"
    {"use", SVG_TOK_USE},
#line 116 "src/svg_elt_keywords.gperf"
    {"text", SVG_TOK_TEXT},
#line 170 "src/svg_elt_keywords.gperf"
    {"vkern", SVG_TOK_VKERN},
#line 158 "src/svg_elt_keywords.gperf"
    {"script", SVG_TOK_SCRIPT},
#line 113 "src/svg_elt_keywords.gperf"
    {"ellipse", SVG_TOK_ELLIPSE},
    {""},
#line 112 "src/svg_elt_keywords.gperf"
    {"line", SVG_TOK_LINE},
#line 117 "src/svg_elt_keywords.gperf"
    {"tspan", SVG_TOK_TSPAN},
    {""},
#line 127 "src/svg_elt_keywords.gperf"
    {"pattern", SVG_TOK_PATTERN},
#line 114 "src/svg_elt_keywords.gperf"
    {"polyline", SVG_TOK_POLYLINE},
#line 110 "src/svg_elt_keywords.gperf"
    {"rect", SVG_TOK_RECT},
#line 108 "src/svg_elt_keywords.gperf"
    {"style", SVG_TOK_STYLE},
#line 111 "src/svg_elt_keywords.gperf"
    {"circle", SVG_TOK_CIRCLE},
#line 115 "src/svg_elt_keywords.gperf"
    {"polygon", SVG_TOK_POLYGON},
    {""},
#line 124 "src/svg_elt_keywords.gperf"
    {"linearGradient", SVG_TOK_LINEARGRADIENT},
#line 107 "src/svg_elt_keywords.gperf"
    {"image", SVG_TOK_IMAGE},
#line 155 "src/svg_elt_keywords.gperf"
    {"cursor", SVG_TOK_CURSOR},
#line 159 "src/svg_elt_keywords.gperf"
    {"animate", SVG_TOK_ANIMATE},
#line 123 "src/svg_elt_keywords.gperf"
    {"color-profile", SVG_TOK_COLOR_PROFILE},
#line 118 "src/svg_elt_keywords.gperf"
    {"tref", SVG_TOK_TREF},
    {""},
#line 104 "src/svg_elt_keywords.gperf"
    {"symbol", SVG_TOK_SYMBOL},
#line 151 "src/svg_elt_keywords.gperf"
    {"feFuncR", SVG_TOK_FEFUNCR},
#line 144 "src/svg_elt_keywords.gperf"
    {"feOffset", SVG_TOK_FEOFFSET},
#line 165 "src/svg_elt_keywords.gperf"
    {"font", SVG_TOK_FONT},
    {""},
#line 122 "src/svg_elt_keywords.gperf"
    {"marker", SVG_TOK_MARKER},
#line 162 "src/svg_elt_keywords.gperf"
    {"animateColor", SVG_TOK_ANIMATECOLOR},
#line 161 "src/svg_elt_keywords.gperf"
    {"animateMotion", SVG_TOK_ANIMATEMOTION},
#line 166 "src/svg_elt_keywords.gperf"
    {"font-face", SVG_TOK_FONT_FACE},
    {""},
#line 130 "src/svg_elt_keywords.gperf"
    {"filter", SVG_TOK_FILTER},
#line 149 "src/svg_elt_keywords.gperf"
    {"fePointLight", SVG_TOK_FEPOINTLIGHT},
#line 174 "src/svg_elt_keywords.gperf"
    {"foreignObject", SVG_TOK_FOREIGNOBJECT},
#line 173 "src/svg_elt_keywords.gperf"
    {"font-face-name", SVG_TOK_FONT_FACE_NAME},
    {""},
#line 172 "src/svg_elt_keywords.gperf"
    {"font-face-format", SVG_TOK_FONT_FACE_FORMAT},
#line 140 "src/svg_elt_keywords.gperf"
    {"feImage", SVG_TOK_FEIMAGE},
#line 120 "src/svg_elt_keywords.gperf"
    {"altGlyph", SVG_TOK_ALTGLYPH},
#line 109 "src/svg_elt_keywords.gperf"
    {"path", SVG_TOK_PATH},
#line 169 "src/svg_elt_keywords.gperf"
    {"hkern", SVG_TOK_HKERN},
#line 134 "src/svg_elt_keywords.gperf"
    {"feComposite", SVG_TOK_FECOMPOSITE},
#line 154 "src/svg_elt_keywords.gperf"
    {"feFuncA", SVG_TOK_FEFUNCA},
#line 119 "src/svg_elt_keywords.gperf"
    {"textPath", SVG_TOK_TEXTPATH},
#line 102 "src/svg_elt_keywords.gperf"
    {"desc", SVG_TOK_DESC},
    {""},
#line 163 "src/svg_elt_keywords.gperf"
    {"animateTransform", SVG_TOK_ANIMATETRANSFORM},
#line 153 "src/svg_elt_keywords.gperf"
    {"feFuncB", SVG_TOK_FEFUNCB},
#line 132 "src/svg_elt_keywords.gperf"
    {"feColorMatrix", SVG_TOK_FECOLORMATRIX},
#line 125 "src/svg_elt_keywords.gperf"
    {"radialGradient", SVG_TOK_RADIALGRADIENT},
    {""},
#line 135 "src/svg_elt_keywords.gperf"
    {"feConvolveMatrix", SVG_TOK_FECONVOLVEMATRIX},
    {""},
#line 171 "src/svg_elt_keywords.gperf"
    {"font-face-uri", SVG_TOK_FONT_FACE_URI},
#line 133 "src/svg_elt_keywords.gperf"
    {"feComponentTransfer", SVG_TOK_FECOMPONENTTRANSFER},
#line 164 "src/svg_elt_keywords.gperf"
    {"mpath", SVG_TOK_MPATH},
#line 106 "src/svg_elt_keywords.gperf"
    {"switch", SVG_TOK_SWITCH},
#line 141 "src/svg_elt_keywords.gperf"
    {"feMerge", SVG_TOK_FEMERGE},
    {""},
#line 101 "src/svg_elt_keywords.gperf"
    {"defs", SVG_TOK_DEFS},
    {""},
#line 142 "src/svg_elt_keywords.gperf"
    {"feMergeNode", SVG_TOK_FEMERGENODE},
#line 138 "src/svg_elt_keywords.gperf"
    {"feFlood", SVG_TOK_FEFLOOD},
#line 168 "src/svg_elt_keywords.gperf"
    {"missing-glyph", SVG_TOK_MISSING_GLYPH},
#line 148 "src/svg_elt_keywords.gperf"
    {"feDistantLight", SVG_TOK_FEDISTANTLIGHT},
    {""},
#line 150 "src/svg_elt_keywords.gperf"
    {"feSpotLight", SVG_TOK_FESPOTLIGHT},
#line 152 "src/svg_elt_keywords.gperf"
    {"feFuncG", SVG_TOK_FEFUNCG},
#line 128 "src/svg_elt_keywords.gperf"
    {"clipPath", SVG_TOK_CLIPPATH},
#line 126 "src/svg_elt_keywords.gperf"
    {"stop", SVG_TOK_STOP},
    {""},
#line 100 "src/svg_elt_keywords.gperf"
    {"g", SVG_TOK_G},
#line 137 "src/svg_elt_keywords.gperf"
    {"feDisplacementMap", SVG_TOK_FEDISPLACEMENTMAP},
#line 99 "src/svg_elt_keywords.gperf"
    {"svg", SVG_TOK_SVG},
#line 129 "src/svg_elt_keywords.gperf"
    {"mask", SVG_TOK_MASK},
    {""},
#line 146 "src/svg_elt_keywords.gperf"
    {"feTile", SVG_TOK_FETILE},
#line 143 "src/svg_elt_keywords.gperf"
    {"feMorphology", SVG_TOK_FEMORPHOLOGY},
#line 121 "src/svg_elt_keywords.gperf"
    {"glyphRef", SVG_TOK_GLYPHREF},
#line 139 "src/svg_elt_keywords.gperf"
    {"feGaussianBlur", SVG_TOK_FEGAUSSIANBLUR},
    {""}, {""},
#line 147 "src/svg_elt_keywords.gperf"
    {"feTurbulence", SVG_TOK_FETURBULENCE},
    {""}, {""}, {""}, {""},
#line 131 "src/svg_elt_keywords.gperf"
    {"feBlend", SVG_TOK_FEBLEND},
    {""}, {""},
#line 167 "src/svg_elt_keywords.gperf"
    {"glyph", SVG_TOK_GLYPH},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 136 "src/svg_elt_keywords.gperf"
    {"feDiffuseLighting", SVG_TOK_FEDIFFUSELIGHTING},
    {""}, {""}, {""}, {""}, {""},
#line 145 "src/svg_elt_keywords.gperf"
    {"feSpecularLighting", SVG_TOK_FESPECULARLIGHTING}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

const struct SvgEltKeyword *
in_word_set (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return (struct SvgEltKeyword *) 0;
}
#line 175 "src/svg_elt_keywords.gperf"

const struct SvgEltKeyword*
lookup_svg_elt_token (register const char *str, register size_t len)
{
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
        register unsigned int key = hash (str, len);

        if (key <= MAX_HASH_VALUE)
        {
            register const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len -1) && s[len] == '\0')
                return &wordlist[key];
        }
    }
    return (struct SvgEltKeyword *) 0;
}
