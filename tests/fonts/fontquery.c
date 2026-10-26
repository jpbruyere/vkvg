#include "test.h"
#ifdef VKVG_USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#ifdef VKVG_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H
#endif

#include <string.h>

float fontSize = 50.0f;
int   curFontStartIdx = 0;
bool  redraw = true;

#define MAX_STRING_LEN 1024

FcConfig* config = NULL;
FcFontSet* font_database = NULL;

typedef struct {
    const char* lang_code;
    const char* sample_text;
} LocaleSample;

// A localized library of pan-unicode text variations
static const LocaleSample script_samples[] = {
    { "en",    "The quick brown fox jumps over the lazy dog." },     // Latin Fallback
    { "ja",    "いろはにほへと ちりぬるを わかよたれそ つねならむ" },    // Japanese Hiragana
    { "zh",    "天地玄黃，宇宙洪荒。日月盈昃，辰宿列張。" },            // Chinese Hanzi
    { "ko",    "다람쥐 쳇바퀴에 타고파" },                            // Korean Hangul
    { "ru",    "Съешь же ещё этих мягких французских булок, да выпей чаю" }, // Cyrillic
    { "ar",    "نص حكيم له سر قاطع وذو شأن عظيم ينقص" },               // Arabic
    { "he",    "דג אכל גזר באשחט" },                                    // Hebrew
    { "hi",    "ऋषियों को तपस्या करते देख, दुष्टों के मन में डर पैदा हुआ।" }, // Hindi / Devanagari
    { "el",    "διαφυλάξτε γενικά τη ζωή σας από τον φόβο" },           // Greek
    { "th",    "เป็นมนุษย์สุดประเสริฐเลิIศคุณค่า" },                      // Thai
    { "hy",    "Ֆիզիկոս Մկրտիչը օճառաջուր ցողելով բժշկում է գնդապետ Հայկի փքված ձախ թևը։" }, // Armenian
    { "agr",   "Agatjai juuk tusha numpanum ijuaku jintanum pujawai" }, // Aguaruna (Latin script variation)
    //{ "aa",    "Afari fanta asat darih doro dahanik xisbisa" },         // Afar (Latin script variation)
    { "bem",   "Abantu bonse bafyalwa abalubuka no kulingana mu mucinshi" }, // Bemba (Bantu Latin script)
    { "pa",    "ਘੁਮਿਆਰ ਨੇ ਮਿੱਟੀ ਚੁੱਕ ਕੇ ਇੱਕ ਬਹੁਤ ਹੀ ਸੁੰਦਰ ਘੜਾ ਬਣਾਇਆ।" },    // Punjabi (Gurmukhi script)
    { "te",    "కచ్ఛపము మందగమనముతో చెరువు వైపు వెళుతోంది." },            // Telugu (Telugu script)
    { "km",    "ខ្យល់បក់បោកបបួលបក្សីហើរទៅកាន់ព្រៃព្រឹក្សាដ៏ស្ងប់ស្ងាត់។" }, // Khmer (Khmer script)
    { "lo",    "ນົກກາງແກບິນຂ້າມທົ່ງນາທີ່ກວ້າງໃຫຍ່ໄພສານຢ່າງສະຫງ່າງາມ." }, // Lao (Lao script)
    { "ta",    "தமிழ் மொழி உலகினில் மிக தொன்மையான மற்றும் அழகான மொழியாகும்." }, // Tamil (Tamil script)
    { "or",    "ଋଷିମାନେ ଅରଣ୍ୟ ମଧ୍ୟରେ ଧ୍ୟାନ କରି ଶାନ୍ତି ଲାଭ କରନ୍ତି।" },      // Odia/Oriya (Odia script)
    { "ml",    "ഭൂമിയിലെ എല്ലാ മനുഷ്യരും തുല്യ അവകാശങ്ങളോടെ ജനിക്കുന്നു." },  // Malayalam (Malayalam script)
    { "as",    "শৰৎকালৰ স্নিগ্ধ জোনাক ৰাতি আকাশখন বৰ ধুনীয়া দেখায়।" },       // Assamese (Bengali-Assamese script)
    { "si",    "සියලු මනුෂ්‍යයෝ නිදහස්ව උපත ලබා ඇත." },                  // Sinhala (Sinhala script)
    { "am",    "የሰው ልጅ ሁሉ ሲወለድ ነጻና በክብርም በሕግም እኩል ነው።" },         // Amharic (Ge'ez/Ethiopic script)
    { "anp",   "सभ्भे मानुस जनम सँ सुतन्तर आरो अधिकार में बराबर छै।" },   // Angika (Devanagari variant)
    { "kn",    "ಎಲ್ಲಾ ಮಾನವರು ಸ್ವತಂತ್ರರಾಗಿಯೇ ಜನಿಸಿದ್ದಾರೆ." },                // Kannada (Kannada script)
    { "gu",    "બધા મનુષ્યો ગૌરવ અને અધિકારોની બાબતમાં સમાન છે." },         // Gujarati (Gujarati script)
    { "bo",    "འགྲོ་བ་མི་ཉིད་མ་སྐྱེས་པ་ནས་བཟུང་རང་དབང་ཡིན།" },               // Tibetan (Tibetan script)
    { "ka",    "ადამიანი იბადება თავისუფალი და თანასწორი." },               // Georgian (Mkhedruli script)
    { "mnw",   "မန်ဝံသဂမၠိုၚ် က္တဵုဒှ်ကၠုၚ် နကဵုဂုဏ်သိက္ခာ ညဳသၟဟ်ဂမၠိုၚ်။" }, // Mon (Mon-Burmese script)
    { "ii",    "ꊿꊪꉼꆹꊪꊪꊿꐊꐥꌋꅍꐛꐹꂿꆪꄮꄯꉼꆹꄮꄯꂿꆪꊨꏦꍏꆪꐥꌋꅍꐛꐹꏦꍏꅪꏦꍏꅪꐨꏦꍏꄮꄯꅪ" }, // Nuosu / Liangshan Yi (Yi syllabary)
    { "chr",   "ᏂᎦᏛ ᎠᏂᏰᎸᎢ ᎤᏂᎲᏓ ᏂᎨᏒᎾ ᎤᎾᏕᎲᎩ ᎢᏳᏍᏗᏉ ᎨᏒ ᎦᎸᏉᏗᏳ" },    // Cherokee (Cherokee syllabary)
    { "ber-ma", "ⵎⴰⵕⵕⴰ ⵉⵎᏓⵏⵏ ⴷⴳ ⵓⵍⵉⴼ ⴷ ⵉⵣⵔⴼⴰⵏ ⴷ ⵍⵃⵇⵇ ⵜⵜⵍⴰⵍⴰⵏ ⴷ ⵉⵍⴻⵍⵍⵉⵢⵏ." }, // Moroccan Berber (Tifinagh script)
    { "ab",    "Дарбанზаалак ауаҩы дшоуп ихы ақәgenericуи иҳаgenericти." }, // Abkhaz (Cyrillic variation)
    { "syr",   "ܟܠ ܒܪܢܫܐ ܡܬܝܠܕ ܚܐܪܐ ܘܫܘܝܐ ܒܐܝܩܪܐ ܘܒܙܕܩܐ." },           // Syriac (Syriac script, right-to-left)
    { "iu",    "ᐃᓅᔪᓕᒫᑦ ᐊᓂᖅᑎᕆᔪᓕᒫᑦ ᐃᓅᕗᑦ ᐃᓱᒪᕐᓱᕐᖢᑎᒃ ᐊᒻᒪ ᐊᔾᔨᒌᒃᑐᒥᒃ" }, // Inuktitut (Canadian Aboriginal Syllabics)
    { "dv",    "ހުރިހާ އިންސާނުންވެސް އުފަންވަނީ މިނިވަންކަމާއި ޢިއްޒަތާއި" },   // Dhivehi / Maldivian (Thaana script, right-to-left)
    { "mn-cn", "Хүн бүр төрөхDiscussion эрх чөлөөтэй, нэр төр, эрхийн хувьд адилхан." }, // Mongolian - Inner Mongolia (Cyrillic fallback)
};

bool try_get_sample_by_fc_lang(const char* fc_lang, char* str) {
    const char* sample = NULL;
    if (fc_lang) {
        for (int i = 0; i < 37; i++) {
            if (!strncmp(fc_lang, script_samples[i].lang_code, strlen(script_samples[i].lang_code)) &&
                strlen(script_samples[i].lang_code) == strlen(fc_lang)) {
                sample = script_samples[i].sample_text;
                printf("sample found for %s->%s\n", fc_lang, script_samples[i].lang_code);
                break;
            }
        }
        if (!sample) {
            sample = script_samples[0].sample_text;
            return false;
        }
    }

    strncpy(str, sample, strlen(sample)+1);
    return true;
}

#ifdef VKVG_USE_FREETYPE
void convert_utf16be_to_utf8(const FT_Byte* src, FT_UInt src_len, char* dest, FT_UInt dest_max_len) {
    FT_UInt d_idx = 0;

    // Iterate 2 bytes at a time for UTF-16
    for (FT_UInt s_idx = 0; s_idx < src_len && d_idx < (dest_max_len - 4); s_idx += 2) {
        // Reconstruct the 16-bit Unicode Code Point (Big Endian)
        unsigned short cp = (src[s_idx] << 8) | src[s_idx + 1];

               // 1-byte UTF-8 (Standard ASCII Range)
        if (cp < 0x80) {
            dest[d_idx++] = (char)cp;
        }
        // 2-byte UTF-8 (Cyrillic, Hebrew, Arabic, etc.)
        else if (cp < 0x800) {
            dest[d_idx++] = (char)(0xC0 | (cp >> 6));
            dest[d_idx++] = (char)(0x80 | (cp & 0x3F));
        }
        // 3-byte UTF-8 (Chinese, Japanese, Devanagari, Hangul, etc.)
        else {
            dest[d_idx++] = (char)(0xE0 | (cp >> 12));
            dest[d_idx++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            dest[d_idx++] = (char)(0x80 | (cp & 0x3F));
        }
    }
    dest[d_idx] = '\0'; // Properly null-terminate
}

bool try_get_sample_string(FcPattern* font_entry, FT_Face face, char* str) {
    // Check if the font has an SFNT name table (TrueType/OpenType files)
    if (!FT_IS_SFNT(face)) {
        return false;
    }

    FT_UInt count = FT_Get_Sfnt_Name_Count(face);
    FT_SfntName name_rec;

    for (FT_UInt i = 0; i < count; i++) {
        if (FT_Get_Sfnt_Name(face, i, &name_rec) == 0) {
            // 19 is the pre-defined ID for "Sample text"
            if (name_rec.name_id == 19) {
                if (name_rec.platform_id == 3 || name_rec.platform_id == 0) {
                    convert_utf16be_to_utf8(name_rec.string, name_rec.string_len, str, MAX_STRING_LEN);
                } else {
                    memcpy(str, name_rec.string, name_rec.string_len);
                    str[name_rec.string_len] = '\0';
                }

                return true;
            }
        }
    }
    FcLangSet* langset = NULL;
    FcChar8* lang = NULL;
    if (FcPatternGetLangSet(font_entry, FC_LANG, 0, &langset) == FcResultMatch) {
        FcStrSet *lang_set = FcLangSetGetLangs(langset);
        if (!lang_set)
            return false;
        FcStrList *lang_list = FcStrListCreate(lang_set);
        if (!lang_list) {
            FcStrListDone(lang_list);
            return false;
        }

        while ((lang = FcStrListNext(lang_list))) {
            if (!lang)
                continue;
            if (try_get_sample_by_fc_lang((char*)lang, str))
                break;
        }

        FcStrListDone(lang_list);
        FcStrSetDestroy(lang_set);
        return true;
    }
    return false;
}
#endif

#define MAX_TXT_LEN 1024
VkvgFont sans16 = NULL;
char txtBuffer[MAX_TXT_LEN];
int txtBuffPtr = 0;
bool vertical = false;
bool variable = false;

float uiFontSize = 18.f;

void encode_utf8(uint64_t cp, char* buffer) {
    // 1. Filter out invalid Unicode code points
    if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
        buffer[0] = '\0'; // Return an empty string for invalid code points
        return;
    }

           // 2. Encode based on the value range of the code point
    if (cp <= 0x7F) {
        // 1 byte sequence
        buffer[0] = (char)cp;
        buffer[1] = '\0';
    } else if (cp <= 0x7FF) {
        // 2 byte sequence
        buffer[0] = (char)(0xC0 | (cp >> 6));
        buffer[1] = (char)(0x80 | (cp & 0x3F));
        buffer[2] = '\0';
    } else if (cp <= 0xFFFF) {
        // 3 byte sequence
        buffer[0] = (char)(0xE0 | (cp >> 12));
        buffer[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buffer[2] = (char)(0x80 | (cp & 0x3F));
        buffer[3] = '\0';
    } else {
        // 4 byte sequence
        buffer[0] = (char)(0xF0 | (cp >> 18));
        buffer[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        buffer[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buffer[3] = (char)(0x80 | (cp & 0x3F));
        buffer[4] = '\0';
    }
}

void draw(VkvgSurface surfFont) {
    if (!font_database)
        return;
    VkvgContext ctx = vkvg_create(surfFont);
    vkvg_clear(ctx);

    float xQ = 10;
    float yQ = 20;

    vkvg_set_font(ctx,sans16);
    vkvg_set_source_rgb(ctx,1,1,1);

    vkvg_font_extents_t fe = {0};
    vkvg_font_extents(ctx, &fe);

    vkvg_rectangle(ctx, xQ-2, yQ - fe.ascent, fe.max_x_advance + MAX_TXT_LEN + 4, fe.height);
    vkvg_set_line_width(ctx, 1);
    vkvg_stroke(ctx);

    vkvg_move_to(ctx, xQ, yQ);
    vkvg_show_text(ctx, txtBuffer);

    char txtBuffCopy[MAX_TXT_LEN];
    char tmp[MAX_TXT_LEN];
    strcpy(txtBuffCopy,txtBuffer);

    FcPattern* request = FcPatternCreate();
    FcObjectSet* requested_elements = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_LANG, FC_VARIABLE, (char *)0);

    char *style = strstr(txtBuffCopy, ":");
    if (style) {
        *style='\0';
        style++;
        if (strlen(style) > 0) {
            FcPatternAddString(request, FC_STYLE, (const FcChar8 *)style);
        } else {
            style = NULL;
        }
    }
    FcPatternAddString(request, FC_FAMILY, (const FcChar8 *)txtBuffCopy);
    if (variable)
        FcPatternAddBool(request, FC_VARIABLE, FcTrue);
    //FcPatternAddBool(request, FC_SCALABLE, FcTrue);
    //FcPatternAddString(request, FC_FONTFORMAT, (const FcChar8 *)"TrueType");

    //FcConfigSubstitute(config, request, FcMatchPattern);
    //FcDefaultSubstitute(request);

    FcFontSet* fonts = FcFontList(config, request, requested_elements);

    FcChar8* family_name = NULL;
    FcChar8* style_variant = NULL;
    FcChar8* fontFile = NULL;
    FcBool var = false;
    int index = 0;

    float penX = 10;
    float penY = 50;



    FcPattern* font = NULL;
    if (fonts && fonts->nfont == 1) {
        font = fonts->fonts[0];

        FcPatternGetString (font, FC_FILE, 0, &fontFile);
        FcPatternGetInteger (font, FC_INDEX, 0, &index);
        FcPatternGetString (font, FC_FAMILY, 0, &family_name);
        FcPatternGetString (font, FC_STYLE, 0, &style_variant);
        FcPatternGetBool(font, FC_VARIABLE, 0, &var);
        VkvgFont f = vkvg_font_create_from_file(surf, (char*)fontFile, index, fontSize);
        if (vkvg_font_status(f)) {
            printf("Error trying to load %s, from %s\n", txtBuffCopy, fontFile);
            fflush(stdout);
            goto clean_exit;
        }
        vkvg_set_font (ctx, f);
        if (vertical)
            vkvg_set_text_direction (ctx, VKVG_VERTICAL);

        penX = 200;
        penY = 100;
        sprintf(txtBuffCopy, "%s : %s", family_name, style_variant);

        vkvg_font_extents_t fe = {0};
        vkvg_font_extents(ctx, &fe);
        VkvgText tr = vkvg_text_create(ctx, txtBuffCopy);
        vkvg_text_extents_t extents = {0};
        vkvg_text_get_extents(tr, &extents);
        vkvg_rectangle(ctx, penX, penY - fe.ascent, extents.width, extents.height);
        vkvg_set_line_width(ctx, 1);
        vkvg_set_source_rgba(ctx,0.5f,0.5f,1, 0.7f);
        vkvg_stroke(ctx);

        vkvg_move_to (ctx, penX, penY);
        vkvg_set_source_rgba(ctx,1,0,0,0.5f);
        vkvg_arc(ctx, penX, penY, 3, 0, 2 * M_PI);
        vkvg_fill(ctx);

        vkvg_move_to (ctx, penX, penY);
        vkvg_set_source_rgb(ctx,1,1,1);
        vkvg_text_render(ctx,tr);

        penX = 50;
        penY = 150;

        FcLangSet* langset = NULL;
        FcChar8* lang = NULL;
        if (FcPatternGetLangSet(font, FC_LANG, 0, &langset) == FcResultMatch) {
            FcStrSet *lang_set = FcLangSetGetLangs(langset);
            if (lang_set) {
                FcStrList *lang_list = FcStrListCreate(lang_set);
                if (lang_list) {
                    while ((lang = FcStrListNext(lang_list))) {
                        if (!lang)
                            continue;
                        if (try_get_sample_by_fc_lang((char*)lang, tmp)) {
                            vkvg_move_to (ctx, penX, penY);
                            vkvg_show_text(ctx, tmp);
                            penY += fe.height;
                        }
                    }
                }
                FcStrListDone(lang_list);
            }
            FcStrSetDestroy(lang_set);
        }

        /*int charPerLine = (test_width - 100) / fe.max_x_advance;
        int visibleLines = (test_height - 150) / fe.height;
        int totLines = (double)0xffffffff / (double)charPerLine;

        char buff[10];

        int initI = curFontStartIdx * charPerLine;

        for (uint64_t i = initI; i < 0xffffffff; ++i) {
            encode_utf8(i, buff);
            vkvg_move_to (ctx, penX, penY);
            vkvg_show_text(ctx, buff);

            penX += fe.max_x_advance;
            if (penX > test_width - 50 - fe.max_x_advance) {
                penY += fe.height;
                penX = 50;
            }
            if (penY > test_height)
                break;
        }

        vkvg_rectangle(ctx, test_width - 50, 150, 20, test_height - 170);
        vkvg_stroke(ctx);

        float lineH = (float)(test_height - 170) / (float)totLines;

        vkvg_rectangle(ctx, test_width - 48, 150 + lineH * (float)initI, 16, 2);//lineH * visibleLines);
        vkvg_fill(ctx);*/

        vkvg_flush(ctx);
        vkvg_text_destroy(tr);
        vkvg_font_destroy(f);
    } else {
        for (int i = curFontStartIdx; i < font_database->nfont; i++) {
            font = font_database->fonts[i];
            FcPatternGetString (font, FC_FILE, 0, &fontFile);
            FcPatternGetInteger (font, FC_INDEX, 0, &index);
            FcPatternGetString (font, FC_FAMILY, 0, &family_name);
            FcPatternGetString (font, FC_STYLE, 0, &style_variant);
            FcPatternGetBool(font, FC_VARIABLE, 0, &var);

            if (variable && !var)
                continue;

            if (family_name && strstr((char*)family_name, (char*)txtBuffCopy)) {
                if (!style || (style && style_variant && strstr((char*)style_variant, style))) {
                    sprintf(tmp, "%s:%s index=%d %s", family_name,style_variant, index, fontFile);
                    vkvg_move_to(ctx, penX, penY);
                    vkvg_show_text(ctx, tmp);
                    penY+=fe.height;

                    if (penY > test_height - 20)
                        break;
                }
            }
        }
    }
clean_exit:
    FcPatternDestroy(request);
    FcFontSetDestroy(fonts);
    FcObjectSetDestroy(requested_elements);

    vkvg_destroy(ctx);
}

static void char_callback(GLFWwindow *window, unsigned int codepoint) {
    if (txtBuffPtr >= MAX_TXT_LEN - 1)
        return;
    txtBuffer[txtBuffPtr] = (char)codepoint;
    txtBuffer[++txtBuffPtr] = '\0';
    redraw = true;
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE)
        return;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_KP_ADD:
        fontSize++;
        break;
    case GLFW_KEY_KP_SUBTRACT:
        if (fontSize > 5.f)
            fontSize--;
        break;
    case GLFW_KEY_BACKSPACE:
        if (txtBuffPtr > 0) {
            txtBuffPtr--;
        }
        txtBuffer[txtBuffPtr]='\0';
        break;
    case GLFW_KEY_PAGE_DOWN:
        //curFontStartIdx += 0xff;
        if (fontSize > 5.f)
            fontSize--;
        break;
    case GLFW_KEY_PAGE_UP:
        //curFontStartIdx -= 0xff;
        //curFontStartIdx = MAX(curFontStartIdx, 0);
        fontSize++;
        break;
    case GLFW_KEY_F3:
        variable = !variable;
        break;
    case GLFW_KEY_F4:
        vertical = !vertical;
        break;
    /*case GLFW_KEY_DELETE:
        if (txtBuffPtr > 0 && txtBuffPtr < MAX_TXT_LEN - 2) {
            memmove(&txtBuffer[txtBuffPtr], (&txtBuffer[txtBuffPtr + 1], MAX_TXT_LEN - txtB)
        }
        break;*/
    }
    redraw = true;
}
static void mouse_move_callback(GLFWwindow* window, double x, double y) {
}
static void scroll_callback(GLFWwindow* window, double x, double y) {
#ifdef VKVG_USE_FONTCONFIG

    const int step = 10;
    if (y > 0.f) {
        if (curFontStartIdx > 0)
            curFontStartIdx -= step;
        curFontStartIdx = MAX(curFontStartIdx, 0);
    } else {
        curFontStartIdx +=step;
    }
#endif
    redraw = true;
}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif) {
    if (but != GLFW_MOUSE_BUTTON_1)
        return;
    if (state == GLFW_TRUE)
        mouseDown = true;
    else
        mouseDown = false;
    redraw = true;
}

int main(int argc, char* argv[]) {
    vkh_log_level = VKH_LOG_FULL;
    vkvg_log_level = VKVG_LOG_ERR | VKVG_LOG_WARN | VKVG_LOG_FONT ;

    _parse_args(argc, argv);
    VkEngine e = vkengine_create (
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);

    VkhPresenter r = e->renderer;
    vkengine_set_key_callback(e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);
    vkengine_set_char_callback(e, char_callback);

    vkvg_device_create_info_t info = {
        vkh_app_get_inst(e->app),
        vkh_device_get_phy(e->dev),
        vkh_device_get_vkdev(e->dev),
        e->gQFamIdx, 0, false
    };//, TESTS_DATA_ROOT "/fonts"};

    device = vkvg_device_create(&info);
    surf = vkvg_surface_create(device, test_width, test_height);
    VkvgSurface surfFont = vkvg_surface_create(device, test_width, test_height);

    vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);

#ifdef VKVG_USE_FONTCONFIG

    if (!FcInit()) {
        fprintf(stderr, "Fatal Error: Failed to initialize Fontconfig.\n");
        return -1;
    }
    config = FcConfigGetCurrent();
    FcPattern* request = FcPatternCreate();
    FcObjectSet* requested_elements = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_LANG, FC_VARIABLE, (char *)0);
    //FcPatternAddBool(request, FC_SCALABLE, FcTrue);
    //FcPatternAddString(request, FC_FONTFORMAT, (const FcChar8 *)"TrueType");

    font_database = FcFontList(config, request, requested_elements);
#endif

    strcpy (txtBuffer, "sans");
    txtBuffPtr = 4;
    sans16 = vkvg_font_create(surf, "sans", 16);

    while (!vkengine_should_close(e)) {
        glfwPollEvents();

        if (redraw) {
            draw(surfFont);
            VkvgContext ctx = vkvg_create(surf);
            vkvg_clear(ctx);
            vkvg_set_source_surface(ctx,surfFont,0,0);
            vkvg_paint(ctx);
            vkvg_destroy(ctx);
            redraw = false;
        }

        if (!vkh_presenter_draw(r)) {
            vkh_presenter_get_size(r, &test_width, &test_height);
            vkvg_surface_destroy(surf);
            vkvg_surface_destroy(surfFont);
            surf = vkvg_surface_create(device, test_width, test_height);
            surfFont = vkvg_surface_create(device, test_width, test_height);
            vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);
            vkengine_wait_idle(e);
            redraw = true;
            continue;
        }
    }
    vkengine_wait_idle(e);

    vkvg_surface_destroy(surfFont);
    vkvg_surface_destroy(surf);

    vkvg_font_destroy(sans16);

    vkvg_device_destroy(device);

    vkengine_destroy(e);

#ifdef VKVG_USE_FONTCONFIG
    FcFontSetDestroy(font_database);
    FcObjectSetDestroy(requested_elements);
    FcPatternDestroy(request);
    FcFini();
#endif

    return 0;
}
