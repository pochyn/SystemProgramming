#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include "hash.h"


int copy_ftree(const char *source, const char *dest) {
    int count = 1;
    int status;
    pid_t pid = 1;
    char name[1024];
    struct stat original;
    if(lstat(source, &original) == -1)
        return -1; //can not continue pragram since path is not valid
    
    //Folowing chunk of code is our base case for recursion
    // if the item is a file
    if(S_ISREG(original.st_mode)) {
        //copy to dest
        struct stat destination;
        char *filename;
        char pathname[1024];
        filename = basename((char *)source);
        sprintf(pathname, "%s/%s", dest, filename);
        
        //check wether the same file
        int checker = 0;
        if (lstat(pathname, &destination) != -1 ) {
            if ((S_ISREG(original.st_mode) && S_ISDIR(destination.st_mode)) || (S_ISDIR(original.st_mode) && S_ISREG(destination.st_mode))) {
                perror("Same name of file and dir, can not copy.\n");
		        exit(0);
            }
            //there exists file with same name, check size
            if (original.st_size == destination.st_size) {
                // same size, check hash
                char *sourcehash;
                char *destinationhash;
                FILE *sourcefile;
                FILE *destinationfile;
                if((sourcefile = fopen(source,"rb")) == NULL) {
		            perror("Open read file error.\n");
		            exit(0);
                }
                if((destinationfile = fopen(pathname,"rb")) == NULL) {
		            perror("Open read file error.\n");
		            exit(0);
                }
                sourcehash = hash(sourcefile);
                destinationhash = hash(destinationfile);
                fclose(sourcefile);
	            fclose(destinationfile);
                if(strcmp(sourcehash, destinationhash) == 0) {
                    //same file
                    checker = 1;
                }
            }
        }
        
        //ready to copy file to new destination
        if (checker != 1) {
            FILE *fileread, *filewrite;
	        int numr,numw;
	        char buffer[100];

	    if((fileread = fopen(source,"rb")) == NULL) {
		   perror("Open read file error.\n");
		   exit(0);
	    }

	    if((filewrite = fopen(pathname,"wb")) == NULL) {
		   perror("Open write file error.\n");
		   exit(0);
	    }
            
	    while(feof(fileread) == 0){	
	        if((numr = fread(buffer, 1, 100, fileread)) != 100) {
		        if(ferror(fileread)!=0){
		            perror("read file error.\n");
		            exit(0);
		         } else if (feof(fileread) != 0) {
                    //do nothing
                 }
                 
                 
	         }
	        if((numw = fwrite(buffer, 1, numr, filewrite)) != numr) {
		        perror("Write file error.\n");
		        exit(0);
	         }
	    }	
	
	    fclose(fileread);
	    fclose(filewrite);
        }
        
        //set permisions to file in new dest
        mode_t per = original.st_mode;
        chmod(pathname, per);
    }
    
    
    //recursive call, when path is directory
    else if(S_ISDIR(original.st_mode)){
        pid = fork();
        
        //parent process, wait
        if(pid > 0){ 
            if(waitpid(pid, &status, 0) == -1) {
                perror("Failed to wait \n");
                exit(0);
            } else {
                if(WIFEXITED(status)) {
                    count += WEXITSTATUS(status);
                }
            }
       }
        
       //child process
       else if(pid == 0){
           //create new directory in destionation, if there doesn't exist one
           mode_t per = original.st_mode;
           char *filename;
           char pathname[1024];
           filename = basename((char *)source);
           sprintf(pathname, "%s/%s", dest, filename);
           struct stat st;
           if (lstat(pathname, &st) == -1) {
               mkdir(pathname, per);
           } else {
               if ((S_ISREG(original.st_mode) && S_ISDIR(st.st_mode)) || (S_ISDIR(original.st_mode) && S_ISREG(st.st_mode))) {
                   perror("Same name of file and dir, can not copy.\n");
		           exit(0);
               }
           }
           
           //read all files in directory, call recursion
           DIR *dptr;
           struct dirent *cur_fdir = NULL;
           if ((dptr = opendir(source)) == NULL){
               perror("Cannot open\n");
               exit(0);
           }
           else{
               while((cur_fdir = readdir(dptr)) != NULL){
                   if(!(cur_fdir->d_name[0] == '.')) {
                        sprintf(name, "%s/%s", source, cur_fdir->d_name);
                        count = copy_ftree(name, pathname);
                   }
               }
               closedir(dptr);
               exit(count);
           }
       }
       
       // fork failed
       else{
           perror("Failed to fork\n");
           exit(0);
       }
    }
    return count;
}

    


        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
