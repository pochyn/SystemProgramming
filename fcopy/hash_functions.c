#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hash.h"
#define BLOCK_SIZE 8

char *hash(FILE *f) {
    //array to store hashed values
    char *hash_val = malloc(sizeof(char) * BLOCK_SIZE);
    for (int j = 0; j < BLOCK_SIZE; j++){
        hash_val[j] = '\0';
    }

    char buffer[1];
    int index = 0;
    //do hashing 1 byte at time
    while(fread(&buffer, 1, 1, f) == 1){
		if(index < BLOCK_SIZE){
			hash_val[index] = hash_val[index] ^ buffer[0];
			index += 1;
		}else{
			index = 0;
			hash_val[index] = hash_val[index] ^ buffer[0];
			index += 1;
		}
	}
	return hash_val;
}