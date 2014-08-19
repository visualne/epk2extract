#include <stdio.h>

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Usage: %s [in] [out.lzhs]\n", argv[0]);
		return 1;
	}
	printf("LZHS Encoding %s => %s...\n", argv[1], argv[2]);
	lzhs_encode(argv[1], argv[2]);
	return 0;
}