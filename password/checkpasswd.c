/* this solution needs error checking! */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Read a user id and password from standard input, 
   - create a new process to run the validate program
   - send 'validate' the user id and password on a pipe, 
   - print a message 
        "Password verified" if the user id and password matched, 
        "Invalid password", or 
        "No such user"
     depending on the return value of 'validate'.
*/

/* Use the exact messages defined below in your program." */

#define VERIFIED "Password verified\n"
#define BAD_USER "No such user\n"
#define BAD_PASSWORD "Invalid password\n"
#define OTHER "Error validating password\n"



int main(void) {
    char userid[10];
    char password[10];

    /* Read a user id and password from stdin */
    printf("User id:\n");
    scanf("%s", userid);
    printf("Password:\n");
    scanf("%s", password);
    
    int fd[2];
    int status;
    pipe(fd);
    int child_id = fork();
    
    if (child_id > 0) {
        close(fd[0]);
        write(fd[1], userid, 10);
        write(fd[1], password, 10);
        if (wait(&status) != -1) {
            printf("%s", userid);
            
            if (WIFEXITED(status)) {
                switch (WEXITSTATUS(status)) {
                    case 0: 
                        printf("%s", VERIFIED);
                        break;
                    case 1:  
                        printf("%s", OTHER);
                        break;
                    case 2:
                        printf("%s", BAD_PASSWORD);
                        break;
                    case 3:
                        printf("%s", BAD_USER);
                        break;
                    default:
                        break;
                }
            }
        } else {
            perror("Error waiting for child \n");
            exit(1);
        }
        close(fd[1]);
        
    } else if (child_id == 0) {
        close(fd[1]);
        dup2(fd[0], fileno(stdin));
        close(fd[0]);
        execlp("./validate", "validate", NULL);
    }
    
    return 0;
}
