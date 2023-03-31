#include <stdio.h>
#include "code.h"
#include <stdint.h>
#include <stdlib.h>
#include "trie.h"
#include "io.h"
#include "word.h"
#define ALPHABET 256

TrieNode *trie_node_create(uint16_t index) { //creates the nodes
    TrieNode *trienode = (TrieNode *) calloc(1, sizeof(TrieNode));
    if (trienode == NULL) {
        return NULL;
    }

    for (uint16_t i = 0; i < ALPHABET; i++) {
        trienode->children[i] = NULL;
    }

    trienode->code = index;
    return trienode;
}

void trie_node_delete(TrieNode *n) {
    if (n != NULL) {
        free(n);
        n = NULL;
    }
}

TrieNode *trie_create(void) { //creates the 1st node
    return trie_node_create(EMPTY_CODE);
}

void trie_reset(TrieNode *root) { //resets trie to the root node
    if (root != NULL) {
        for (int16_t i = 0; i < ALPHABET; i++) {

            trie_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
}

void trie_delete(TrieNode *n) { //deletes parent and all children under it
    if (n != NULL) {
        for (uint16_t i = 0; i < ALPHABET; i++) {
            trie_delete(n->children[i]);
        }
    }
    trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    //returns pointer to child node representing symbol sym

    if (n->children[sym] != NULL) {
        return n->children[sym];
    } else {
        return NULL;
    }
}
