CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -g -gdwarf-4

.PHONY: all clear

all: encode decode

encode: encode.o trie.o word.o io.o
	$(CC) -o $@ $^

decode: decode.o trie.o word.o io.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o encode decode

format: 
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make
