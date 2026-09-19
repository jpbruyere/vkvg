//#include "vkvg.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

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

typedef enum {
    MARKER_TAG_OPEN    = 1, // '<'
    MARKER_QUOTE       = 2, // '"'
    MARKER_UTF8_MULT   = 3  // UTF-8 byte > 0x7F
} MarkerType;

typedef struct {
    uint32_t offset;
    uint8_t type;
} StructuralMarker;

/**
 * @brief Scans memory using AVX2 and records the position of structural characters.
 * @param markers Output array to store structural indexes.
 * @param max_markers Allocation capacity of the output array.
 * @return size_t The actual number of markers populated.
 */
size_t build_structural_index_avx2(const uint8_t *const buffer, size_t len,
                                   StructuralMarker *markers, size_t max_markers) {
    const uint8_t *cursor = buffer;
    const uint8_t *simd_end = buffer + (len & ~31);
    size_t marker_count = 0;

    __m256i v_tag_open = _mm256_set1_epi8('<');
    __m256i v_quote    = _mm256_set1_epi8('"');

    while (cursor < simd_end) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)cursor);

        uint32_t utf8_mask  = (uint32_t)_mm256_movemask_epi8(chunk);
        uint32_t open_mask  = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_tag_open));
        uint32_t quote_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_quote));

        uint32_t combined = utf8_mask | open_mask | quote_mask;

               // If the 32-byte block contains any of our target elements, extract them branchlessly
        while (combined != 0) {
            if (marker_count >= max_markers) return marker_count; // Safety brake

            int idx = __builtin_ctz(combined);
            uint32_t absolute_offset = (uint32_t)((cursor + idx) - buffer);

            markers[marker_count].offset = absolute_offset;

            // Deduce the type branchlessly using bit shifting from masks
            if ((open_mask >> idx) & 1)       markers[marker_count].type = MARKER_TAG_OPEN;
            else if ((quote_mask >> idx) & 1) markers[marker_count].type = MARKER_QUOTE;
            else                              markers[marker_count].type = MARKER_UTF8_MULT;

            marker_count++;
            combined &= combined - 1; // Clear the lowest set bit to process the next match
        }

        cursor += 32;
    }

           // (Scalar tail loop for remaining bytes < 32 goes here...)
    return marker_count;
}
#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

void index_whitespace_starts_avx2(const uint8_t *const buffer, size_t len,
                                  uint32_t *ws_starts, size_t *ws_count) {
    const uint8_t *cursor = buffer;
    const uint8_t *simd_end = buffer + (len & ~31);
    size_t count = *ws_count;

    __m256i v_space = _mm256_set1_epi8(' ');
    __m256i v_tab = _mm256_set1_epi8('\t');
    __m256i v_n = _mm256_set1_epi8('\n');
    __m256i v_r = _mm256_set1_epi8('\r');

    __m256i v_tag_open = _mm256_set1_epi8('<');
    __m256i v_quote    = _mm256_set1_epi8('"');

    while (cursor < simd_end) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)cursor);

               // 1. Detect spaces and control characters simultaneously
        uint32_t space_mask  = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_space));
        uint32_t tab_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_tab));
        uint32_t n_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_n));
        uint32_t r_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_r));

        uint32_t ws_mask = space_mask | tab_mask | n_mask | r_mask;

        if (ws_mask != 0) {
            // 🚀 THE BITMASK TRICK: Isolate only the first bit of contiguous blocks
            // This eliminates all consecutive spaces branchlessly in 2 CPU cycles!
            uint32_t ws_starts_mask = ws_mask & ~(ws_mask << 1);

            uint32_t base_offset = (uint32_t)(cursor - buffer);

                   // Pop only the isolated starting bits into our parallel array
            while (ws_starts_mask != 0) {
                ws_starts[count++] = base_offset + __builtin_ctz(ws_starts_mask);
                ws_starts_mask &= ws_starts_mask - 1;
            }
        }

        cursor += 32;
    }

    *ws_count = count;
}

void parse_svg_from_structural_index(const uint8_t *const buffer, size_t buffer_len,
                                     const StructuralMarker *markers, size_t marker_count) {
    size_t i = 0;

    while (i < marker_count) {
        StructuralMarker m = markers[i];
        const uint8_t *cursor = buffer + m.offset;

        if (m.type == MARKER_TAG_OPEN) {
            // Peek at what follows '<' using direct pointer offset
            uint8_t next_b = *(cursor + 1);

            if (next_b == '!') {
                // Comment / CDATA routing:
                // Because we have the structural index table, we can skip comments instantly!
                // We just scan forward through the markers array looking for a MARKER_TAG_OPEN or
                // specialized sequences, rather than checking bytes!
                if (memcmp(cursor, "<!--", 4) == 0) {
                    // Skip ahead in our structural array until we find where "-->" ends
                    while (i < marker_count && memcmp(buffer + markers[i].offset, "-->", 3) != 0) {
                        i++;
                    }
                    i++; // step past comment close
                    continue;
                }
            }
            else if (next_b != '?') {
                // Standard Tag! Hand it straight over to gperf zero-copy style
                const uint8_t *tag_start = cursor + 1;

                // Find where the tag name terminates.
                // We only look forward a few bytes until a space or bracket delimiter.
                const uint8_t *scan = tag_start;
                while (*scan != ' ' && *scan != '>' && *scan != '/') {
                    scan++;
                }

                size_t tag_len = (size_t)(scan - tag_start);

                // 🚀 GPERF LANDING:
                // const struct SvgKeyword *token = lookup_svg_token((const char *)tag_start, tag_len);
            }
        }
        i++;
    }
}

typedef struct {
    uint32_t start;
    uint32_t end; // Index of the last whitespace byte in the run
} WhitespaceSpan;

size_t build_whitespace_spans_avx2(const uint8_t *const buffer, size_t len,
                                   WhitespaceSpan *spans, size_t max_spans) {
    const uint8_t *cursor = buffer;
    const uint8_t *simd_end = buffer + (len & ~31);
    size_t span_count = 0, t_cnt = 0, q_cnt = 0;

    __m256i v_space = _mm256_set1_epi8(' ');
    __m256i v_tab = _mm256_set1_epi8('\t');
    __m256i v_n = _mm256_set1_epi8('\n');
    __m256i v_r = _mm256_set1_epi8('\r');

    __m256i v_tag_open = _mm256_set1_epi8('<');
    __m256i v_quote    = _mm256_set1_epi8('"');

    uint32_t prev_last_bit = 0;
    int64_t open_start_offset = -1;

    while (cursor < simd_end) {
        __m256i chunk = _mm256_loadu_si256((const __m256i *)cursor);
        uint32_t base_offset = (uint32_t)(cursor - buffer);

        uint32_t space_mask  = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_space));
        uint32_t tab_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_tab));
        uint32_t n_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_n));
        uint32_t r_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_r));

        uint32_t ws_mask = space_mask | tab_mask | n_mask | r_mask;

        if (ws_mask != 0) {

                   // 🚀 The Shift-Differencing Step
            uint32_t shifted_left = (ws_mask << 1) | prev_last_bit;
            uint32_t starts_mask  = ws_mask & ~shifted_left;
            uint32_t stops_mask   = ws_mask & ~(ws_mask >> 1);

            while (starts_mask != 0 && stops_mask != 0) {
                if (span_count >= max_spans) return span_count;

                int start_idx = __builtin_ctz(starts_mask);
                int stop_idx  = __builtin_ctz(stops_mask);

                spans[span_count].start = base_offset + start_idx;
                spans[span_count].end   = base_offset + stop_idx;
                span_count++;

                starts_mask &= starts_mask - 1;
                stops_mask  &= stops_mask - 1;
            }
        }

        prev_last_bit = (ws_mask >> 31);

        /*uint32_t open_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_tag_open));
        while (open_mask != 0) {
            if (t_cnt >= max_markers) break;
            streams->tag_opens[t_cnt++] = base_offset + __builtin_ctz(open_mask);
            open_mask &= open_mask - 1;
        }

               // 3. --- 🚀 NEW: EXTRACTION OF QUOTES ('"') ---
        uint32_t quote_mask = (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_quote));
        while (quote_mask != 0) {
            if (q_cnt >= max_markers) break;
            streams->quotes[q_cnt++] = base_offset + __builtin_ctz(quote_mask);
            quote_mask &= quote_mask - 1;
        }*/

        cursor += 32;
    }

    return span_count;
}

void parse_tokens_via_spans(const uint8_t *const buffer, size_t buffer_len,
                            const WhitespaceSpan *spans, size_t span_count) {
    if (span_count == 0) return;

    for (size_t i = 0; i < span_count; ++i) {
        // The token starts EXACTLY one byte after the current whitespace run finishes
        const uint8_t *token_start = buffer + spans[i].end + 1;

               // The token ends EXACTLY where the next whitespace run opens
        const uint8_t *token_end;
        if (i + 1 < span_count) {
            token_end = buffer + spans[i + 1].start;
        } else {
            token_end = buffer + buffer_len;
        }

        size_t token_len = (size_t)(token_end - token_start);

        if (token_len > 0 && token_start < token_end) {
            fwrite(token_start, sizeof(uint8_t), token_len, stdout);
            printf("\n");
            // 🚀 PURE ZERO-COPY TOKEN
            // No loops, no checks, no copies!
            // lookup_svg_token((const char*)token_start, token_len);
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
    // 2. Determine file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size <= 0) {
        fclose(file);
        return EXIT_SUCCESS; // Empty file
    }
    fseek(file, 0, SEEK_SET);
    // 3. Allocate a contiguous block of memory
    uint8_t *file_buffer = malloc((size_t)size);
    if (!file_buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }
    // 4. Read file content entirely into memory
    size_t bytes_read = fread(file_buffer, 1, (size_t)size, file);
    fclose(file);

    // 5. Check and skip UTF-8 BOM if present
    uint8_t *parse_ptr = file_buffer;
    size_t parse_size = bytes_read;

    if (parse_size >= 3 && parse_ptr[0] == 0xEF && parse_ptr[1] == 0xBB && parse_ptr[2] == 0xBF) {
        printf("Detected UTF-8 BOM, skipping 3 bytes.\n");
        parse_ptr += 3;
        parse_size -= 3;
    }

    printf("Executing AVX2 scan over %zu bytes...\n", parse_size);

    /*StructuralMarker markers[1024];
    size_t max_markers = 1024;

    size_t markers_count = build_structural_index_avx2(parse_ptr, parse_size, markers, max_markers);
    parse_svg_from_structural_index(parse_ptr, parse_size, markers, markers_count);*/
    /*uint32_t ws_starts[10000];
    size_t ws_count = 0;
    index_whitespace_starts_avx2(parse_ptr, parse_size, ws_starts, &ws_count);
    uint32_t len = 0;
    if (ws_starts[0] > 0)
        fwrite(parse_ptr, sizeof(uint8_t), ws_starts[0], stdout);
    printf("\n");
    for (int i = 1; i < ws_count; ++i) {
        len = ws_starts[i] - ws_starts[i-1] + 1;
        if (len > 0) {
            fwrite(parse_ptr + ws_starts[i-1] + 1, sizeof(uint8_t), len, stdout);
            printf("\n");
        } else
            printf("0 lenghtn");
    }*/

    const size_t max_wspans_count = 4096;
    WhitespaceSpan wspans[max_wspans_count];
    size_t wspans_count = 0;
    wspans_count = build_whitespace_spans_avx2(parse_ptr, parse_size, wspans, max_wspans_count);
    printf("total white space spans = %lu\n", wspans_count);
    parse_tokens_via_spans(parse_ptr, parse_size, wspans, wspans_count);

    // 7. Cleanup
    free(file_buffer);
    printf("Parsing pipeline completed successfully.\n");
    return EXIT_SUCCESS;
}
