#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "code.h"
#include "word.h"

Word *word_create(uint8_t *syms, uint32_t len) {
    Word *mem_size_word = malloc(sizeof(Word)); //allocate memory for size of word

    if (mem_size_word != NULL) {
        mem_size_word->len = len;
        mem_size_word->syms = calloc(len, sizeof(uint8_t));

        for (uint32_t i = 0; i < len; i++) { //looping through and copying sym
            mem_size_word->syms[i] = syms[i];
        }
    }

    return mem_size_word;
}

Word *word_append_sym(Word *w, uint8_t sym) { //appends to an existing word

    Word *new_word = malloc(sizeof(Word)); //allocate memory for size of word

    if (new_word != NULL) {
        new_word->len = w->len + 1;
        new_word->syms = calloc(w->len + 1, sizeof(uint8_t));

        //loop thru sym
        for (uint32_t i = 0; i < w->len; i++) {
            new_word->syms[i] = w->syms[i];
        }
        new_word->syms[w->len] = sym;

        return new_word;

    } //if close
    return NULL;
}

void word_delete(Word *w) {
    if (w != NULL) {
        free(w->syms);
        free(w);
    }
}

WordTable *wt_create(void) {
    WordTable *new_WT = (WordTable *) calloc(MAX_CODE, sizeof(WordTable));
    if (new_WT == NULL) {
        return NULL;
    }
    new_WT[EMPTY_CODE] = word_create(NULL, 0);
    return new_WT;
}

void wt_reset(WordTable *wt) {
    for (int i = START_CODE; i < MAX_CODE; i++) {
        if (i != EMPTY_CODE) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
}

void wt_delete(WordTable *wt) {
    if (wt != NULL) {
        for (int i = 0; i < MAX_CODE; i++) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }

        free(wt);
    }
}
