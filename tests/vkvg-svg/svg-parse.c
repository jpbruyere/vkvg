//#include "vkvg.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#include "parser_gen.h"

// A perfectly optimized, branchless UTF-8 length lookup table
static const uint8_t utf8_len_table[256] = {
    // 0x00 to 0x7F: Standard ASCII (1 byte)
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

    // 0x80 to 0xBF: Continuation bytes (invalid as first byte, but default to 1 to prevent infinite loops)
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

    // 0xC0 to 0xDF: Multi-byte leaders (2 bytes)
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,

    // 0xE0 to 0xEF: Multi-byte leaders (3 bytes)
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,

    // 0xF0 to 0xF7: Multi-byte leaders (4 bytes)
    4,4,4,4,4,4,4,4,

    // 0xF8 to 0xFF: Invalid UTF-8 (safely fallback to 1)
    1,1,1,1,1,1,1,1
};

int get_utf8_char_length(uint8_t first_byte) {
    return utf8_len_table[first_byte];
}

#define ISWHITESPACE(c) (c == 0x9 || c == 0x20 || c == 0xA || c == 0xD)
#define READ if (++ptr < buff_size) c = buff[ptr]; else break

void print_tabs(int n) {
    while (n-- > 0) {
        putchar('\t'); // Ou ' ' si tu préfères des espaces
    }
}

typedef struct {
    uint8_t *buff;
    size_t offset;
    size_t len;
} _stream_t;

typedef _stream_t *const restrict stream;

int parse_children(svg_context *const svg);

int parse_element_tag(svg_context *const svg) {
    static uint8_t level = 0;
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;
    while (buff < buff_end) {
        if (*buff > 64) { //attribute name start
            svg->att = buff;
            while (++buff < buff_end) {
                if (*buff < 65) {
                    if (*buff == ':') {
                        svg->ns = svg->att;
                        svg->ns_len = buff - 1 - svg->ns;
                        svg->att = buff + 1;
                        continue;
                    }
                    svg->att_len = buff - svg->att;
                    print_tabs(level);
                    printf("\tAttribute: ");
                    fwrite(svg->att, sizeof(uint8_t), svg->att_len,stdout);
                    //expecting '=' or white space
                    while (*buff != '=') {
                        if (++buff == buff_end) {
                            perror("malformed xml, expecting '=' or white space.\n");
                            return -1;
                        }
                    }
                    while (++buff < buff_end) {
                        if (*buff == '"')
                            break;
                    }
                    svg->value = buff + 1;
                    while (++buff < buff_end) {
                        if (*buff == '"')
                            break;
                    }
                    svg->value_len = buff - svg->value;
                    printf("=");
                    fwrite(svg->value, sizeof(uint8_t), svg->value_len,stdout);
                    printf("\n");
                    fflush(stdout);
                    break;
                }
            }
        } else if (*buff == '>') {
            //read element childrens
            svg->buff_ptr = ++buff;
            level++;
            print_tabs(level);
            parse_children(svg);
            buff = svg->buff_ptr;
            level--;
            return 0;
        } else if (*buff == '/') {
            //self closing tag
            if (++buff < buff_end && *buff == '>') {
                svg->buff_ptr = ++buff;
                return 0;
            }
            perror("malformed xml, expecting '>'\n");
            return -1;
        } else if (*buff == '<') {
            perror("malformed xml, unexpected '<', expecting '>'\n");
            return -1;
        } else
            buff++;
    }
    return -1;
}

int parse_children(svg_context *const svg) {
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;
    //store current element name
    const uint8_t *const ns = svg->ns;
    const uint8_t *const elt = svg->elt;
    const size_t ns_len = svg->ns_len;
    const size_t elt_len = svg->elt_len;
    uint8_t c = 0;

    while (buff < buff_end) {
        if (*buff == '<') {
            if (++buff == buff_end)
                return -1;
            c = *buff;
            if (c > 64) {//element name
                svg->elt = buff;
                while (++buff < buff_end) {
                    if (*buff < 65) {
                        if (*buff == ':') {
                            svg->ns = svg->elt;
                            svg->ns_len = buff - 1 - svg->ns;
                            svg->elt = buff + 1;
                            continue;
                        }
                        break;
                    }
                }
                svg->elt_len = buff - svg->elt;
                svg->buff_ptr = buff;
                elt_lut_func(svg);
                buff = svg->buff_ptr;
                continue;
            } else if (c == '!') {
                if (++buff == buff_end)
                    return -1;
                if (*buff == '-' && ++buff < buff_end && *buff == '-') {
                    const uint8_t* commentStart = buff + 1;
                    while (++buff + 2 < buff_end) {
                        if (*buff == '-' && *(buff+1) == '-' && *(buff+2) == '>') {
                            printf("comment: ");
                            fwrite(commentStart, sizeof(uint8_t), buff - commentStart, stdout);
                            printf("\n");
                            fflush(stdout);
                            buff+=3;
                            break;
                        }
                    }
                    continue;
                } else if (buff + 7 < buff_end && !memcmp (buff, "[CDATA[", 7)) {
                    buff+=7;
                    const uint8_t* cdataStart = buff;
                    while (++buff + 2 < buff_end) {
                        if (*buff == ']' && *(buff+1) == ']' && *(buff+2) == '>') {
                            printf("cdata: ");
                            fwrite(cdataStart, sizeof(uint8_t), buff - cdataStart, stdout);
                            printf("\n");
                            fflush(stdout);
                            buff+=3;
                            break;
                        }
                    }
                    continue;
                }
            } else if (c == '/'){
                //closing tag
                if ((++buff)+elt_len < buff_end && !memcmp (buff, elt, elt_len)) {
                    svg->buff_ptr = buff + 1;
                    return 0;
                }
            } else if (c == '?') {
                continue;
            }
            perror("malformed xml\n");
            return -1;
        } else {
            buff++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.svg>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size <= 0) {
        fclose(file);
        return EXIT_SUCCESS; // Empty file
    }
    fseek(file, 0, SEEK_SET);
    uint8_t *const file_buffer = malloc((size_t)size);
    if (!file_buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }
    size_t bytes_read = fread(file_buffer, 1, (size_t)size, file);
    fclose(file);

    // 5. Check and skip UTF-8 BOM if present

    svg_context svg = {file_buffer, file_buffer + bytes_read};
    parse_children (&svg);

    free(file_buffer);
    printf("Parsing pipeline completed successfully.\n");
    return EXIT_SUCCESS;
}
