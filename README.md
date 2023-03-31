## Short Description:
	This program implements the Lempel-Ziv 78 compression and decompression algorithm.
	Compressed files are decompressed with the corresponding decoder and vice versa.

## Build:
	To build/compile this program we can make use of the following commands:
	$ make
	$ make all
	
	To compile all necessary files for encode, decode:
	$ make encode
	$ make decode
	
	To delete all the files that are compiler generated use the following command:
	$ make clean
	
	To make all the files clang formatted use the following command:
	$ make format
	

## Running:
	Encode:
	    OPTIONS
           -v          Display compression statistics
           -i input    Specify input to compress (stdin by default)
           -o output   Specify output of compressed input (stdout by default)
           -h          Display program help and usage
    Decode:
        OPTIONS
           -v          Display decompression statistics
           -i input    Specify input to decompress (stdin by default)
           -o output   Specify output of decompressed input (stdout by default)
           -h          Display program usage

