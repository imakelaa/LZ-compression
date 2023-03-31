#include <stdio.h>
#include "word.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "code.h"
#include "endian.h"
#include <sys/stat.h>
#include <assert.h>
#include "io.h"
#include "trie.h"
#include <fcntl.h>
#include <sys/types.h>

#define OPTIONS "vhi:o:"
uint32_t bit_length(uint32_t n);

int main(int argc, char **argv) {

    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    bool v = false;

    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            infile = open(optarg, O_RDONLY);
            if (infile < 0) {
                printf("File does not exist.\n");
            }
            break;
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC);
            if (outfile < 0) {
                printf("File does not exist.\n");
            }
            break;
        case 'h':
            printf("SYNOPSIS\n\tDecompresses files using the LZ78 decompression algorithm.\n\tUsed "
                   "with files decompressed with the corresponding encoder.\n\nUSAGE\n\t./decode "
                   "[-vh] [-i input] [-o output]\n\nOPTIONS\n\t-v          Display decompression "
                   "statistics\n\t-i input    Specify input to decompress (stdin by default)\n\t-o "
                   "output   Specify output of compressed input (stdout by default)\n\t-h          "
                   "Display program help\n");
            return -1;
            break;
        case 'v': v = true; break;
        }
    }
    
    FileHeader in = { 0, 0 };
    read_header(infile, &in); //verfies magic no.

    fchmod(outfile, in.protection);

    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code)) == true) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);

    
    int comp_size = total_bits / 8;
    if (total_bits % 8 != 0) {
        comp_size += 1;
    }
    int uncomp_size = total_syms;

    if (v) {
        double space_save = (100 * (1 - ((double) comp_size / uncomp_size)));

        printf("File Compressed size: %d bytes\n", comp_size);
        printf("File uncompressed size: %d bytes\n", uncomp_size);
        printf("Compression ratio: %.2f%%\n", space_save);
    }
    wt_delete(table);
    close(infile);
    close(outfile);
    return 0;
}

uint32_t bit_length(uint32_t n) {
    uint32_t len = 0;
    while (n != 0) {
        n >>= 1;
        len++;
    }
    return len;
}
