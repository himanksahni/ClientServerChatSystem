#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

char *hash(FILE *f) {
	char *hash_val = malloc(sizeof(char)*9);
	int i;
	for (i=0; i<(BLOCKSIZE+1); i++){
		hash_val[i] = '\0';
	}
	i = 0;
	char c;
	while (fread(&c,sizeof(char),1,f) == 1){
		hash_val[i] = hash_val[i]^c;
		i = i + 1;
		if (i >= BLOCKSIZE){
			i = 0;
		}
        }
	return hash_val;
}

