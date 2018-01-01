#include <stdio.h>
#include <math.h>

// Complete these two functions according to the assignment specifications


void hash(char *hash_val, long block_size) {
    for (int j = 0; j < block_size; j++){
        hash_val[j] = '\0';
    }
    
    char character;
    int i = 0;
    while(scanf("%c", &character) != EOF){
       if (i < block_size){
           hash_val[i] ^= character;
           i++;
       } else {
           i = 0;
           hash_val[i] ^= character;
           i++;
       }
    }
}


int check_hash(const char *hash1, const char *hash2, long block_size) {
    // A place holder so that the file will compile without error.
	// Replace with the correct return value.
    for (int i = 0; i < block_size; i++){
        if (hash1[i] == hash2[i]){
            //do nothing
        } else {
            return i;
        }
    }	
    return (int)block_size;
    
}