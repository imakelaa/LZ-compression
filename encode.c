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

#define OPTIONS "vhi:o:"

uint32_t bit_length(uint32_t n);

int main(int argc, char **argv) {

    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    bool v = false;

    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
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
            outfile = open(optarg, O_WRONLY | O_CREAT);
            if (outfile < 0) {
                printf("File does not exist.\n");
            }
            break;
        case 'h':
            printf("SYNOPSIS\n\tCompresses files using the LZ78 compression "
                   "algorithm.\n\tCompressed files are decompressed with the corresponding "
                   "decoder.\n\nUSAGE\n\t./encode [-vh] [-i input] [-o output]\n\nOPTIONS\n\t-v    "
                   "      Display compression statistics\n\t-i input    Specify input to compress "
                   "(stdin by default)\n\t-o output   Specify output of compressed input (stdout "
                   "by default)\n\t-h          Display program help and usage\n");
            return -1;
            break;
        case 'v': v = true; break;
        }
    }
    
    struct stat stats;
    fstat(infile, &stats);

    FileHeader out = { 0 };
    out.magic = MAGIC;
    out.protection = stats.st_mode;

    fchmod(outfile, stats.st_mode);
    write_header(outfile, &out);

    TrieNode *next_node;

    while (read_sym(infile, &curr_sym) == true) {
        next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node; //declare next node
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile);

    int comp_size = total_bits / 8;
    if (total_bits % 8 != 0) {
        comp_size += 1;
    }

    int uncomp_size = total_syms - 1;

    if (v) {
        double space_save = (100 * (1 - ((double) comp_size / uncomp_size)));

        printf("File Compressed size: %d bytes\n", comp_size);
        printf("File uncompressed size: %d bytes\n", uncomp_size);
        printf("Compression ratio: %.2f%%\n", space_save);
    }

    trie_delete(root);
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
