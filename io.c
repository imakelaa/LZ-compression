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

uint64_t total_syms = 0; // To count the symbols processed.
uint64_t total_bits = 0; // To count the bits processed.

static uint8_t buff[BLOCK] = { 0 }; //for read_pair, write_pair, flush_pairs
static int index = 0; //tracks bit not byte

static uint8_t buff2[BLOCK] = { 0 }; //for read_sym, write_word, flush_words
static int index2 = 0;

static int bytes_read = 0; //for read_pair

int read_bytes(int infile, uint8_t *buf, int to_read) {
    uint32_t bytes_read = 0; //keeping track of bytes read so far
    int latest_call = 0; //keep track of bytes read on the latest read call = 0
    if (to_read == 0) {
        return 0;
    }

    while ((latest_call = read(infile, buf + bytes_read, to_read))) {
        //check if latest_call is -1 or 0(EOF) -> return total bytes;
        if (latest_call == -1 || latest_call == 0) {
            return bytes_read;
        }

        bytes_read += latest_call; //total bytes read so far += bytes read on latest read call

        to_read -= latest_call;

        if (to_read == 0) {
            return bytes_read; //stop reading once "to_read" bytes is read
        }
    }
    //return total no. of bytes read
    return bytes_read;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    if (to_write == 0) {
        return 0;
    }
    uint32_t bytes_written = 0; //keep track of bytes written so far = 0
    int32_t latest_write = 0; //keep track of bytes written on latest read call = 0

    while ((latest_write = write(outfile, buf + bytes_written, to_write))) {
        //if latest_write == -1-- then handle error
        if (latest_write == -1) {
            return bytes_read;
        }
        bytes_written += latest_write; //total bytes written so far += latest_write
        to_write -= latest_write;

        if (to_write == 0) {
            return bytes_written;
        }
    }
    return bytes_written;
}

void read_header(int infile, FileHeader *header) {
    //reads sizeof(FileHeader) from input file
    //bytes are read into the supplied header
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));

    //endian-ness is swapped if byte order isn't little endian
    if (!(little_endian())) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    assert(header->magic == MAGIC); //verify the magic number
    total_bits += sizeof(FileHeader) * 8;
}

void write_header(int outfile, FileHeader *header) {
    //write sizeof(FileHeader) to outputfile

    if (!(little_endian())) { //endianness is swapped if byte order isn't little endian
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    total_bits += sizeof(FileHeader) * 8;
}

//store the index of the buffer
bool read_sym(int infile, uint8_t *sym) {
    //count of which bit ur on & length as we move thru
    static int bytes = 0;

    if (index2 == 0) { //if index is at 0, we read
        bytes = read_bytes(infile, buff2, BLOCK); //how many bytes there are
    }
    *sym = buff2[index2];
    index2++; //to move thru what bit ur on

    if (bytes == index2) { // finished reading block
        index2 = 0;
    }

    total_syms += 1;
    return index2 != bytes + 1;
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    //writes pair to outfile

    //code
    for (int i = 0; i < bitlen; i++) {
        //grab position at i
        uint32_t bit = code >> (i % 16) & (uint16_t) 1;

        if (bit == 1) { //-> set

            //set bit at next available index in buffer
            buff[index / 8] |= (1 << (index % 8)); //buffer is 8 bit buffer
        }

        if (bit == 0) { //-> clear
            buff[index / 8] &= ~(0x1 << index % 8);
        }

        //if buff index is at end
        index++;
        if (index == BLOCK * 8) {
            //write out buffer
            write_bytes(outfile, buff, BLOCK);
            //reset index and buffer
            index = 0;
        }
    }

    //sym
    for (int i = 0; i < 8; i++) {
        //grab position at i
        uint32_t bit = sym >> (i % 8) & (uint16_t) 1;

        if (bit == 1) { // -> set

            //set bit at next available index in buffer
            buff[index / 8] |= (1 << (index % 8));
        }

        if (bit == 0) { // -> clear
            buff[index / 8] &= ~(0x1 << index % 8);
        }

        //if buff index is at end
        index++;
        if (index == BLOCK * 8) {
            //write out buffer
            write_bytes(outfile, buff, BLOCK);
            //reset index and buffer
            index = 0;
        }
    }
    total_bits += bitlen + 8;
}

void flush_pairs(int outfile) {

    write_bytes(outfile, buff, index % 8 == 0 ? index / 8 : (index / 8) + 1);

    index = 0; //reset index
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) { //decode

    *code = 0;
    *sym = 0;

    //bit length is length of everything in file and not length of buffer
    for (int i = 0; i < bitlen; i++) {
        //at end, so then we proceed to populate
        if (index == bytes_read * 8) {
            //populating buff
            bytes_read = read_bytes(infile, buff, BLOCK);
            // bit index = (bit index +1)%(BLOCK*8) this is wrapping around, empty and full are the same

            index = 0;
        }

        uint8_t get_bit_code = (buff[index / 8] >> (index % 8)) & 0x1;

        if (get_bit_code == 1) { //-> set

            //set bit at next available index in buffer
            *code |= (1 << i); //nishant
        }

        if (get_bit_code == 0) { //-> clear
            *code &= ~(1 << i);
        }

        index++;
    }

    for (int i = 0; i < 8; i++) {
        //at end, so then we proceed to populate
        if (index == bytes_read * 8) {
            //populate
            bytes_read = read_bytes(infile, buff, BLOCK);
            index = 0;
        }

        uint8_t get_bit_sym = (buff[index / 8] >> (index % 8)) & 0x1;

        if (get_bit_sym == 1) { //-> set

            //set bit at next available index in buffer
            *sym |= (1 << i);
        }

        if (get_bit_sym == 0) { //-> clear
            *sym &= ~(1 << i);
        }

        index++;
    } //for loop close

    total_bits += bitlen + 8;

    return *code == STOP_CODE ? false : true;
}

void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        if (index2 == BLOCK) {
            write_bytes(outfile, buff2, BLOCK);
            index2 = 0;
        }
        buff2[index2] = w->syms[i];
        index2++;
    }
    total_syms += w->len;
}

void flush_words(int outfile) {
    if (index2 != 0) {
        write_bytes(outfile, buff2, index2);
        index2 = 0;
    }
}
