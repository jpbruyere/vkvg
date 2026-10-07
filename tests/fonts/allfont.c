#include "test.h"
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H

float fontSize = 16.0f;
int   curFontStartIdx = 0;
bool  redraw = true;

#define MAX_STRING_LEN 1024

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
    { "th",    "เป็นมนุษย์สุดประเสริฐเลิศคุณค่า" },                      // Thai
    { "hy",    "Ֆիզիկոս Մկրտիչը օճառաջուր ցողելով բժշկում է գնդապետ Հայկի փքված ձախ թևը։" }, // Armenian
    { "agr",   "Agatjai juuk tusha numpanum ijuaku jintanum pujawai" }, // Aguaruna (Latin script variation)
    { "aa",    "Afari fanta asat darih doro dahanik xisbisa" },         // Afar (Latin script variation)
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

void get_sample_by_fc_lang(const char* fc_lang, char* str) {
    const char* sample = NULL;
    if (fc_lang) {
        for (int i = 1; i < 38; i++) {
            if (strncmp(fc_lang, script_samples[i].lang_code, strlen(script_samples[i].lang_code)) == 0) {
                sample = script_samples[i].sample_text;
                break;
            }
        }
        if (!sample) {
            sample = script_samples[0].sample_text;
            printf("missing lang sample: %s\n", fc_lang);
        }
    }

    // Check if the fontconfig language string starts with a known code
    strncpy(str, sample, strlen(sample)+1);
}


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
        if (!lang_list)
            return false;

        lang = FcStrListNext(lang_list);

        if (!lang)
            return false;
        //printf("lang: %s\n", str);
        get_sample_by_fc_lang((char*)lang, str);

        FcStrListDone(lang_list);
        return true;
    }
    return false;
}


void draw(VkvgSurface surfFont) {
    if (!font_database)
        return;
    VkvgContext ctx = vkvg_create(surfFont);
    vkvg_clear(ctx);
    vkvg_set_source_rgb(ctx,1,1,1);

    float penX = 10.f;
    float penY = 50.f;

    char tmp[1024];

    for (int i = curFontStartIdx; i < font_database->nfont; i++) {
        FcPattern* font_entry = font_database->fonts[i];

        FcChar8* family_name = NULL;
        FcChar8* style_variant = NULL;
        FcChar8* file_path = NULL;

        if (FcPatternGetString(font_entry, FC_FAMILY, 0, &family_name) == FcResultMatch &&
            FcPatternGetString(font_entry, FC_STYLE, 0, &style_variant) == FcResultMatch) {

            FcPatternGetString(font_entry, FC_FILE, 0, &file_path);
            sprintf(tmp, "%s", family_name);


            VkvgFont font = vkvg_font_create (device, tmp, fontSize);
            if (vkvg_font_status(font)) {
                printf("Error trying to load %s, from %s\n", tmp, file_path);
                fflush(stdout);
                continue;
            }
            vkvg_set_font (ctx, font);
            vkvg_move_to (ctx, penX, penY);

            FT_Face face = (FT_Face)vkvg_font_get_face(font);
            if (!try_get_sample_string(font_entry, face, tmp))
                sprintf(tmp, "%s : %s", family_name, style_variant);

            vkvg_show_text (ctx,tmp);
            vkvg_flush(ctx);
            vkvg_font_destroy(font);

            penY += fontSize * 1.5f;
        }
        if (penY > test_height - 100)
            break;
        tmp[0] = '\0';
    }

    vkvg_destroy(ctx);
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
    }
    redraw = true;
}
static void mouse_move_callback(GLFWwindow* window, double x, double y) {
}
static void scroll_callback(GLFWwindow* window, double x, double y) {
    if (y > 0.f) {
        if (curFontStartIdx > 0)
            curFontStartIdx --;
    } else if (font_database) {
        if (curFontStartIdx < font_database->nfont)
        curFontStartIdx ++;
    }
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
    vkh_log_level = VKVG_LOG_ERR;

    _parse_args(argc, argv);
    VkEngine e = vkengine_create (
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);

    VkhPresenter r = e->renderer;
    vkengine_set_key_callback(e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    vkvg_device_create_info_t info = {
                                      4, false,
                                      vkh_app_get_inst(e->app),
                                      vkh_device_get_phy(e->dev),
                                      vkh_device_get_vkdev(e->dev),
                                      e->gQFamIdx, 0};

    device = vkvg_device_create(&info);
    surf = vkvg_surface_create(device, test_width, test_height);
    VkvgSurface surfFont = vkvg_surface_create(device, test_width, test_height);

    vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);

    if (!FcInit()) {
        fprintf(stderr, "Fatal Error: Failed to initialize Fontconfig.\n");
        return -1;
    }
    FcConfig* config = FcConfigGetCurrent();
    FcPattern* blank_pattern = FcPatternCreate();
    FcObjectSet* requested_elements = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_LANG, (char *)0);
    font_database = FcFontList(config, blank_pattern, requested_elements);

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

    vkvg_device_destroy(device);

    vkengine_destroy(e);

    FcFontSetDestroy(font_database);
    FcObjectSetDestroy(requested_elements);
    FcPatternDestroy(blank_pattern);
    FcFini();

    return 0;
}
