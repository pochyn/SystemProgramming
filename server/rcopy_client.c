#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "ftree.h"
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <libgen.h>

#define BLOCK_SIZE 8
#ifndef PORT
  #define PORT 54748
#endif
//using constants from ex10
#define TIMES 5 // number of times to send the message
#define MINCHARS 3
#define MAXCHARS 7


void select_child(int child_soc) {
    
    int max_fd_child = child_soc;
    fd_set all_fds_child;
    FD_ZERO(&all_fds_child);
    FD_SET(child_soc, &all_fds_child);
    int nready_child = select(max_fd_child + 1, &all_fds_child, NULL, NULL, NULL);
    if (FD_ISSET(child_soc, &all_fds_child)) {
        int child_status;
        if (read(child_soc, child_status, sizeof(int)) < 0) {
            perror("read");
            exit(1);
        }
        if (child_status == OK) {
            close(child_soc);
            exit(0);
        }
        else {
            exit(1);
        }
    }
}

int send_socket(int num, int f_d) {
    int length = htonl(num);
    char *data = (char*)&length;
    int l = sizeof(conversion);
    int buff;
    buff = write(f_d, data, l);
    while (l > 0) {
        if (buff < 0) {
            if (errno != EINTR) {
                return -1;
            }
        }
        else {
            data += buff;
            l -= buff;
        }
    }
    return 0;
}


int rcopy_client(char *src, char *host, unsigned short port) {
    
    char *src_base = basename(src);
    int soc;
    struct sockaddr_in peer;

    //reuse firest part of client code from ex10
    if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("randclient: socket");
      exit(1);
    }
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, host, &server.sin_addr) < 1) {
      perror("randclient: inet_pton");
      close(soc);
      exit(1);
    }

    if (connect(soc, (struct sockaddr *)&server, sizeof(server)) == -1) {
      perror("randclient: connect");
      exit(1);
    }
    struct request req;
    struct stat st;
    stat(src, &st);
    
    //change statuses according to data send
    if (S_ISREG(stat.st_mode) {
        req.type = REGFILE;
    }
        
    else if (S_ISDIR(stat.st_mode) {
        req.type = REGDIR;
    }

    //continue using code from ex10         
    strncpy(req.path, src_base, strlen(src_base) + 1);
    req.mode = stat.st_mode;
    req.size = stat.st_size;
    FILE *file;
    file = fopen(src_base, "rb");
             
    if (file == NULL) {
        perror("read");
        exit(1);
    }
             
    char *hash_file = hash(file);
    strncpy(req.hash, hash)file, strlen(hash_file + 1);
    
    //send file to socket
    write(soc, &req.type, sizeof(int)    
    write(soc, &req.path, MAXPATH)          
    write(soc, &req.hash, BLOCK_SIZE)
    write(soc, &req.mode, sizeof(int)   
    send_socket(req.size, soc) 
       
    //using select code from lecture
    int max_fd = soc;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(soc, &all_fds);
    int nready = select(max_fd + 1, &all_fds, NULL, NULL, NULL);
             
    if (nready == -1) {
        perror("server: select");
        exit(1);
    }
             
    if (FD_ISSET(soc, &all_fds)) {
        int status;
        if (read(soc, status, sizeof(int)) < 0) {
            perror("read");
            exit(1);
        }
        
        if (status == SENDFILE) {
            int pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }
            
            if (pid == 0) {
                int child;
                struct sockaddr_in peer;
                
                //code from ex10
                if ((child = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                  perror("randclient: socket");
                  exit(1);
                }
                
                peer.sin_family = AF_INET;
                peer.sin_port = htons(PORT);
                if (inet_pton(AF_INET, host, &peer.sin_addr) < 1) {
                    perror("randclient: inet_pton");
                    close(child);
                    close(soc);
                    exit(1);
                }

                if (connect(child, (struct sockaddr *)&peer, sizeof(peer)) == -1) {
                    perror("randclient: connect");
                    close(soc);
                    exit(1);
                }
                req.type = TRANSFILE;
                int buffer = write(child, &req, sizeof(struct request));
                if (buffer < 0) {
                    exit(1);
                }
                else {
                    /* Если не сработает убери функцию
                    int max_fd_child = child_soc;
                    fd_set all_fds_child;
                    FD_ZERO(&all_fds_child);
                    FD_SET(child_soc, &all_fds_child);
                    int nready_child = select(max_fd_child + 1, &all_fds_child, NULL, NULL, NULL);
                    if (FD_ISSET(child_soc, &all_fds_child)) {
                        int child_status;
                        if (read(child_soc, child_status, sizeof(int)) < 0) {
                            perror("read");
                            exit(1);
                        }
                        if (child_status == OK) {
                            close(child_soc);
                            exit(0);
                        }
                        else {
                            exit(1);
                        }
                    }
                    */
                    select_child(child);
                }
            }
            
            else if (pid > 0) {
                int fork_status;
                if ((wait(&fork_status)) == -1) {
                    return 1;
                }
                else {
                    if (WIFEXITED(fork_status)) {
                        int exit_status = WEXITSTATUS(status);
                    }
                }
            }
        }
    }
    close(soc);
    return 0;
}


int main(int argc, char **argv) {
    /* Note: In most cases, you'll want HOST to be localhost or 127.0.0.1, so 
     * you can test on your local machine.*/
        
    char *dest;
        
    if (argc != 3) {
        printf("Usage:\n\trcopy_client SRC HOST\n");
        printf("\t SRC - The file or directory to copy to the server\n");
        printf("\t HOST - The hostname of the server");
        return 1;
    }

    if (rcopy_client(argv[1], argv[2], PORT) != 0) {
        printf("Error while copying \n");
        return 1;
    } else {
        printf("Copy successfully completed!\n");
        return 0;
    }
}

