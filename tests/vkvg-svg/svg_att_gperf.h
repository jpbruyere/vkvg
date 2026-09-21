/* ANSI-C code produced by gperf version 3.2.1 */
/* Command-line: gperf --output-file=build/svg_att_gperf.h src/svg_att_keywords.gperf  */
/* Computed positions: -k'1,3,8,10,19,28,$' */

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

#line 1 "src/svg_att_keywords.gperf"

#include <string.h>

/* Enum representing our supported SVG elements and attributes */
typedef enum {
	SVG_ATT_TOK_UNKNOWN,
    SVG_ATT_TOK_ACCUMULATE,
    SVG_ATT_TOK_ADDITIVE,
    SVG_ATT_TOK_ATTRIBUTENAME,
    SVG_ATT_TOK_ATTRIBUTETYPE,
    SVG_ATT_TOK_BEGIN,
    SVG_ATT_TOK_BY,
    SVG_ATT_TOK_CALCMODE,
    SVG_ATT_TOK_DUR,
    SVG_ATT_TOK_END,
    SVG_ATT_TOK_FILL,
    SVG_ATT_TOK_FROM,
    SVG_ATT_TOK_KEYPOINTS,
    SVG_ATT_TOK_KEYSPLINES,
    SVG_ATT_TOK_KEYTIMES,
    SVG_ATT_TOK_MAX,
    SVG_ATT_TOK_MIN,
    SVG_ATT_TOK_ORIGIN,
    SVG_ATT_TOK_PATH,
    SVG_ATT_TOK_REPEATCOUNT,
    SVG_ATT_TOK_REPEATDUR,
    SVG_ATT_TOK_RESTART,
    SVG_ATT_TOK_ROTATE,
    SVG_ATT_TOK_TO,
    SVG_ATT_TOK_TYPE,
    SVG_ATT_TOK_VALUES,
    SVG_ATT_TOK_CX,
    SVG_ATT_TOK_CY,
    SVG_ATT_TOK_R,
    SVG_ATT_TOK_CLIPPATHUNITS,
    SVG_ATT_TOK_REQUIREDEXTENSIONS,
    SVG_ATT_TOK_REQUIREDFEATURES,
    SVG_ATT_TOK_SYSTEMLANGUAGE,
    SVG_ATT_TOK_CLASS,
    SVG_ATT_TOK_ID,
    SVG_ATT_TOK_STYLE,
    SVG_ATT_TOK_TABINDEX,
    SVG_ATT_TOK_XML_BASE,
    SVG_ATT_TOK_XML_LANG,
    SVG_ATT_TOK_XML_SPACE,
    SVG_ATT_TOK_RX,
    SVG_ATT_TOK_RY,
    SVG_ATT_TOK_FEBLEND_MODE,
    SVG_ATT_TOK_FECOLORMATRIX_TYPE,
    SVG_ATT_TOK_FECOLORMATRIX_VALUES,
    SVG_ATT_TOK_FECOMPONENTTRANSFER_TYPE,
    SVG_ATT_TOK_FECOMPOSITE_K1,
    SVG_ATT_TOK_FECOMPOSITE_K2,
    SVG_ATT_TOK_FECOMPOSITE_K3,
    SVG_ATT_TOK_FECOMPOSITE_K4,
    SVG_ATT_TOK_FECOMPOSITE_OPERATOR,
    SVG_ATT_TOK_FECONVOLVEMATRIX_BIAS,
    SVG_ATT_TOK_FECONVOLVEMATRIX_DIVISOR,
    SVG_ATT_TOK_FECONVOLVEMATRIX_EDGEMODE,
    SVG_ATT_TOK_FECONVOLVEMATRIX_KERNELMATRIX,
    SVG_ATT_TOK_FECONVOLVEMATRIX_KERNELUNITLENGTH,
    SVG_ATT_TOK_FECONVOLVEMATRIX_ORDER,
    SVG_ATT_TOK_FECONVOLVEMATRIX_PRESERVEALPHA,
    SVG_ATT_TOK_FECONVOLVEMATRIX_TARGETX,
    SVG_ATT_TOK_FECONVOLVEMATRIX_TARGETY,
    SVG_ATT_TOK_FEDIFFUSELIGHTING_DIFFUSECONSTANT,
    SVG_ATT_TOK_FEDIFFUSELIGHTING_KERNELUNITLENGTH,
    SVG_ATT_TOK_FEDIFFUSELIGHTING_SURFACESCALE,
    SVG_ATT_TOK_FEDISPLACEMENTMAP_SCALE,
    SVG_ATT_TOK_FEDISPLACEMENTMAP_XCHANNELSELECTOR,
    SVG_ATT_TOK_FEDISPLACEMENTMAP_YCHANNELSELECTOR,
    SVG_ATT_TOK_FEDROPSHADOW_DX,
    SVG_ATT_TOK_FEDROPSHADOW_DY,
    SVG_ATT_TOK_FEDROPSHADOW_STDDEVIATION,
    SVG_ATT_TOK_FEGAUSSIANBLUR_EDGEMODE,
    SVG_ATT_TOK_FEGAUSSIANBLUR_STDDEVIATION,
    SVG_ATT_TOK_FEIMAGE_PRESERVEASPECTRATIO,
    SVG_ATT_TOK_FEMORPHOLOGY_OPERATOR,
    SVG_ATT_TOK_FEMORPHOLOGY_RADIUS,
    SVG_ATT_TOK_FESPECULARLIGHTING_KERNELUNITLENGTH,
    SVG_ATT_TOK_FESPECULARLIGHTING_SPECULARCONSTANT,
    SVG_ATT_TOK_FESPECULARLIGHTING_SPECULAREXPONENT,
    SVG_ATT_TOK_FESPECULARLIGHTING_SURFACESCALE,
    SVG_ATT_TOK_FETILE,
    SVG_ATT_TOK_FETURBULENCE_BASEFREQUENCY,
    SVG_ATT_TOK_FETURBULENCE_NUMOCTAVES,
    SVG_ATT_TOK_FETURBULENCE_SEED,
    SVG_ATT_TOK_FETURBULENCE_STITCHTILES,
    SVG_ATT_TOK_FETURBULENCE_TYPE,
    SVG_ATT_TOK_FILTERUNITS,
    SVG_ATT_TOK_PRIMITIVEUNITS,
    SVG_ATT_TOK_HEIGHT,
    SVG_ATT_TOK_IN,
    SVG_ATT_TOK_IN2,
    SVG_ATT_TOK_RESULT,
    SVG_ATT_TOK_WIDTH,
    SVG_ATT_TOK_X,
    SVG_ATT_TOK_Y,
    SVG_ATT_TOK_CROSSORIGIN,
    SVG_ATT_TOK_HREF,
    SVG_ATT_TOK_PRESERVEASPECTRATIO,
    SVG_ATT_TOK_ONABORT,
    SVG_ATT_TOK_ONACTIVATE,
    SVG_ATT_TOK_ONBEGIN,
    SVG_ATT_TOK_ONCLICK,
    SVG_ATT_TOK_ONEND,
    SVG_ATT_TOK_ONERROR,
    SVG_ATT_TOK_ONFOCUSIN,
    SVG_ATT_TOK_ONFOCUSOUT,
    SVG_ATT_TOK_ONKEYDOWN,
    SVG_ATT_TOK_ONKEYPRESS,
    SVG_ATT_TOK_ONKEYUP,
    SVG_ATT_TOK_ONLOAD,
    SVG_ATT_TOK_ONMOUSEDOWN,
    SVG_ATT_TOK_ONMOUSEMOVE,
    SVG_ATT_TOK_ONMOUSEOUT,
    SVG_ATT_TOK_ONMOUSEOVER,
    SVG_ATT_TOK_ONMOUSEUP,
    SVG_ATT_TOK_ONREPEAT,
    SVG_ATT_TOK_ONRESIZE,
    SVG_ATT_TOK_ONSCROLL,
    SVG_ATT_TOK_ONUNLOAD,
    SVG_ATT_TOK_X1,
    SVG_ATT_TOK_X2,
    SVG_ATT_TOK_Y1,
    SVG_ATT_TOK_Y2,
    SVG_ATT_TOK_GRADIENTTRANSFORM,
    SVG_ATT_TOK_GRADIENTUNITS,
    SVG_ATT_TOK_SPREADMETHOD,
    SVG_ATT_TOK_MARKERHEIGHT,
    SVG_ATT_TOK_MARKERUNITS,
    SVG_ATT_TOK_MARKERWIDTH,
    SVG_ATT_TOK_ORIENT,
    SVG_ATT_TOK_REFX,
    SVG_ATT_TOK_REFY,
    SVG_ATT_TOK_VIEWBOX,
    SVG_ATT_TOK_MASKCONTENTUNITS,
    SVG_ATT_TOK_MASKUNITS,
    SVG_ATT_TOK_D,
    SVG_ATT_TOK_PATHLENGTH,
    SVG_ATT_TOK_PATTERNCONTENTUNITS,
    SVG_ATT_TOK_PATTERNTRANSFORM,
    SVG_ATT_TOK_PATTERNUNITS,
    SVG_ATT_TOK_POINTS,
    SVG_ATT_TOK_ALIGNMENT_BASELINE,
    SVG_ATT_TOK_BASELINE_SHIFT,
    SVG_ATT_TOK_CLIP,
    SVG_ATT_TOK_CLIP_PATH,
    SVG_ATT_TOK_CLIP_RULE,
    SVG_ATT_TOK_COLOR,
    SVG_ATT_TOK_COLOR_INTERPOLATION,
    SVG_ATT_TOK_COLOR_INTERPOLATION_FILTERS,
    SVG_ATT_TOK_COLOR_PROFILE,
    SVG_ATT_TOK_COLOR_RENDERING,
    SVG_ATT_TOK_CURSOR,
    SVG_ATT_TOK_DIRECTION,
    SVG_ATT_TOK_DISPLAY,
    SVG_ATT_TOK_DOMINANT_BASELINE,
    SVG_ATT_TOK_ENABLE_BACKGROUND,
    SVG_ATT_TOK_FILL_OPACITY,
    SVG_ATT_TOK_FILL_RULE,
    SVG_ATT_TOK_FILTER,
    SVG_ATT_TOK_FLOOD_COLOR,
    SVG_ATT_TOK_FLOOD_OPACITY,
    SVG_ATT_TOK_FONT_FAMILY,
    SVG_ATT_TOK_FONT_SIZE,
    SVG_ATT_TOK_FONT_SIZE_ADJUST,
    SVG_ATT_TOK_FONT_STRETCH,
    SVG_ATT_TOK_FONT_STYLE,
    SVG_ATT_TOK_FONT_VARIANT,
    SVG_ATT_TOK_FONT_WEIGHT,
    SVG_ATT_TOK_GLYPH_ORIENTATION_HORIZONTAL,
    SVG_ATT_TOK_GLYPH_ORIENTATION_VERTICAL,
    SVG_ATT_TOK_IMAGE_RENDERING,
    SVG_ATT_TOK_LIGHTING_COLOR,
    SVG_ATT_TOK_MARKER_END,
    SVG_ATT_TOK_MARKER_MID,
    SVG_ATT_TOK_MARKER_START,
    SVG_ATT_TOK_MASK,
    SVG_ATT_TOK_OPACITY,
    SVG_ATT_TOK_OVERFLOW,
    SVG_ATT_TOK_PAINT_ORDER,
    SVG_ATT_TOK_POINTER_EVENTS,
    SVG_ATT_TOK_SHAPE_RENDERING,
    SVG_ATT_TOK_STOP_COLOR,
    SVG_ATT_TOK_STOP_OPACITY,
    SVG_ATT_TOK_STROKE,
    SVG_ATT_TOK_STROKE_DASHARRAY,
    SVG_ATT_TOK_STROKE_DASHOFFSET,
    SVG_ATT_TOK_STROKE_LINECAP,
    SVG_ATT_TOK_STROKE_LINEJOIN,
    SVG_ATT_TOK_STROKE_MITERLIMIT,
    SVG_ATT_TOK_STROKE_OPACITY,
    SVG_ATT_TOK_STROKE_WIDTH,
    SVG_ATT_TOK_TEXT_ANCHOR,
    SVG_ATT_TOK_TEXT_DECORATION,
    SVG_ATT_TOK_TEXT_RENDERING,
    SVG_ATT_TOK_UNICODE_BIDI,
    SVG_ATT_TOK_VECTOR_EFFECT,
    SVG_ATT_TOK_VISIBILITY,
    SVG_ATT_TOK_WORD_SPACING,
    SVG_ATT_TOK_WRITING_MODE,
    SVG_ATT_TOK_FR,
    SVG_ATT_TOK_FX,
    SVG_ATT_TOK_FY,
    SVG_ATT_TOK_OFFSET,
    SVG_ATT_TOK_BASEPROFILE,
    SVG_ATT_TOK_VERSION,
    SVG_ATT_TOK_ZOOMANDPAN,
    SVG_ATT_TOK_DX,
    SVG_ATT_TOK_DY,
    SVG_ATT_TOK_LENGTHADJUST,
    SVG_ATT_TOK_TEXTLENGTH,
    SVG_ATT_TOK_METHOD,
    SVG_ATT_TOK_SIDE,
    SVG_ATT_TOK_SPACING,
    SVG_ATT_TOK_STARTOFFSET,
    SVG_ATT_TOK_XLINK_ACTUATE,
    SVG_ATT_TOK_XLINK_ARCROLE,
    SVG_ATT_TOK_XLINK_HREF,
    SVG_ATT_TOK_XLINK_ROLE,
    SVG_ATT_TOK_XLINK_SHOW,
    SVG_ATT_TOK_XLINK_TITLE,
    SVG_ATT_TOK_XLINK_TYPE
} SvgAttTokId;

/* Structure returned by gperf */
struct SvgAttKeyword {
    const char *name;
    SvgAttTokId id;
};
#line 243 "src/svg_att_keywords.gperf"
struct SvgAttKeyword;

#define ATT_TOTAL_KEYWORDS 218
#define ATT_MIN_WORD_LENGTH 1
#define ATT_MAX_WORD_LENGTH 35
#define ATT_MIN_HASH_VALUE 1
#define ATT_MAX_HASH_VALUE 535
/* maximum key range = 535, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_svg_att (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536,  15, 536, 536, 536, 220,
      185,  55,   0, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536,  20,  85,   5,  30,
      536,  35, 536,  25, 536, 536,  70, 105,   0, 536,
        0, 536, 536,  20, 125,  85, 536, 536,  10,   0,
      536, 536, 536, 536, 536,  15, 536,  60, 200,  80,
        0,  10,  40, 105,  85,  95, 536,   5,   0,  15,
       50,  10,  55,  10,  10,   0,  15,  80,   0, 170,
        5, 120,  35, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536, 536, 536, 536, 536,
      536, 536, 536, 536, 536, 536
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[27]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 27:
      case 26:
      case 25:
      case 24:
      case 23:
      case 22:
      case 21:
      case 20:
      case 19:
        hval += asso_values[(unsigned char)str[18]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 18:
      case 17:
      case 16:
      case 15:
      case 14:
      case 13:
      case 12:
      case 11:
      case 10:
        hval += asso_values[(unsigned char)str[9]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 9:
      case 8:
        hval += asso_values[(unsigned char)str[7]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
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
static const struct SvgAttKeyword svg_att_wordlist[] =
  {
    {""},
#line 378 "src/svg_att_keywords.gperf"
    {"d", SVG_ATT_TOK_D},
    {""}, {""}, {""}, {""},
#line 270 "src/svg_att_keywords.gperf"
    {"values", SVG_ATT_TOK_VALUES},
#line 449 "src/svg_att_keywords.gperf"
    {"dx", SVG_ATT_TOK_DX},
    {""}, {""}, {""},
#line 336 "src/svg_att_keywords.gperf"
    {"x", SVG_ATT_TOK_X},
    {""},
#line 254 "src/svg_att_keywords.gperf"
    {"end", SVG_ATT_TOK_END},
#line 454 "src/svg_att_keywords.gperf"
    {"side", SVG_ATT_TOK_SIDE},
    {""},
#line 352 "src/svg_att_keywords.gperf"
    {"onload", SVG_ATT_TOK_ONLOAD},
#line 285 "src/svg_att_keywords.gperf"
    {"rx", SVG_ATT_TOK_RX},
#line 360 "src/svg_att_keywords.gperf"
    {"onscroll", SVG_ATT_TOK_ONSCROLL},
    {""}, {""},
#line 273 "src/svg_att_keywords.gperf"
    {"r", SVG_ATT_TOK_R},
#line 375 "src/svg_att_keywords.gperf"
    {"viewBox", SVG_ATT_TOK_VIEWBOX},
#line 253 "src/svg_att_keywords.gperf"
    {"dur", SVG_ATT_TOK_DUR},
#line 418 "src/svg_att_keywords.gperf"
    {"mask", SVG_ATT_TOK_MASK},
#line 345 "src/svg_att_keywords.gperf"
    {"onend", SVG_ATT_TOK_ONEND},
#line 426 "src/svg_att_keywords.gperf"
    {"stroke", SVG_ATT_TOK_STROKE},
#line 268 "src/svg_att_keywords.gperf"
    {"to", SVG_ATT_TOK_TO},
#line 260 "src/svg_att_keywords.gperf"
    {"max", SVG_ATT_TOK_MAX},
    {""}, {""},
#line 334 "src/svg_att_keywords.gperf"
    {"result", SVG_ATT_TOK_RESULT},
#line 266 "src/svg_att_keywords.gperf"
    {"restart", SVG_ATT_TOK_RESTART},
#line 282 "src/svg_att_keywords.gperf"
    {"xml:base", SVG_ATT_TOK_XML_BASE},
    {""},
#line 350 "src/svg_att_keywords.gperf"
    {"onkeypress", SVG_ATT_TOK_ONKEYPRESS},
#line 453 "src/svg_att_keywords.gperf"
    {"method", SVG_ATT_TOK_METHOD},
#line 346 "src/svg_att_keywords.gperf"
    {"onerror", SVG_ATT_TOK_ONERROR},
    {""},
#line 377 "src/svg_att_keywords.gperf"
    {"maskUnits", SVG_ATT_TOK_MASKUNITS},
#line 424 "src/svg_att_keywords.gperf"
    {"stop-color", SVG_ATT_TOK_STOP_COLOR},
#line 267 "src/svg_att_keywords.gperf"
    {"rotate", SVG_ATT_TOK_ROTATE},
#line 428 "src/svg_att_keywords.gperf"
    {"stroke-dashoffset", SVG_ATT_TOK_STROKE_DASHOFFSET},
#line 275 "src/svg_att_keywords.gperf"
    {"requiredExtensions", SVG_ATT_TOK_REQUIREDEXTENSIONS},
#line 255 "src/svg_att_keywords.gperf"
    {"fill", SVG_ATT_TOK_FILL},
#line 415 "src/svg_att_keywords.gperf"
    {"marker-end", SVG_ATT_TOK_MARKER_END},
#line 276 "src/svg_att_keywords.gperf"
    {"requiredFeatures", SVG_ATT_TOK_REQUIREDFEATURES},
#line 443 "src/svg_att_keywords.gperf"
    {"fx", SVG_ATT_TOK_FX},
#line 359 "src/svg_att_keywords.gperf"
    {"onresize", SVG_ATT_TOK_ONRESIZE},
    {""},
#line 416 "src/svg_att_keywords.gperf"
    {"marker-mid", SVG_ATT_TOK_MARKER_MID},
    {""},
#line 442 "src/svg_att_keywords.gperf"
    {"fr", SVG_ATT_TOK_FR},
    {""},
#line 374 "src/svg_att_keywords.gperf"
    {"refY", SVG_ATT_TOK_REFY},
    {""},
#line 401 "src/svg_att_keywords.gperf"
    {"filter", SVG_ATT_TOK_FILTER},
    {""},
#line 358 "src/svg_att_keywords.gperf"
    {"onrepeat", SVG_ATT_TOK_ONREPEAT},
#line 400 "src/svg_att_keywords.gperf"
    {"fill-rule", SVG_ATT_TOK_FILL_RULE},
    {""},
#line 354 "src/svg_att_keywords.gperf"
    {"onmousemove", SVG_ATT_TOK_ONMOUSEMOVE},
    {""}, {""},
#line 373 "src/svg_att_keywords.gperf"
    {"refX", SVG_ATT_TOK_REFX},
    {""},
#line 356 "src/svg_att_keywords.gperf"
    {"onmouseover", SVG_ATT_TOK_ONMOUSEOVER},
#line 447 "src/svg_att_keywords.gperf"
    {"version", SVG_ATT_TOK_VERSION},
    {""},
#line 256 "src/svg_att_keywords.gperf"
    {"from", SVG_ATT_TOK_FROM},
    {""},
#line 445 "src/svg_att_keywords.gperf"
    {"offset", SVG_ATT_TOK_OFFSET},
#line 431 "src/svg_att_keywords.gperf"
    {"stroke-miterlimit", SVG_ATT_TOK_STROKE_MITERLIMIT},
    {""}, {""},
#line 355 "src/svg_att_keywords.gperf"
    {"onmouseout", SVG_ATT_TOK_ONMOUSEOUT},
    {""},
#line 351 "src/svg_att_keywords.gperf"
    {"onkeyup", SVG_ATT_TOK_ONKEYUP},
    {""},
#line 395 "src/svg_att_keywords.gperf"
    {"direction", SVG_ATT_TOK_DIRECTION},
    {""}, {""}, {""}, {""},
#line 269 "src/svg_att_keywords.gperf"
    {"type", SVG_ATT_TOK_TYPE},
    {""}, {""},
#line 271 "src/svg_att_keywords.gperf"
    {"cx", SVG_ATT_TOK_CX},
#line 247 "src/svg_att_keywords.gperf"
    {"additive", SVG_ATT_TOK_ADDITIVE},
    {""}, {""},
#line 402 "src/svg_att_keywords.gperf"
    {"flood-color", SVG_ATT_TOK_FLOOD_COLOR},
#line 341 "src/svg_att_keywords.gperf"
    {"onabort", SVG_ATT_TOK_ONABORT},
    {""}, {""},
#line 389 "src/svg_att_keywords.gperf"
    {"color", SVG_ATT_TOK_COLOR},
#line 376 "src/svg_att_keywords.gperf"
    {"maskContentUnits", SVG_ATT_TOK_MASKCONTENTUNITS},
#line 279 "src/svg_att_keywords.gperf"
    {"id", SVG_ATT_TOK_ID},
#line 361 "src/svg_att_keywords.gperf"
    {"onunload", SVG_ATT_TOK_ONUNLOAD},
    {""},
#line 348 "src/svg_att_keywords.gperf"
    {"onfocusout", SVG_ATT_TOK_ONFOCUSOUT},
#line 370 "src/svg_att_keywords.gperf"
    {"markerUnits", SVG_ATT_TOK_MARKERUNITS},
#line 344 "src/svg_att_keywords.gperf"
    {"onclick", SVG_ATT_TOK_ONCLICK},
    {""},
#line 284 "src/svg_att_keywords.gperf"
    {"xml:space", SVG_ATT_TOK_XML_SPACE},
    {""},
#line 394 "src/svg_att_keywords.gperf"
    {"cursor", SVG_ATT_TOK_CURSOR},
#line 287 "src/svg_att_keywords.gperf"
    {"feBlend_mode", SVG_ATT_TOK_FEBLEND_MODE},
#line 252 "src/svg_att_keywords.gperf"
    {"calcMode", SVG_ATT_TOK_CALCMODE},
    {""}, {""}, {""},
#line 417 "src/svg_att_keywords.gperf"
    {"marker-start", SVG_ATT_TOK_MARKER_START},
#line 248 "src/svg_att_keywords.gperf"
    {"attributeName", SVG_ATT_TOK_ATTRIBUTENAME},
#line 340 "src/svg_att_keywords.gperf"
    {"preserveAspectRatio", SVG_ATT_TOK_PRESERVEASPECTRATIO},
    {""},
#line 329 "src/svg_att_keywords.gperf"
    {"filterUnits", SVG_ATT_TOK_FILTERUNITS},
#line 368 "src/svg_att_keywords.gperf"
    {"spreadMethod", SVG_ATT_TOK_SPREADMETHOD},
#line 261 "src/svg_att_keywords.gperf"
    {"min", SVG_ATT_TOK_MIN},
    {""}, {""}, {""},
#line 450 "src/svg_att_keywords.gperf"
    {"dy", SVG_ATT_TOK_DY},
    {""}, {""},
#line 430 "src/svg_att_keywords.gperf"
    {"stroke-linejoin", SVG_ATT_TOK_STROKE_LINEJOIN},
#line 372 "src/svg_att_keywords.gperf"
    {"orient", SVG_ATT_TOK_ORIENT},
#line 396 "src/svg_att_keywords.gperf"
    {"display", SVG_ATT_TOK_DISPLAY},
    {""},
#line 429 "src/svg_att_keywords.gperf"
    {"stroke-linecap", SVG_ATT_TOK_STROKE_LINECAP},
    {""},
#line 434 "src/svg_att_keywords.gperf"
    {"text-anchor", SVG_ATT_TOK_TEXT_ANCHOR},
#line 286 "src/svg_att_keywords.gperf"
    {"ry", SVG_ATT_TOK_RY},
#line 259 "src/svg_att_keywords.gperf"
    {"keyTimes", SVG_ATT_TOK_KEYTIMES},
    {""},
#line 280 "src/svg_att_keywords.gperf"
    {"style", SVG_ATT_TOK_STYLE},
#line 456 "src/svg_att_keywords.gperf"
    {"startOffset", SVG_ATT_TOK_STARTOFFSET},
    {""}, {""},
#line 339 "src/svg_att_keywords.gperf"
    {"href", SVG_ATT_TOK_HREF},
#line 460 "src/svg_att_keywords.gperf"
    {"xlink:role", SVG_ATT_TOK_XLINK_ROLE},
    {""}, {""},
#line 458 "src/svg_att_keywords.gperf"
    {"xlink:arcrole", SVG_ATT_TOK_XLINK_ARCROLE},
#line 405 "src/svg_att_keywords.gperf"
    {"font-size", SVG_ATT_TOK_FONT_SIZE},
#line 278 "src/svg_att_keywords.gperf"
    {"class", SVG_ATT_TOK_CLASS},
#line 427 "src/svg_att_keywords.gperf"
    {"stroke-dasharray", SVG_ATT_TOK_STROKE_DASHARRAY},
#line 332 "src/svg_att_keywords.gperf"
    {"in", SVG_ATT_TOK_IN},
#line 308 "src/svg_att_keywords.gperf"
    {"feDisplacementMap_scale", SVG_ATT_TOK_FEDISPLACEMENTMAP_SCALE},
#line 257 "src/svg_att_keywords.gperf"
    {"keyPoints", SVG_ATT_TOK_KEYPOINTS},
#line 311 "src/svg_att_keywords.gperf"
    {"feDropShadow_dx", SVG_ATT_TOK_FEDROPSHADOW_DX},
#line 264 "src/svg_att_keywords.gperf"
    {"repeatCount", SVG_ATT_TOK_REPEATCOUNT},
    {""},
#line 392 "src/svg_att_keywords.gperf"
    {"color-profile", SVG_ATT_TOK_COLOR_PROFILE},
#line 294 "src/svg_att_keywords.gperf"
    {"feComposite_k4", SVG_ATT_TOK_FECOMPOSITE_K4},
#line 448 "src/svg_att_keywords.gperf"
    {"zoomAndPan", SVG_ATT_TOK_ZOOMANDPAN},
#line 383 "src/svg_att_keywords.gperf"
    {"points", SVG_ATT_TOK_POINTS},
#line 451 "src/svg_att_keywords.gperf"
    {"lengthAdjust", SVG_ATT_TOK_LENGTHADJUST},
#line 438 "src/svg_att_keywords.gperf"
    {"vector-effect", SVG_ATT_TOK_VECTOR_EFFECT},
#line 263 "src/svg_att_keywords.gperf"
    {"path", SVG_ATT_TOK_PATH},
#line 342 "src/svg_att_keywords.gperf"
    {"onactivate", SVG_ATT_TOK_ONACTIVATE},
#line 262 "src/svg_att_keywords.gperf"
    {"origin", SVG_ATT_TOK_ORIGIN},
#line 444 "src/svg_att_keywords.gperf"
    {"fy", SVG_ATT_TOK_FY},
    {""},
#line 265 "src/svg_att_keywords.gperf"
    {"repeatDur", SVG_ATT_TOK_REPEATDUR},
#line 321 "src/svg_att_keywords.gperf"
    {"feSpecularLighting_specularExponent", SVG_ATT_TOK_FESPECULARLIGHTING_SPECULAREXPONENT},
    {""},
#line 369 "src/svg_att_keywords.gperf"
    {"markerHeight", SVG_ATT_TOK_MARKERHEIGHT},
    {""},
#line 357 "src/svg_att_keywords.gperf"
    {"onmouseup", SVG_ATT_TOK_ONMOUSEUP},
#line 298 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_edgeMode", SVG_ATT_TOK_FECONVOLVEMATRIX_EDGEMODE},
#line 406 "src/svg_att_keywords.gperf"
    {"font-size-adjust", SVG_ATT_TOK_FONT_SIZE_ADJUST},
#line 455 "src/svg_att_keywords.gperf"
    {"spacing", SVG_ATT_TOK_SPACING},
    {""},
#line 309 "src/svg_att_keywords.gperf"
    {"feDisplacementMap_xChannelSelector", SVG_ATT_TOK_FEDISPLACEMENTMAP_XCHANNELSELECTOR},
#line 435 "src/svg_att_keywords.gperf"
    {"text-decoration", SVG_ATT_TOK_TEXT_DECORATION},
    {""},
#line 301 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_order", SVG_ATT_TOK_FECONVOLVEMATRIX_ORDER},
    {""},
#line 422 "src/svg_att_keywords.gperf"
    {"pointer-events", SVG_ATT_TOK_POINTER_EVENTS},
#line 295 "src/svg_att_keywords.gperf"
    {"feComposite_operator", SVG_ATT_TOK_FECOMPOSITE_OPERATOR},
#line 323 "src/svg_att_keywords.gperf"
    {"feTile", SVG_ATT_TOK_FETILE},
#line 316 "src/svg_att_keywords.gperf"
    {"feImage_preserveAspectRatio", SVG_ATT_TOK_FEIMAGE_PRESERVEASPECTRATIO},
    {""},
#line 318 "src/svg_att_keywords.gperf"
    {"feMorphology_radius", SVG_ATT_TOK_FEMORPHOLOGY_RADIUS},
#line 258 "src/svg_att_keywords.gperf"
    {"keySplines", SVG_ATT_TOK_KEYSPLINES},
    {""},
#line 409 "src/svg_att_keywords.gperf"
    {"font-variant", SVG_ATT_TOK_FONT_VARIANT},
    {""},
#line 277 "src/svg_att_keywords.gperf"
    {"systemLanguage", SVG_ATT_TOK_SYSTEMLANGUAGE},
#line 423 "src/svg_att_keywords.gperf"
    {"shape-rendering", SVG_ATT_TOK_SHAPE_RENDERING},
#line 421 "src/svg_att_keywords.gperf"
    {"paint-order", SVG_ATT_TOK_PAINT_ORDER},
#line 363 "src/svg_att_keywords.gperf"
    {"x2", SVG_ATT_TOK_X2},
    {""},
#line 388 "src/svg_att_keywords.gperf"
    {"clip-rule", SVG_ATT_TOK_CLIP_RULE},
    {""}, {""},
#line 419 "src/svg_att_keywords.gperf"
    {"opacity", SVG_ATT_TOK_OPACITY},
    {""},
#line 436 "src/svg_att_keywords.gperf"
    {"text-rendering", SVG_ATT_TOK_TEXT_RENDERING},
#line 459 "src/svg_att_keywords.gperf"
    {"xlink:href", SVG_ATT_TOK_XLINK_HREF},
#line 331 "src/svg_att_keywords.gperf"
    {"height", SVG_ATT_TOK_HEIGHT},
#line 272 "src/svg_att_keywords.gperf"
    {"cy", SVG_ATT_TOK_CY},
    {""},
#line 347 "src/svg_att_keywords.gperf"
    {"onfocusin", SVG_ATT_TOK_ONFOCUSIN},
#line 313 "src/svg_att_keywords.gperf"
    {"feDropShadow_stdDeviation", SVG_ATT_TOK_FEDROPSHADOW_STDDEVIATION},
#line 322 "src/svg_att_keywords.gperf"
    {"feSpecularLighting_surfaceScale", SVG_ATT_TOK_FESPECULARLIGHTING_SURFACESCALE},
    {""}, {""},
#line 293 "src/svg_att_keywords.gperf"
    {"feComposite_k3", SVG_ATT_TOK_FECOMPOSITE_K3},
#line 393 "src/svg_att_keywords.gperf"
    {"color-rendering", SVG_ATT_TOK_COLOR_RENDERING},
#line 317 "src/svg_att_keywords.gperf"
    {"feMorphology_operator", SVG_ATT_TOK_FEMORPHOLOGY_OPERATOR},
#line 407 "src/svg_att_keywords.gperf"
    {"font-stretch", SVG_ATT_TOK_FONT_STRETCH},
#line 305 "src/svg_att_keywords.gperf"
    {"feDiffuseLighting_diffuseConstant", SVG_ATT_TOK_FEDIFFUSELIGHTING_DIFFUSECONSTANT},
#line 432 "src/svg_att_keywords.gperf"
    {"stroke-opacity", SVG_ATT_TOK_STROKE_OPACITY},
#line 307 "src/svg_att_keywords.gperf"
    {"feDiffuseLighting_surfaceScale", SVG_ATT_TOK_FEDIFFUSELIGHTING_SURFACESCALE},
#line 462 "src/svg_att_keywords.gperf"
    {"xlink:title", SVG_ATT_TOK_XLINK_TITLE},
#line 391 "src/svg_att_keywords.gperf"
    {"color-interpolation-filters", SVG_ATT_TOK_COLOR_INTERPOLATION_FILTERS},
    {""},
#line 304 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_targetY", SVG_ATT_TOK_FECONVOLVEMATRIX_TARGETY},
#line 320 "src/svg_att_keywords.gperf"
    {"feSpecularLighting_specularConstant", SVG_ATT_TOK_FESPECULARLIGHTING_SPECULARCONSTANT},
    {""},
#line 366 "src/svg_att_keywords.gperf"
    {"gradientTransform", SVG_ATT_TOK_GRADIENTTRANSFORM},
#line 283 "src/svg_att_keywords.gperf"
    {"xml:lang", SVG_ATT_TOK_XML_LANG},
#line 380 "src/svg_att_keywords.gperf"
    {"patternContentUnits", SVG_ATT_TOK_PATTERNCONTENTUNITS},
    {""}, {""},
#line 362 "src/svg_att_keywords.gperf"
    {"x1", SVG_ATT_TOK_X1},
    {""},
#line 303 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_targetX", SVG_ATT_TOK_FECONVOLVEMATRIX_TARGETX},
#line 246 "src/svg_att_keywords.gperf"
    {"accumulate", SVG_ATT_TOK_ACCUMULATE},
#line 371 "src/svg_att_keywords.gperf"
    {"markerWidth", SVG_ATT_TOK_MARKERWIDTH},
#line 326 "src/svg_att_keywords.gperf"
    {"feTurbulence_seed", SVG_ATT_TOK_FETURBULENCE_SEED},
#line 281 "src/svg_att_keywords.gperf"
    {"tabindex", SVG_ATT_TOK_TABINDEX},
#line 386 "src/svg_att_keywords.gperf"
    {"clip", SVG_ATT_TOK_CLIP},
    {""},
#line 404 "src/svg_att_keywords.gperf"
    {"font-family", SVG_ATT_TOK_FONT_FAMILY},
    {""},
#line 249 "src/svg_att_keywords.gperf"
    {"attributeType", SVG_ATT_TOK_ATTRIBUTETYPE},
#line 385 "src/svg_att_keywords.gperf"
    {"baseline-shift", SVG_ATT_TOK_BASELINE_SHIFT},
#line 408 "src/svg_att_keywords.gperf"
    {"font-style", SVG_ATT_TOK_FONT_STYLE},
#line 337 "src/svg_att_keywords.gperf"
    {"y", SVG_ATT_TOK_Y},
#line 328 "src/svg_att_keywords.gperf"
    {"feTurbulence_type", SVG_ATT_TOK_FETURBULENCE_TYPE},
#line 367 "src/svg_att_keywords.gperf"
    {"gradientUnits", SVG_ATT_TOK_GRADIENTUNITS},
#line 349 "src/svg_att_keywords.gperf"
    {"onkeydown", SVG_ATT_TOK_ONKEYDOWN},
    {""}, {""}, {""},
#line 384 "src/svg_att_keywords.gperf"
    {"alignment-baseline", SVG_ATT_TOK_ALIGNMENT_BASELINE},
#line 330 "src/svg_att_keywords.gperf"
    {"primitiveUnits", SVG_ATT_TOK_PRIMITIVEUNITS},
#line 463 "src/svg_att_keywords.gperf"
    {"xlink:type", SVG_ATT_TOK_XLINK_TYPE},
#line 296 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_bias", SVG_ATT_TOK_FECONVOLVEMATRIX_BIAS},
    {""},
#line 325 "src/svg_att_keywords.gperf"
    {"feTurbulence_numOctaves", SVG_ATT_TOK_FETURBULENCE_NUMOCTAVES},
#line 306 "src/svg_att_keywords.gperf"
    {"feDiffuseLighting_kernelUnitLength", SVG_ATT_TOK_FEDIFFUSELIGHTING_KERNELUNITLENGTH},
    {""},
#line 353 "src/svg_att_keywords.gperf"
    {"onmousedown", SVG_ATT_TOK_ONMOUSEDOWN},
#line 397 "src/svg_att_keywords.gperf"
    {"dominant-baseline", SVG_ATT_TOK_DOMINANT_BASELINE},
    {""},
#line 390 "src/svg_att_keywords.gperf"
    {"color-interpolation", SVG_ATT_TOK_COLOR_INTERPOLATION},
#line 335 "src/svg_att_keywords.gperf"
    {"width", SVG_ATT_TOK_WIDTH},
#line 446 "src/svg_att_keywords.gperf"
    {"baseProfile", SVG_ATT_TOK_BASEPROFILE},
#line 382 "src/svg_att_keywords.gperf"
    {"patternUnits", SVG_ATT_TOK_PATTERNUNITS},
#line 314 "src/svg_att_keywords.gperf"
    {"feGaussianBlur_edgeMode", SVG_ATT_TOK_FEGAUSSIANBLUR_EDGEMODE},
#line 297 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_divisor", SVG_ATT_TOK_FECONVOLVEMATRIX_DIVISOR},
#line 312 "src/svg_att_keywords.gperf"
    {"feDropShadow_dy", SVG_ATT_TOK_FEDROPSHADOW_DY},
    {""},
#line 343 "src/svg_att_keywords.gperf"
    {"onbegin", SVG_ATT_TOK_ONBEGIN},
    {""},
#line 290 "src/svg_att_keywords.gperf"
    {"feComponentTransfer_type", SVG_ATT_TOK_FECOMPONENTTRANSFER_TYPE},
    {""},
#line 412 "src/svg_att_keywords.gperf"
    {"glyph-orientation-vertical", SVG_ATT_TOK_GLYPH_ORIENTATION_VERTICAL},
    {""},
#line 288 "src/svg_att_keywords.gperf"
    {"feColorMatrix_type", SVG_ATT_TOK_FECOLORMATRIX_TYPE},
#line 299 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_kernelMatrix", SVG_ATT_TOK_FECONVOLVEMATRIX_KERNELMATRIX},
#line 289 "src/svg_att_keywords.gperf"
    {"feColorMatrix_values", SVG_ATT_TOK_FECOLORMATRIX_VALUES},
    {""},
#line 433 "src/svg_att_keywords.gperf"
    {"stroke-width", SVG_ATT_TOK_STROKE_WIDTH},
    {""}, {""}, {""}, {""}, {""},
#line 457 "src/svg_att_keywords.gperf"
    {"xlink:actuate", SVG_ATT_TOK_XLINK_ACTUATE},
#line 387 "src/svg_att_keywords.gperf"
    {"clip-path", SVG_ATT_TOK_CLIP_PATH},
#line 413 "src/svg_att_keywords.gperf"
    {"image-rendering", SVG_ATT_TOK_IMAGE_RENDERING},
#line 381 "src/svg_att_keywords.gperf"
    {"patternTransform", SVG_ATT_TOK_PATTERNTRANSFORM},
    {""}, {""},
#line 310 "src/svg_att_keywords.gperf"
    {"feDisplacementMap_yChannelSelector", SVG_ATT_TOK_FEDISPLACEMENTMAP_YCHANNELSELECTOR},
#line 302 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_preserveAlpha", SVG_ATT_TOK_FECONVOLVEMATRIX_PRESERVEALPHA},
    {""}, {""}, {""}, {""}, {""},
#line 410 "src/svg_att_keywords.gperf"
    {"font-weight", SVG_ATT_TOK_FONT_WEIGHT},
#line 425 "src/svg_att_keywords.gperf"
    {"stop-opacity", SVG_ATT_TOK_STOP_OPACITY},
    {""}, {""},
#line 319 "src/svg_att_keywords.gperf"
    {"feSpecularLighting_kernelUnitLength", SVG_ATT_TOK_FESPECULARLIGHTING_KERNELUNITLENGTH},
    {""},
#line 315 "src/svg_att_keywords.gperf"
    {"feGaussianBlur_stdDeviation", SVG_ATT_TOK_FEGAUSSIANBLUR_STDDEVIATION},
    {""}, {""},
#line 452 "src/svg_att_keywords.gperf"
    {"textLength", SVG_ATT_TOK_TEXTLENGTH},
    {""},
#line 365 "src/svg_att_keywords.gperf"
    {"y2", SVG_ATT_TOK_Y2},
    {""}, {""}, {""}, {""},
#line 441 "src/svg_att_keywords.gperf"
    {"writing-mode", SVG_ATT_TOK_WRITING_MODE},
    {""},
#line 414 "src/svg_att_keywords.gperf"
    {"lighting-color", SVG_ATT_TOK_LIGHTING_COLOR},
    {""}, {""}, {""},
#line 403 "src/svg_att_keywords.gperf"
    {"flood-opacity", SVG_ATT_TOK_FLOOD_OPACITY},
    {""}, {""}, {""},
#line 251 "src/svg_att_keywords.gperf"
    {"by", SVG_ATT_TOK_BY},
#line 274 "src/svg_att_keywords.gperf"
    {"clipPathUnits", SVG_ATT_TOK_CLIPPATHUNITS},
#line 327 "src/svg_att_keywords.gperf"
    {"feTurbulence_stitchTiles", SVG_ATT_TOK_FETURBULENCE_STITCHTILES},
    {""}, {""},
#line 399 "src/svg_att_keywords.gperf"
    {"fill-opacity", SVG_ATT_TOK_FILL_OPACITY},
    {""}, {""}, {""}, {""}, {""},
#line 300 "src/svg_att_keywords.gperf"
    {"feConvolveMatrix_kernelUnitLength", SVG_ATT_TOK_FECONVOLVEMATRIX_KERNELUNITLENGTH},
    {""}, {""}, {""}, {""}, {""},
#line 292 "src/svg_att_keywords.gperf"
    {"feComposite_k2", SVG_ATT_TOK_FECOMPOSITE_K2},
    {""},
#line 338 "src/svg_att_keywords.gperf"
    {"crossorigin", SVG_ATT_TOK_CROSSORIGIN},
#line 364 "src/svg_att_keywords.gperf"
    {"y1", SVG_ATT_TOK_Y1},
    {""}, {""},
#line 439 "src/svg_att_keywords.gperf"
    {"visibility", SVG_ATT_TOK_VISIBILITY},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 379 "src/svg_att_keywords.gperf"
    {"pathLength", SVG_ATT_TOK_PATHLENGTH},
    {""}, {""},
#line 411 "src/svg_att_keywords.gperf"
    {"glyph-orientation-horizontal", SVG_ATT_TOK_GLYPH_ORIENTATION_HORIZONTAL},
    {""},
#line 250 "src/svg_att_keywords.gperf"
    {"begin", SVG_ATT_TOK_BEGIN},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 398 "src/svg_att_keywords.gperf"
    {"enable-background", SVG_ATT_TOK_ENABLE_BACKGROUND},
#line 420 "src/svg_att_keywords.gperf"
    {"overflow", SVG_ATT_TOK_OVERFLOW},
    {""}, {""},
#line 324 "src/svg_att_keywords.gperf"
    {"feTurbulence_baseFrequency", SVG_ATT_TOK_FETURBULENCE_BASEFREQUENCY},
    {""}, {""},
#line 291 "src/svg_att_keywords.gperf"
    {"feComposite_k1", SVG_ATT_TOK_FECOMPOSITE_K1},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 437 "src/svg_att_keywords.gperf"
    {"unicode-bidi", SVG_ATT_TOK_UNICODE_BIDI},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""},
#line 440 "src/svg_att_keywords.gperf"
    {"word-spacing", SVG_ATT_TOK_WORD_SPACING},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 333 "src/svg_att_keywords.gperf"
    {"in2", SVG_ATT_TOK_IN2},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""},
#line 461 "src/svg_att_keywords.gperf"
    {"xlink:show", SVG_ATT_TOK_XLINK_SHOW}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

const struct SvgAttKeyword *
lookup_svg_att_token_default (register const char *str, register size_t len)
{
  if (len <= ATT_MAX_WORD_LENGTH && len >= ATT_MIN_WORD_LENGTH)
    {
      register unsigned int key = hash_svg_att (str, len);

      if (key <= ATT_MAX_HASH_VALUE)
        {
          register const char *s = svg_att_wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &svg_att_wordlist[key];
        }
    }
  return (struct SvgAttKeyword *) 0;
}
#line 464 "src/svg_att_keywords.gperf"

const struct SvgAttKeyword*
lookup_svg_att_token (register const char *str, register size_t len)
{
    if (len <= ATT_MAX_WORD_LENGTH && len >= ATT_MIN_WORD_LENGTH)
    {
        register unsigned int key = hash_svg_att (str, len);

        if (key <= ATT_MAX_HASH_VALUE)
        {
            register const char *s = svg_att_wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len -1) && s[len] == '\0')
                return &svg_att_wordlist[key];
        }
    }
    return (struct SvgAttKeyword *) 0;
}
