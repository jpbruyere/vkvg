/* ANSI-C code produced by gperf version 3.2.1 */
/* Command-line: gperf --output-file=build/svg_att_gperf.h src/svg_att_keywords.gperf  */
/* Computed positions: -k'1,3,9-10,$' */

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
	SVG_ATT_TOK_XMLNS,
	SVG_ATT_TOK_XMLNS_XLINK,
	SVG_ATT_TOK_ID,
	SVG_ATT_TOK_XML_BASE,
	SVG_ATT_TOK_XML_LANG,
	SVG_ATT_TOK_REQUIREDFEATURES,
	SVG_ATT_TOK_REQUIREDEXTENSIONS,
	SVG_ATT_TOK_SYSTEMLANGUAGE,
	SVG_ATT_TOK_STYLE,
	SVG_ATT_TOK_CLASS,
	SVG_ATT_TOK_ENABLE_BACKGROUND,
	SVG_ATT_TOK_CLIP,
	SVG_ATT_TOK_BASELINE_SHIFT,
	SVG_ATT_TOK_GLYPH_ORIENTATION_HORIZONTAL,
	SVG_ATT_TOK_GLYPH_ORIENTATION_VERTICAL,
	SVG_ATT_TOK_KERNING,
	SVG_ATT_TOK_LETTER_SPACING,
	SVG_ATT_TOK_TEXT_DECORATION,
	SVG_ATT_TOK_WORD_SPACING,
	SVG_ATT_TOK_FONT_FAMILY,
	SVG_ATT_TOK_FONT_SIZE,
	SVG_ATT_TOK_FONT_SIZE_ADJUST,
	SVG_ATT_TOK_FILL,
	SVG_ATT_TOK_STROKE,
	SVG_ATT_TOK_STROKE_DASHARRAY,
	SVG_ATT_TOK_STROKE_DASHOFFSET,
	SVG_ATT_TOK_STROKE_MITERLIMIT,
	SVG_ATT_TOK_STROKE_WIDTH,
	SVG_ATT_TOK_COLOR,
	SVG_ATT_TOK_OPACITY,
	SVG_ATT_TOK_FILL_OPACITY,
	SVG_ATT_TOK_STROKE_OPACITY,
	SVG_ATT_TOK_MARKER_START,
	SVG_ATT_TOK_MARKER_MID,
	SVG_ATT_TOK_MARKER_END,
	SVG_ATT_TOK_COLOR_PROFILE,
	SVG_ATT_TOK_STOP_COLOR,
	SVG_ATT_TOK_STOP_OPACITY,
	SVG_ATT_TOK_CLIP_PATH,
	SVG_ATT_TOK_MASK,
	SVG_ATT_TOK_FILTER,
	SVG_ATT_TOK_CURSOR,
	SVG_ATT_TOK_FLOOD_COLOR,
	SVG_ATT_TOK_FLOOD_OPACITY,
	SVG_ATT_TOK_LIGHTING_COLOR,
	SVG_ATT_TOK_ONUNLOAD,
	SVG_ATT_TOK_ONABORT,
	SVG_ATT_TOK_ONERROR,
	SVG_ATT_TOK_ONRESIZE,
	SVG_ATT_TOK_ONSCROLL,
	SVG_ATT_TOK_ONZOOM,
	SVG_ATT_TOK_ONFOCUSIN,
	SVG_ATT_TOK_ONFOCUSOUT,
	SVG_ATT_TOK_ONACTIVATE,
	SVG_ATT_TOK_ONCLICK,
	SVG_ATT_TOK_ONMOUSEDOWN,
	SVG_ATT_TOK_ONMOUSEUP,
	SVG_ATT_TOK_ONMOUSEOVER,
	SVG_ATT_TOK_ONMOUSEMOVE,
	SVG_ATT_TOK_ONMOUSEOUT,
	SVG_ATT_TOK_ONLOAD,
	SVG_ATT_TOK_X,
	SVG_ATT_TOK_Y,
	SVG_ATT_TOK_WIDTH,
	SVG_ATT_TOK_HEIGHT,
	SVG_ATT_TOK_VIEWBOX,
	SVG_ATT_TOK_PRESERVEASPECTRATIO,
	SVG_ATT_TOK_VERSION,
	SVG_ATT_TOK_BASEPROFILE,
	SVG_ATT_TOK_CONTENTSCRIPTTYPE,
	SVG_ATT_TOK_CONTENTSTYLETYPE,
	SVG_ATT_TOK_TRANSFORM,
	SVG_ATT_TOK_XLINK_HREF,
	SVG_ATT_TOK_XLINK_ROLE,
	SVG_ATT_TOK_XLINK_ARCROLE,
	SVG_ATT_TOK_XLINK_TITLE,
	SVG_ATT_TOK_TYPE,
	SVG_ATT_TOK_MEDIA,
	SVG_ATT_TOK_TITLE,
	SVG_ATT_TOK_D,
	SVG_ATT_TOK_PATHLENGTH,
	SVG_ATT_TOK_RX,
	SVG_ATT_TOK_RY,
	SVG_ATT_TOK_CX,
	SVG_ATT_TOK_CY,
	SVG_ATT_TOK_R,
	SVG_ATT_TOK_X1,
	SVG_ATT_TOK_Y1,
	SVG_ATT_TOK_X2,
	SVG_ATT_TOK_Y2,
	SVG_ATT_TOK_POINTS,
	SVG_ATT_TOK_DX,
	SVG_ATT_TOK_DY,
	SVG_ATT_TOK_ROTATE,
	SVG_ATT_TOK_TEXTLENGTH,
	SVG_ATT_TOK_STARTOFFSET,
	SVG_ATT_TOK_GLYPHREF,
	SVG_ATT_TOK_FORMAT,
	SVG_ATT_TOK_REFX,
	SVG_ATT_TOK_REFY,
	SVG_ATT_TOK_MARKERWIDTH,
	SVG_ATT_TOK_MARKERHEIGHT,
	SVG_ATT_TOK_ORIENT,
	SVG_ATT_TOK_LOCAL,
	SVG_ATT_TOK_NAME,
	SVG_ATT_TOK_GRADIENTTRANSFORM,
	SVG_ATT_TOK_FX,
	SVG_ATT_TOK_FY,
	SVG_ATT_TOK_OFFSET,
	SVG_ATT_TOK_PATTERNTRANSFORM,
	SVG_ATT_TOK_FILTERRES,
	SVG_ATT_TOK_RESULT,
	SVG_ATT_TOK_IN,
	SVG_ATT_TOK_IN2,
	SVG_ATT_TOK_VALUES,
	SVG_ATT_TOK_K1,
	SVG_ATT_TOK_K2,
	SVG_ATT_TOK_K3,
	SVG_ATT_TOK_K4,
	SVG_ATT_TOK_ORDER,
	SVG_ATT_TOK_KERNELMATRIX,
	SVG_ATT_TOK_DIVISOR,
	SVG_ATT_TOK_BIAS,
	SVG_ATT_TOK_TARGETX,
	SVG_ATT_TOK_TARGETY,
	SVG_ATT_TOK_KERNELUNITLENGTH,
	SVG_ATT_TOK_SURFACESCALE,
	SVG_ATT_TOK_DIFFUSECONSTANT,
	SVG_ATT_TOK_SCALE,
	SVG_ATT_TOK_STDDEVIATION,
	SVG_ATT_TOK_RADIUS,
	SVG_ATT_TOK_SPECULARCONSTANT,
	SVG_ATT_TOK_SPECULAREXPONENT,
	SVG_ATT_TOK_BASEFREQUENCY,
	SVG_ATT_TOK_NUMOCTAVES,
	SVG_ATT_TOK_SEED,
	SVG_ATT_TOK_AZIMUTH,
	SVG_ATT_TOK_ELEVATION,
	SVG_ATT_TOK_Z,
	SVG_ATT_TOK_POINTSATX,
	SVG_ATT_TOK_POINTSATY,
	SVG_ATT_TOK_POINTSATZ,
	SVG_ATT_TOK_LIMITINGCONEANGLE,
	SVG_ATT_TOK_TABLEVALUES,
	SVG_ATT_TOK_SLOPE,
	SVG_ATT_TOK_INTERCEPT,
	SVG_ATT_TOK_AMPLITUDE,
	SVG_ATT_TOK_EXPONENT,
	SVG_ATT_TOK_TARGET,
	SVG_ATT_TOK_VIEWTARGET,
	SVG_ATT_TOK_ONBEGIN,
	SVG_ATT_TOK_ONEND,
	SVG_ATT_TOK_ONREPEAT,
	SVG_ATT_TOK_ATTRIBUTENAME,
	SVG_ATT_TOK_ATTRIBUTETYPE,
	SVG_ATT_TOK_BEGIN,
	SVG_ATT_TOK_DUR,
	SVG_ATT_TOK_END,
	SVG_ATT_TOK_MIN,
	SVG_ATT_TOK_MAX,
	SVG_ATT_TOK_REPEATCOUNT,
	SVG_ATT_TOK_REPEATDUR,
	SVG_ATT_TOK_KEYTIMES,
	SVG_ATT_TOK_KEYSPLINES,
	SVG_ATT_TOK_FROM,
	SVG_ATT_TOK_TO,
	SVG_ATT_TOK_BY,
	SVG_ATT_TOK_PATH,
	SVG_ATT_TOK_KEYPOINTS,
	SVG_ATT_TOK_ORIGIN,
	SVG_ATT_TOK_HORIZ_ORIGIN_X,
	SVG_ATT_TOK_HORIZ_ORIGIN_Y,
	SVG_ATT_TOK_HORIZ_ADV_X,
	SVG_ATT_TOK_VERT_ORIGIN_X,
	SVG_ATT_TOK_VERT_ORIGIN_Y,
	SVG_ATT_TOK_VERT_ADV_Y,
	SVG_ATT_TOK_FONT_STYLE,
	SVG_ATT_TOK_FONT_VARIANT,
	SVG_ATT_TOK_FONT_WEIGHT,
	SVG_ATT_TOK_FONT_STRETCH,
	SVG_ATT_TOK_UNICODE_RANGE,
	SVG_ATT_TOK_UNITS_PER_EM,
	SVG_ATT_TOK_PANOSE_1,
	SVG_ATT_TOK_STEMV,
	SVG_ATT_TOK_STEMH,
	SVG_ATT_TOK_CAP_HEIGHT,
	SVG_ATT_TOK_X_HEIGHT,
	SVG_ATT_TOK_ACCENT_HEIGHT,
	SVG_ATT_TOK_ASCENT,
	SVG_ATT_TOK_DESCENT,
	SVG_ATT_TOK_WIDTHS,
	SVG_ATT_TOK_BBOX,
	SVG_ATT_TOK_IDEOGRAPHIC,
	SVG_ATT_TOK_ALPHABETIC,
	SVG_ATT_TOK_MATHEMATICAL,
	SVG_ATT_TOK_HANGING,
	SVG_ATT_TOK_V_IDEOGRAPHIC,
	SVG_ATT_TOK_V_ALPHABETIC,
	SVG_ATT_TOK_V_MATHEMATICAL,
	SVG_ATT_TOK_V_HANGING,
	SVG_ATT_TOK_UNDERLINE_POSITION,
	SVG_ATT_TOK_UNDERLINE_THICKNESS,
	SVG_ATT_TOK_STRIKETHROUGH_POSITION,
	SVG_ATT_TOK_STRIKETHROUGH_THICKNESS,
	SVG_ATT_TOK_OVERLINE_POSITION,
	SVG_ATT_TOK_OVERLINE_THICKNESS,
	SVG_ATT_TOK_UNICODE,
	SVG_ATT_TOK_GLYPH_NAME,
	SVG_ATT_TOK_ORIENTATION,
	SVG_ATT_TOK_ARABIC_FORM,
	SVG_ATT_TOK_LANG,
	SVG_ATT_TOK_U1,
	SVG_ATT_TOK_G1,
	SVG_ATT_TOK_U2,
	SVG_ATT_TOK_G2,
	SVG_ATT_TOK_K,
	SVG_ATT_TOK_STRING
} SvgAttTokId;

/* Structure returned by gperf */
struct SvgAttKeyword {
    const char *name;
    SvgAttTokId id;
};
#line 242 "src/svg_att_keywords.gperf"
struct SvgAttKeyword;

#define ATT_TOTAL_KEYWORDS 217
#define ATT_MIN_WORD_LENGTH 1
#define ATT_MAX_WORD_LENGTH 28
#define ATT_MIN_HASH_VALUE 11
#define ATT_MAX_HASH_VALUE 611
/* maximum key range = 601, duplicates = 0 */

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
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 125, 612, 612, 612,  94,
      245,  35,  30, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612,   5, 612,  30, 612,   0,
       25, 612, 612, 612, 612, 612, 612, 612,   0, 612,
      612, 612, 612, 612,  35, 612, 612, 612, 190, 175,
        0, 612, 612, 612, 612, 612, 612,  20, 143,  55,
       15,  10, 100, 130, 130,  20, 612, 205,  55, 120,
       75,   0,  95,   0,   5,   0,  25, 135, 195, 180,
      145,  55,  20, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
      612, 612, 612, 612, 612, 612
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[9]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 8:
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
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""},
#line 330 "src/svg_att_keywords.gperf"
    {"r", SVG_ATT_TOK_R},
    {""}, {""}, {""},
#line 389 "src/svg_att_keywords.gperf"
    {"slope", SVG_ATT_TOK_SLOPE},
    {""}, {""}, {""}, {""},
#line 281 "src/svg_att_keywords.gperf"
    {"stop-color", SVG_ATT_TOK_STOP_COLOR},
#line 268 "src/svg_att_keywords.gperf"
    {"stroke", SVG_ATT_TOK_STROKE},
#line 292 "src/svg_att_keywords.gperf"
    {"onerror", SVG_ATT_TOK_ONERROR},
#line 293 "src/svg_att_keywords.gperf"
    {"onresize", SVG_ATT_TOK_ONRESIZE},
    {""},
#line 364 "src/svg_att_keywords.gperf"
    {"order", SVG_ATT_TOK_ORDER},
#line 375 "src/svg_att_keywords.gperf"
    {"radius", SVG_ATT_TOK_RADIUS},
#line 410 "src/svg_att_keywords.gperf"
    {"to", SVG_ATT_TOK_TO},
#line 401 "src/svg_att_keywords.gperf"
    {"dur", SVG_ATT_TOK_DUR},
#line 380 "src/svg_att_keywords.gperf"
    {"seed", SVG_ATT_TOK_SEED},
#line 396 "src/svg_att_keywords.gperf"
    {"onend", SVG_ATT_TOK_ONEND},
#line 324 "src/svg_att_keywords.gperf"
    {"d", SVG_ATT_TOK_D},
    {""},
#line 448 "src/svg_att_keywords.gperf"
    {"strikethrough-thickness", SVG_ATT_TOK_STRIKETHROUGH_THICKNESS},
    {""},
#line 373 "src/svg_att_keywords.gperf"
    {"scale", SVG_ATT_TOK_SCALE},
#line 356 "src/svg_att_keywords.gperf"
    {"result", SVG_ATT_TOK_RESULT},
#line 247 "src/svg_att_keywords.gperf"
    {"id", SVG_ATT_TOK_ID},
#line 397 "src/svg_att_keywords.gperf"
    {"onrepeat", SVG_ATT_TOK_ONREPEAT},
    {""}, {""},
#line 383 "src/svg_att_keywords.gperf"
    {"z", SVG_ATT_TOK_Z},
    {""},
#line 402 "src/svg_att_keywords.gperf"
    {"end", SVG_ATT_TOK_END},
    {""}, {""},
#line 338 "src/svg_att_keywords.gperf"
    {"rotate", SVG_ATT_TOK_ROTATE},
#line 434 "src/svg_att_keywords.gperf"
    {"descent", SVG_ATT_TOK_DESCENT},
    {""}, {""}, {""},
#line 347 "src/svg_att_keywords.gperf"
    {"orient", SVG_ATT_TOK_ORIENT},
#line 291 "src/svg_att_keywords.gperf"
    {"onabort", SVG_ATT_TOK_ONABORT},
    {""}, {""}, {""},
#line 250 "src/svg_att_keywords.gperf"
    {"requiredFeatures", SVG_ATT_TOK_REQUIREDFEATURES},
    {""}, {""}, {""}, {""},
#line 393 "src/svg_att_keywords.gperf"
    {"target", SVG_ATT_TOK_TARGET},
#line 327 "src/svg_att_keywords.gperf"
    {"ry", SVG_ATT_TOK_RY},
#line 294 "src/svg_att_keywords.gperf"
    {"onscroll", SVG_ATT_TOK_ONSCROLL},
    {""},
#line 323 "src/svg_att_keywords.gperf"
    {"title", SVG_ATT_TOK_TITLE},
#line 340 "src/svg_att_keywords.gperf"
    {"startOffset", SVG_ATT_TOK_STARTOFFSET},
#line 270 "src/svg_att_keywords.gperf"
    {"stroke-dashoffset", SVG_ATT_TOK_STROKE_DASHOFFSET},
    {""}, {""},
#line 253 "src/svg_att_keywords.gperf"
    {"style", SVG_ATT_TOK_STYLE},
#line 286 "src/svg_att_keywords.gperf"
    {"cursor", SVG_ATT_TOK_CURSOR},
#line 337 "src/svg_att_keywords.gperf"
    {"dy", SVG_ATT_TOK_DY},
    {""}, {""},
#line 298 "src/svg_att_keywords.gperf"
    {"onactivate", SVG_ATT_TOK_ONACTIVATE},
#line 305 "src/svg_att_keywords.gperf"
    {"onload", SVG_ATT_TOK_ONLOAD},
    {""},
#line 398 "src/svg_att_keywords.gperf"
    {"attributeName", SVG_ATT_TOK_ATTRIBUTENAME},
    {""},
#line 254 "src/svg_att_keywords.gperf"
    {"class", SVG_ATT_TOK_CLASS},
#line 376 "src/svg_att_keywords.gperf"
    {"specularConstant", SVG_ATT_TOK_SPECULARCONSTANT},
#line 274 "src/svg_att_keywords.gperf"
    {"opacity", SVG_ATT_TOK_OPACITY},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 271 "src/svg_att_keywords.gperf"
    {"stroke-miterlimit", SVG_ATT_TOK_STROKE_MITERLIMIT},
    {""}, {""}, {""},
#line 269 "src/svg_att_keywords.gperf"
    {"stroke-dasharray", SVG_ATT_TOK_STROKE_DASHARRAY},
#line 357 "src/svg_att_keywords.gperf"
    {"in", SVG_ATT_TOK_IN},
    {""}, {""}, {""},
#line 414 "src/svg_att_keywords.gperf"
    {"origin", SVG_ATT_TOK_ORIGIN},
#line 371 "src/svg_att_keywords.gperf"
    {"surfaceScale", SVG_ATT_TOK_SURFACESCALE},
    {""},
#line 390 "src/svg_att_keywords.gperf"
    {"intercept", SVG_ATT_TOK_INTERCEPT},
    {""},
#line 433 "src/svg_att_keywords.gperf"
    {"ascent", SVG_ATT_TOK_ASCENT},
#line 447 "src/svg_att_keywords.gperf"
    {"strikethrough-position", SVG_ATT_TOK_STRIKETHROUGH_POSITION},
    {""}, {""}, {""},
#line 307 "src/svg_att_keywords.gperf"
    {"y", SVG_ATT_TOK_Y},
#line 329 "src/svg_att_keywords.gperf"
    {"cy", SVG_ATT_TOK_CY},
#line 399 "src/svg_att_keywords.gperf"
    {"attributeType", SVG_ATT_TOK_ATTRIBUTETYPE},
    {""}, {""}, {""}, {""}, {""},
#line 406 "src/svg_att_keywords.gperf"
    {"repeatDur", SVG_ATT_TOK_REPEATDUR},
#line 273 "src/svg_att_keywords.gperf"
    {"color", SVG_ATT_TOK_COLOR},
#line 335 "src/svg_att_keywords.gperf"
    {"points", SVG_ATT_TOK_POINTS},
    {""}, {""},
#line 386 "src/svg_att_keywords.gperf"
    {"pointsAtZ", SVG_ATT_TOK_POINTSATZ},
    {""},
#line 453 "src/svg_att_keywords.gperf"
    {"orientation", SVG_ATT_TOK_ORIENTATION},
    {""}, {""},
#line 311 "src/svg_att_keywords.gperf"
    {"preserveAspectRatio", SVG_ATT_TOK_PRESERVEASPECTRATIO},
    {""},
#line 353 "src/svg_att_keywords.gperf"
    {"offset", SVG_ATT_TOK_OFFSET},
    {""}, {""},
#line 321 "src/svg_att_keywords.gperf"
    {"type", SVG_ATT_TOK_TYPE},
    {""},
#line 342 "src/svg_att_keywords.gperf"
    {"format", SVG_ATT_TOK_FORMAT},
#line 255 "src/svg_att_keywords.gperf"
    {"enable-background", SVG_ATT_TOK_ENABLE_BACKGROUND},
#line 392 "src/svg_att_keywords.gperf"
    {"exponent", SVG_ATT_TOK_EXPONENT},
    {""}, {""},
#line 461 "src/svg_att_keywords.gperf"
    {"string", SVG_ATT_TOK_STRING},
#line 282 "src/svg_att_keywords.gperf"
    {"stop-opacity", SVG_ATT_TOK_STOP_OPACITY},
#line 432 "src/svg_att_keywords.gperf"
    {"accent-height", SVG_ATT_TOK_ACCENT_HEIGHT},
#line 391 "src/svg_att_keywords.gperf"
    {"amplitude", SVG_ATT_TOK_AMPLITUDE},
#line 429 "src/svg_att_keywords.gperf"
    {"stemh", SVG_ATT_TOK_STEMH},
#line 295 "src/svg_att_keywords.gperf"
    {"onzoom", SVG_ATT_TOK_ONZOOM},
#line 374 "src/svg_att_keywords.gperf"
    {"stdDeviation", SVG_ATT_TOK_STDDEVIATION},
    {""}, {""}, {""},
#line 332 "src/svg_att_keywords.gperf"
    {"y1", SVG_ATT_TOK_Y1},
#line 326 "src/svg_att_keywords.gperf"
    {"rx", SVG_ATT_TOK_RX},
    {""}, {""}, {""}, {""},
#line 352 "src/svg_att_keywords.gperf"
    {"fy", SVG_ATT_TOK_FY},
#line 290 "src/svg_att_keywords.gperf"
    {"onunload", SVG_ATT_TOK_ONUNLOAD},
    {""},
#line 322 "src/svg_att_keywords.gperf"
    {"media", SVG_ATT_TOK_MEDIA},
    {""},
#line 336 "src/svg_att_keywords.gperf"
    {"dx", SVG_ATT_TOK_DX},
    {""},
#line 355 "src/svg_att_keywords.gperf"
    {"filterRes", SVG_ATT_TOK_FILTERRES},
    {""},
#line 285 "src/svg_att_keywords.gperf"
    {"filter", SVG_ATT_TOK_FILTER},
#line 367 "src/svg_att_keywords.gperf"
    {"bias", SVG_ATT_TOK_BIAS},
#line 251 "src/svg_att_keywords.gperf"
    {"requiredExtensions", SVG_ATT_TOK_REQUIREDEXTENSIONS},
    {""},
#line 348 "src/svg_att_keywords.gperf"
    {"local", SVG_ATT_TOK_LOCAL},
#line 287 "src/svg_att_keywords.gperf"
    {"flood-color", SVG_ATT_TOK_FLOOD_COLOR},
#line 451 "src/svg_att_keywords.gperf"
    {"unicode", SVG_ATT_TOK_UNICODE},
    {""},
#line 256 "src/svg_att_keywords.gperf"
    {"clip", SVG_ATT_TOK_CLIP},
    {""},
#line 454 "src/svg_att_keywords.gperf"
    {"arabic-form", SVG_ATT_TOK_ARABIC_FORM},
#line 381 "src/svg_att_keywords.gperf"
    {"azimuth", SVG_ATT_TOK_AZIMUTH},
#line 450 "src/svg_att_keywords.gperf"
    {"overline-thickness", SVG_ATT_TOK_OVERLINE_THICKNESS},
#line 382 "src/svg_att_keywords.gperf"
    {"elevation", SVG_ATT_TOK_ELEVATION},
    {""},
#line 309 "src/svg_att_keywords.gperf"
    {"height", SVG_ATT_TOK_HEIGHT},
#line 272 "src/svg_att_keywords.gperf"
    {"stroke-width", SVG_ATT_TOK_STROKE_WIDTH},
    {""}, {""},
#line 278 "src/svg_att_keywords.gperf"
    {"marker-mid", SVG_ATT_TOK_MARKER_MID},
    {""}, {""}, {""},
#line 276 "src/svg_att_keywords.gperf"
    {"stroke-opacity", SVG_ATT_TOK_STROKE_OPACITY},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 377 "src/svg_att_keywords.gperf"
    {"specularExponent", SVG_ATT_TOK_SPECULAREXPONENT},
    {""}, {""}, {""},
#line 411 "src/svg_att_keywords.gperf"
    {"by", SVG_ATT_TOK_BY},
#line 435 "src/svg_att_keywords.gperf"
    {"widths", SVG_ATT_TOK_WIDTHS},
#line 328 "src/svg_att_keywords.gperf"
    {"cx", SVG_ATT_TOK_CX},
#line 425 "src/svg_att_keywords.gperf"
    {"unicode-range", SVG_ATT_TOK_UNICODE_RANGE},
#line 265 "src/svg_att_keywords.gperf"
    {"font-size", SVG_ATT_TOK_FONT_SIZE},
#line 245 "src/svg_att_keywords.gperf"
    {"xmlns", SVG_ATT_TOK_XMLNS},
    {""},
#line 277 "src/svg_att_keywords.gperf"
    {"marker-start", SVG_ATT_TOK_MARKER_START},
    {""},
#line 349 "src/svg_att_keywords.gperf"
    {"name", SVG_ATT_TOK_NAME},
#line 428 "src/svg_att_keywords.gperf"
    {"stemv", SVG_ATT_TOK_STEMV},
    {""},
#line 369 "src/svg_att_keywords.gperf"
    {"targetY", SVG_ATT_TOK_TARGETY},
    {""},
#line 267 "src/svg_att_keywords.gperf"
    {"fill", SVG_ATT_TOK_FILL},
#line 379 "src/svg_att_keywords.gperf"
    {"numOctaves", SVG_ATT_TOK_NUMOCTAVES},
    {""},
#line 314 "src/svg_att_keywords.gperf"
    {"contentScriptType", SVG_ATT_TOK_CONTENTSCRIPTTYPE},
#line 248 "src/svg_att_keywords.gperf"
    {"xml:base", SVG_ATT_TOK_XML_BASE},
    {""}, {""}, {""},
#line 366 "src/svg_att_keywords.gperf"
    {"divisor", SVG_ATT_TOK_DIVISOR},
    {""},
#line 409 "src/svg_att_keywords.gperf"
    {"from", SVG_ATT_TOK_FROM},
#line 395 "src/svg_att_keywords.gperf"
    {"onbegin", SVG_ATT_TOK_ONBEGIN},
#line 457 "src/svg_att_keywords.gperf"
    {"g1", SVG_ATT_TOK_G1},
#line 368 "src/svg_att_keywords.gperf"
    {"targetX", SVG_ATT_TOK_TARGETX},
    {""},
#line 252 "src/svg_att_keywords.gperf"
    {"systemLanguage", SVG_ATT_TOK_SYSTEMLANGUAGE},
#line 372 "src/svg_att_keywords.gperf"
    {"diffuseConstant", SVG_ATT_TOK_DIFFUSECONSTANT},
#line 456 "src/svg_att_keywords.gperf"
    {"u1", SVG_ATT_TOK_U1},
#line 387 "src/svg_att_keywords.gperf"
    {"limitingConeAngle", SVG_ATT_TOK_LIMITINGCONEANGLE},
#line 280 "src/svg_att_keywords.gperf"
    {"color-profile", SVG_ATT_TOK_COLOR_PROFILE},
    {""}, {""},
#line 315 "src/svg_att_keywords.gperf"
    {"contentStyleType", SVG_ATT_TOK_CONTENTSTYLETYPE},
#line 363 "src/svg_att_keywords.gperf"
    {"k4", SVG_ATT_TOK_K4},
    {""},
#line 313 "src/svg_att_keywords.gperf"
    {"baseProfile", SVG_ATT_TOK_BASEPROFILE},
#line 279 "src/svg_att_keywords.gperf"
    {"marker-end", SVG_ATT_TOK_MARKER_END},
#line 331 "src/svg_att_keywords.gperf"
    {"x1", SVG_ATT_TOK_X1},
#line 362 "src/svg_att_keywords.gperf"
    {"k3", SVG_ATT_TOK_K3},
#line 288 "src/svg_att_keywords.gperf"
    {"flood-opacity", SVG_ATT_TOK_FLOOD_OPACITY},
    {""}, {""},
#line 437 "src/svg_att_keywords.gperf"
    {"ideographic", SVG_ATT_TOK_IDEOGRAPHIC},
#line 351 "src/svg_att_keywords.gperf"
    {"fx", SVG_ATT_TOK_FX},
#line 319 "src/svg_att_keywords.gperf"
    {"xlink:arcrole", SVG_ATT_TOK_XLINK_ARCROLE},
    {""},
#line 318 "src/svg_att_keywords.gperf"
    {"xlink:role", SVG_ATT_TOK_XLINK_ROLE},
    {""},
#line 422 "src/svg_att_keywords.gperf"
    {"font-variant", SVG_ATT_TOK_FONT_VARIANT},
    {""},
#line 412 "src/svg_att_keywords.gperf"
    {"path", SVG_ATT_TOK_PATH},
#line 438 "src/svg_att_keywords.gperf"
    {"alphabetic", SVG_ATT_TOK_ALPHABETIC},
#line 359 "src/svg_att_keywords.gperf"
    {"values", SVG_ATT_TOK_VALUES},
    {""}, {""},
#line 296 "src/svg_att_keywords.gperf"
    {"onfocusin", SVG_ATT_TOK_ONFOCUSIN},
#line 421 "src/svg_att_keywords.gperf"
    {"font-style", SVG_ATT_TOK_FONT_STYLE},
    {""}, {""}, {""},
#line 455 "src/svg_att_keywords.gperf"
    {"lang", SVG_ATT_TOK_LANG},
#line 262 "src/svg_att_keywords.gperf"
    {"text-decoration", SVG_ATT_TOK_TEXT_DECORATION},
#line 320 "src/svg_att_keywords.gperf"
    {"xlink:title", SVG_ATT_TOK_XLINK_TITLE},
#line 299 "src/svg_att_keywords.gperf"
    {"onclick", SVG_ATT_TOK_ONCLICK},
#line 407 "src/svg_att_keywords.gperf"
    {"keyTimes", SVG_ATT_TOK_KEYTIMES},
#line 413 "src/svg_att_keywords.gperf"
    {"keyPoints", SVG_ATT_TOK_KEYPOINTS},
    {""}, {""},
#line 427 "src/svg_att_keywords.gperf"
    {"panose-1", SVG_ATT_TOK_PANOSE_1},
#line 403 "src/svg_att_keywords.gperf"
    {"min", SVG_ATT_TOK_MIN},
    {""},
#line 394 "src/svg_att_keywords.gperf"
    {"viewTarget", SVG_ATT_TOK_VIEWTARGET},
    {""}, {""}, {""}, {""},
#line 408 "src/svg_att_keywords.gperf"
    {"keySplines", SVG_ATT_TOK_KEYSPLINES},
#line 354 "src/svg_att_keywords.gperf"
    {"patternTransform", SVG_ATT_TOK_PATTERNTRANSFORM},
#line 312 "src/svg_att_keywords.gperf"
    {"version", SVG_ATT_TOK_VERSION},
    {""},
#line 344 "src/svg_att_keywords.gperf"
    {"refY", SVG_ATT_TOK_REFY},
    {""}, {""},
#line 439 "src/svg_att_keywords.gperf"
    {"mathematical", SVG_ATT_TOK_MATHEMATICAL},
    {""}, {""}, {""},
#line 306 "src/svg_att_keywords.gperf"
    {"x", SVG_ATT_TOK_X},
#line 436 "src/svg_att_keywords.gperf"
    {"bbox", SVG_ATT_TOK_BBOX},
#line 341 "src/svg_att_keywords.gperf"
    {"glyphRef", SVG_ATT_TOK_GLYPHREF},
#line 316 "src/svg_att_keywords.gperf"
    {"transform", SVG_ATT_TOK_TRANSFORM},
#line 297 "src/svg_att_keywords.gperf"
    {"onfocusout", SVG_ATT_TOK_ONFOCUSOUT},
#line 259 "src/svg_att_keywords.gperf"
    {"glyph-orientation-vertical", SVG_ATT_TOK_GLYPH_ORIENTATION_VERTICAL},
#line 275 "src/svg_att_keywords.gperf"
    {"fill-opacity", SVG_ATT_TOK_FILL_OPACITY},
#line 258 "src/svg_att_keywords.gperf"
    {"glyph-orientation-horizontal", SVG_ATT_TOK_GLYPH_ORIENTATION_HORIZONTAL},
#line 343 "src/svg_att_keywords.gperf"
    {"refX", SVG_ATT_TOK_REFX},
    {""},
#line 360 "src/svg_att_keywords.gperf"
    {"k1", SVG_ATT_TOK_K1},
#line 334 "src/svg_att_keywords.gperf"
    {"y2", SVG_ATT_TOK_Y2},
    {""},
#line 446 "src/svg_att_keywords.gperf"
    {"underline-thickness", SVG_ATT_TOK_UNDERLINE_THICKNESS},
    {""},
#line 345 "src/svg_att_keywords.gperf"
    {"markerWidth", SVG_ATT_TOK_MARKERWIDTH},
#line 257 "src/svg_att_keywords.gperf"
    {"baseline-shift", SVG_ATT_TOK_BASELINE_SHIFT},
#line 431 "src/svg_att_keywords.gperf"
    {"x-height", SVG_ATT_TOK_X_HEIGHT},
    {""}, {""}, {""},
#line 346 "src/svg_att_keywords.gperf"
    {"markerHeight", SVG_ATT_TOK_MARKERHEIGHT},
    {""}, {""},
#line 304 "src/svg_att_keywords.gperf"
    {"onmouseout", SVG_ATT_TOK_ONMOUSEOUT},
#line 264 "src/svg_att_keywords.gperf"
    {"font-family", SVG_ATT_TOK_FONT_FAMILY},
#line 442 "src/svg_att_keywords.gperf"
    {"v-alphabetic", SVG_ATT_TOK_V_ALPHABETIC},
    {""},
#line 301 "src/svg_att_keywords.gperf"
    {"onmouseup", SVG_ATT_TOK_ONMOUSEUP},
    {""}, {""},
#line 449 "src/svg_att_keywords.gperf"
    {"overline-position", SVG_ATT_TOK_OVERLINE_POSITION},
    {""},
#line 388 "src/svg_att_keywords.gperf"
    {"tableValues", SVG_ATT_TOK_TABLEVALUES},
    {""}, {""},
#line 350 "src/svg_att_keywords.gperf"
    {"gradientTransform", SVG_ATT_TOK_GRADIENTTRANSFORM},
    {""},
#line 284 "src/svg_att_keywords.gperf"
    {"mask", SVG_ATT_TOK_MASK},
#line 308 "src/svg_att_keywords.gperf"
    {"width", SVG_ATT_TOK_WIDTH},
    {""}, {""}, {""}, {""},
#line 452 "src/svg_att_keywords.gperf"
    {"glyph-name", SVG_ATT_TOK_GLYPH_NAME},
#line 303 "src/svg_att_keywords.gperf"
    {"onmousemove", SVG_ATT_TOK_ONMOUSEMOVE},
    {""},
#line 249 "src/svg_att_keywords.gperf"
    {"xml:lang", SVG_ATT_TOK_XML_LANG},
#line 261 "src/svg_att_keywords.gperf"
    {"letter-spacing", SVG_ATT_TOK_LETTER_SPACING},
#line 430 "src/svg_att_keywords.gperf"
    {"cap-height", SVG_ATT_TOK_CAP_HEIGHT},
#line 302 "src/svg_att_keywords.gperf"
    {"onmouseover", SVG_ATT_TOK_ONMOUSEOVER},
#line 440 "src/svg_att_keywords.gperf"
    {"hanging", SVG_ATT_TOK_HANGING},
    {""},
#line 283 "src/svg_att_keywords.gperf"
    {"clip-path", SVG_ATT_TOK_CLIP_PATH},
    {""},
#line 405 "src/svg_att_keywords.gperf"
    {"repeatCount", SVG_ATT_TOK_REPEATCOUNT},
#line 260 "src/svg_att_keywords.gperf"
    {"kerning", SVG_ATT_TOK_KERNING},
    {""}, {""}, {""},
#line 266 "src/svg_att_keywords.gperf"
    {"font-size-adjust", SVG_ATT_TOK_FONT_SIZE_ADJUST},
#line 424 "src/svg_att_keywords.gperf"
    {"font-stretch", SVG_ATT_TOK_FONT_STRETCH},
#line 400 "src/svg_att_keywords.gperf"
    {"begin", SVG_ATT_TOK_BEGIN},
#line 416 "src/svg_att_keywords.gperf"
    {"horiz-origin-y", SVG_ATT_TOK_HORIZ_ORIGIN_Y},
    {""},
#line 378 "src/svg_att_keywords.gperf"
    {"baseFrequency", SVG_ATT_TOK_BASEFREQUENCY},
#line 310 "src/svg_att_keywords.gperf"
    {"viewBox", SVG_ATT_TOK_VIEWBOX},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
#line 459 "src/svg_att_keywords.gperf"
    {"g2", SVG_ATT_TOK_G2},
#line 445 "src/svg_att_keywords.gperf"
    {"underline-position", SVG_ATT_TOK_UNDERLINE_POSITION},
    {""}, {""}, {""},
#line 458 "src/svg_att_keywords.gperf"
    {"u2", SVG_ATT_TOK_U2},
    {""},
#line 289 "src/svg_att_keywords.gperf"
    {"lighting-color", SVG_ATT_TOK_LIGHTING_COLOR},
#line 317 "src/svg_att_keywords.gperf"
    {"xlink:href", SVG_ATT_TOK_XLINK_HREF},
#line 300 "src/svg_att_keywords.gperf"
    {"onmousedown", SVG_ATT_TOK_ONMOUSEDOWN},
    {""}, {""}, {""}, {""}, {""},
#line 333 "src/svg_att_keywords.gperf"
    {"x2", SVG_ATT_TOK_X2},
    {""}, {""}, {""}, {""},
#line 365 "src/svg_att_keywords.gperf"
    {"kernelMatrix", SVG_ATT_TOK_KERNELMATRIX},
#line 441 "src/svg_att_keywords.gperf"
    {"v-ideographic", SVG_ATT_TOK_V_IDEOGRAPHIC},
    {""}, {""},
#line 370 "src/svg_att_keywords.gperf"
    {"kernelUnitLength", SVG_ATT_TOK_KERNELUNITLENGTH},
#line 263 "src/svg_att_keywords.gperf"
    {"word-spacing", SVG_ATT_TOK_WORD_SPACING},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 460 "src/svg_att_keywords.gperf"
    {"k", SVG_ATT_TOK_K},
    {""},
#line 404 "src/svg_att_keywords.gperf"
    {"max", SVG_ATT_TOK_MAX},
    {""},
#line 325 "src/svg_att_keywords.gperf"
    {"pathLength", SVG_ATT_TOK_PATHLENGTH},
    {""},
#line 426 "src/svg_att_keywords.gperf"
    {"units-per-em", SVG_ATT_TOK_UNITS_PER_EM},
#line 419 "src/svg_att_keywords.gperf"
    {"vert-origin-y", SVG_ATT_TOK_VERT_ORIGIN_Y},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
#line 443 "src/svg_att_keywords.gperf"
    {"v-mathematical", SVG_ATT_TOK_V_MATHEMATICAL},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""},
#line 415 "src/svg_att_keywords.gperf"
    {"horiz-origin-x", SVG_ATT_TOK_HORIZ_ORIGIN_X},
#line 420 "src/svg_att_keywords.gperf"
    {"vert-adv-y", SVG_ATT_TOK_VERT_ADV_Y},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 361 "src/svg_att_keywords.gperf"
    {"k2", SVG_ATT_TOK_K2},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""},
#line 339 "src/svg_att_keywords.gperf"
    {"textLength", SVG_ATT_TOK_TEXTLENGTH},
    {""}, {""}, {""}, {""}, {""},
#line 423 "src/svg_att_keywords.gperf"
    {"font-weight", SVG_ATT_TOK_FONT_WEIGHT},
    {""}, {""},
#line 385 "src/svg_att_keywords.gperf"
    {"pointsAtY", SVG_ATT_TOK_POINTSATY},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""},
#line 384 "src/svg_att_keywords.gperf"
    {"pointsAtX", SVG_ATT_TOK_POINTSATX},
    {""}, {""}, {""},
#line 418 "src/svg_att_keywords.gperf"
    {"vert-origin-x", SVG_ATT_TOK_VERT_ORIGIN_X},
    {""}, {""},
#line 246 "src/svg_att_keywords.gperf"
    {"xmlns:xlink", SVG_ATT_TOK_XMLNS_XLINK},
    {""},
#line 358 "src/svg_att_keywords.gperf"
    {"in2", SVG_ATT_TOK_IN2},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 444 "src/svg_att_keywords.gperf"
    {"v-hanging", SVG_ATT_TOK_V_HANGING},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 417 "src/svg_att_keywords.gperf"
    {"horiz-adv-x", SVG_ATT_TOK_HORIZ_ADV_X}
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
#line 462 "src/svg_att_keywords.gperf"

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
