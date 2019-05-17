#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <stdbool.h>

#include "vkvg.h"

#include "test.h"

#define XML_PARSER_ERROR_BAD_CHAR = -2;

static const char* filePath = "data/tiger.svg";
//static const char* filePath = "data/test.xml";

bool wchar_is_space (wchar_t* c) {
    switch (*c) {
    case 0x20:
    case 0x9:
    case 0xD:
    case 0xA:
        return true;
    default:
        return false;
    }
}

bool fgetwc_first_char_after_spaces(FILE* pf, wchar_t* pChar) {
    while (!feof (pf)){
        *pChar = (wchar_t)fgetwc(pf);
        if (!wchar_is_space(pChar))
            return true;
    }
    return false;
}

// ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] |
// [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
bool is_valid_name_start_char (wchar_t c){
    if (
            (c == ':') ||
            (c == '_') ||
            (c >= 0x41 && c <= 0x5A) ||
            (c >= 0x61 && c <= 0x7A) ||
            (c >= 0xC0 && c <= 0xD6) ||
            (c >= 0xD8 && c <= 0xF6) ||
            (c >= 0xF8 && c <= 0x2FF) ||
            (c >= 0x370 && c <= 0x37D) ||
            (c >= 0x37F && c <= 0x1FFF) ||
            (c >= 0x200C && c <= 0x200D) ||
            (c >= 0x2070 && c <= 0x218F) ||
            (c >= 0x2C00 && c <= 0x2FEF) ||
            (c >= 0x3001 && c <= 0xD7FF) ||
            (c >= 0xF900 && c <= 0xFDCF) ||
            (c >= 0xFDF0 && c <= 0xFFFD) ||
            (c >= 0x10000 && c <= 0xEFFFF)
            )
        return true;
   return false;
}
bool is_valid_name_char (wchar_t c){
    if (is_valid_name_start_char (c) ||
            (c == '-') ||
            (c == '.') ||
            (c == 0xB7) ||
            (c >= 0x30 && c <= 0x39) ||
            (c >= 0x300 && c <= 0x36F) ||
            (c >= 0x203F && c <= 0x2040))
        return true;
   return false;
}
static bool get_next_wchar (FILE* pf, wchar_t* pC) {
    if (feof (pf)){
        perror("Unexpected end of file.");
        return false;
    }
    *pC = (wchar_t)fgetwc (pf);
    return true;
}
static bool peek_next_wchar (FILE* pf, wchar_t* pC) {
    if (feof (pf)){
        perror("Unexpected end of file.");
        return false;
    }
    wint_t c = fgetwc (pf);
    ungetwc(c, pf);
    *pC = (wchar_t)c;
    return true;
}
static inline void skip_space (FILE* pf, wchar_t* pc) {
    while (wchar_is_space (pc))
        get_next_wchar (pf, pc);
}
int parse_attributes (FILE* pf, const wchar_t* cur_element, VkvgContext ctx) {

}

enum parser_state {
    prolog,
    element,
    attribute
};

int parse_xml (FILE* pf, const wchar_t* cur_element, bool attributes, VkvgContext ctx) {
    wchar_t buff[1024];
    int buffLength=0;

    while (!feof (pf)){
        wchar_t c = (wchar_t)fgetwc (pf);

        switch (c) {
        case '<':
            peek_next_wchar(pf, &c);
            switch (c) {
            case '?':
                while (get_next_wchar (pf, &c)){
                    if (c == '>')
                        break;
                }
                break;
            case '!':
                //comments
                if (fgetws (buff, 3, pf) == NULL){
                    perror("Unexpected end of file.");
                    return -1;
                }
                if (wcscmp (buff+1, L"--") != 0){
                    perror ("Unexpected characters, expected '--'.");
                    return -1;
                }
                while (get_next_wchar (pf, &c)){
                    if (c != '-')
                        continue;
                    get_next_wchar (pf, &c);
                    if (c != '-')
                        continue;
                    get_next_wchar (pf, &c);
                    if (c == '>')
                        break;
                }
                break;
            case '/':
                if (cur_element == NULL) {
                    perror("Unexpected closing tag.");
                    return -1;
                }
                if (fgetws (buff, (int)wcslen(cur_element)+2, pf) == NULL){
                    perror("Unexpected end of file.");
                    return -1;
                }
                if (wcscmp(buff+1, cur_element)!=0) {
                    perror("Closing tag mismatch.");
                    return -1;
                }
                get_next_wchar (pf, &c);
                if (c != '>'){
                    perror("Expecting '>'.");
                    return -1;
                }
                return 0;
            default:
                if (!is_valid_name_start_char (c)){
                    perror ("Invalid first character for name");
                    return -1;
                }
                get_next_wchar (pf, &c);
                buff[buffLength++] = c;
                while (get_next_wchar (pf, &c)) {
                    if (wchar_is_space(&c))
                        break;
                    if (!is_valid_name_char (c)){
                        perror ("Invalid character in name");
                        return -1;
                    }
                    buff[buffLength++] = c;
                }
                buff[buffLength++] = 0;
                printf("%ls\n", buff);
                parse_xml (pf, buff, true, ctx);
                break;
            }
            break;
        case '>':
            if (!attributes){
                perror ("Unexpected '>'");
                return -1;
            }
            parse_xml (pf, cur_element, false, ctx);
            return 0;
            break;
        default:
            if (wchar_is_space(&c))
                break;
            if (cur_element == NULL){
                if (feof(pf))
                    return 0;
                perror ("Unexpected character");
                return -1;
            }
            if (attributes){
                wchar_t valBuff[1024];
                int valBuffLength=0;

                //self closing tag
                if (c == '/'){
                    get_next_wchar (pf, &c);
                    if (c != '>'){
                        perror ("Expecting '>'");
                        return -1;
                    }
                    return 0;
                }

                //attribute
                if (!is_valid_name_start_char (c)){
                    perror ("Invalid first character for name");
                    return -1;
                }
                //name
                buff[buffLength++] = c;
                while (get_next_wchar (pf, &c)) {
                    if (wchar_is_space(&c) || c == '=')
                        break;
                    if (!is_valid_name_char (c)){
                        perror ("Invalid character in name");
                        return -1;
                    }
                    buff[buffLength++] = c;
                }
                buff[buffLength++] = 0;
                //eq
                skip_space (pf, &c);
                if (c != '='){
                    perror ("Expecting '='");
                    return -1;
                }
                get_next_wchar (pf, &c);
                skip_space (pf, &c);
                if (c == '"'){
                    while (get_next_wchar(pf, &c)){
                        if (c=='"')
                            break;
                        valBuff[valBuffLength++] = c;
                    }
                } else if (c == '\''){
                    while (get_next_wchar(pf, &c)){
                        if (c=='\'')
                            break;
                        valBuff[valBuffLength++] = c;
                    }
                }else {
                    perror ("Expecting attribute value.");
                    return -1;
                }
                valBuff[valBuffLength++] = 0;

                printf("\t%ls = %ls\n", buff, valBuff);
                buffLength = valBuffLength = 0;
            }
        }
    }
}
int parser_test() {
    enum parser_state state = prolog;


    FILE *pf = fopen ( filePath, "r" );

    if (pf == NULL)
        return -1;

    setlocale(LC_ALL, "");

    VkvgContext ctx = vkvg_create(surf);

    parse_xml (pf, NULL, false, ctx);

    vkvg_destroy(ctx);

    fclose (pf);
    return 0;
}


int main(int argc, char *argv[]) {

    perform_test (parser_test, 1024, 768);
    return 0;
}

