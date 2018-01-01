#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include "ftree.h"
#include "hash.h"

/*
 * This is helper function for finding last possible node with next.
 */
struct TreeNode *find_next(struct TreeNode *tree) {
    if (tree->contents != NULL) {
        struct TreeNode *curr_node = tree->contents;
        while (curr_node->next != NULL) {
            curr_node = curr_node->next;
        }
        
        return curr_node;
    }
    return NULL;
}

/*
 * Returns the FTree rooted at the path fname.
 */
struct TreeNode *generate_ftree(const char *fname) {
    
    // creating stat structure for working with file
    struct stat buffer;
    lstat(fname, &buffer);
    
    //create Node and assign info to corresponding structure
    struct TreeNode *file_tree = malloc(sizeof(struct TreeNode));
    file_tree->fname = malloc(sizeof(char) * strlen(fname) + 1);
    strcpy(file_tree->fname, fname);
    file_tree->permissions = (buffer.st_mode & 0777);
    file_tree->contents = NULL;
    file_tree->hash = NULL;
    file_tree->next = NULL;
    
    //Next we recursively generate nodes in given directory, linking them
    //Forst, working with files and links

    if (S_ISREG(buffer.st_mode) || S_ISLNK(buffer.st_mode)) {
        FILE *open_file = fopen(file_tree->fname, "r");
        if (open_file != NULL) {
            file_tree->hash = hash(open_file);
        } else {
            //to distinguish from directories
            file_tree->hash = "1";
        }
    // Working with directories
    } else if (S_ISDIR(buffer.st_mode)) {
        //open given directory to work with it
        DIR *dptr;
        if ((dptr = opendir(fname)) != NULL) {
            struct dirent *cur_fdir;
            int count = 0;
            
            //going through every object in directory
            while ((cur_fdir = readdir(dptr)) != NULL) {
                //exclude files that start fith '.'
                if (!(cur_fdir->d_name[0] == '.')){
                    //get path of given file
                    char path[1024];
                    int len = snprintf(path, (sizeof(path) - 1), "%s/%s", fname, cur_fdir->d_name);
                    path[len] = '\0';
                    
                    //if it is first file inside directory it will its cintent
                    if (count == 0) {
                        file_tree->contents = generate_ftree(path);
                    } else {
                        //all next files are next of each other starting from content
                        struct TreeNode *next_node = find_next(file_tree);
                        (*next_node).next = generate_ftree(path);
                    }
                    count++; 
                }
            }
            if (cur_fdir != NULL) {
                closedir(dptr);
            }
        }
    }
    return file_tree;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 */
void print_ftree(struct TreeNode *root) {
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");

    if (root != NULL){
        //check whether it is file, since hash is for file or links only
        if (root->hash != NULL) {
            printf("%s (%o) \n", basename(root->fname), root->permissions);
            if (root->next != NULL) {
                print_ftree(root->next);
            }
        //If no hash, then it is directory
        } else if (root->hash == NULL) {
            depth++;
            printf ("===== %s (%o) ===== \n", basename(root->fname), root->permissions);
            if (root->contents != NULL) {
                print_ftree(root->contents);
            }
            depth--;
            if (root->next != NULL) {
                print_ftree(root->next);
            }
            
        }
    }
}
