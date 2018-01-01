#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ftree.h"

#ifndef PORT
  #define PORT 54748
#endif
#define BLOCK_SIZE 8


void status_check(int *sts, int f_d, struct request req) {
    
    if (*sts == AWAITING_TYPE) {
        if (read(f_d, &req.type, sizeof(int)) < 0) {
            perror("read");
            exit(1);
        }
        *sts = AWAITING_PATH;
    }
    
    else if (*sts == AWAITING_PATH) {
        if(read(f_d,&req.path, MAXPATH) < 0) {
            perror("read");
            exit(1);
        }
        *sts = AWAITING_PERM;
    }
    else if (*sts == AWAITING_PERM) {
        if (read(f_d,&req.mode, sizeof(int)) < 0) {
            perror("read");
            exit(1);
        }
        *sts = AWAITING_HASH;
    }
    else if (*sts == AWAITING_HASH) {
        if (read(f_d,&req.hash, BLOCK_SIZE) < 0) {
            perror("read");
            exit(1);
        }
        *sts = AWAITING_SIZE;
    }
    else if (*sts == AWAITING_SIZE) {
        if (socket_receive(&req.size, f_d) != 0) {
            perror("read");
            exit(1);
        }
        *sts = AWAITING_TYPE;
    }
    
}

void rcopy_server(unsigned short prt) {
    
    int f_d = prepare();
    struct request req;
    int max_fd = f_d;
    fd_set all_fds, listen_fds;
    
    while (1) {
        if ((accept(f_d, NULL, NULL)) < 0) {
            perror("accept");
            exit(1);
        } 
        else {
            printf("Accepted connection\n");
            int sts = AWAITING_TYPE;
            listen_fds = all_fds;
            FD_ZERO(&all_fds);
            FD_SET(f_d, &all_fds);
            int nready = select(max_fd + 1, &all_fds, NULL, NULL, NULL);
            
            if (nready == -1) {
                perror("server: select");
                exit(1);
            }
            if (FD_ISSET(f_d, &all_fds)) {
                printf("hi\n");
                /*
                if (sts == AWAITING_TYPE) {
                    if (read(f_d, &req.type, sizeof(int)) < 0) {
                        perror("read");
                        exit(1);
                    }
                    sts = AWAITING_PATH;
                }
                else if (sts == AWAITING_PATH) {
                    if(read(f_d,&req.path, MAXPATH) < 0) {
                        perror("read");
                        exit(1);
                    }
                    sts = AWAITING_PERM;
                }
                else if (sts == AWAITING_PERM) {
                    if (read(f_d,&req.mode, sizeof(int)) < 0) {
                        perror("read");
                        exit(1);
                    }
                    sts = AWAITING_HASH;
                }
                else if (sts == AWAITING_HASH) {
                    if (read(f_d,&req.hash, BLOCK_SIZE) < 0) {
                        perror("read");
                        exit(1);
                    }
                    sts = AWAITING_SIZE;
                }
                else if (sts == AWAITING_SIZE) {
                    if (socket_receive(&req.size, f_d) != 0) {
                        perror("read");
                        exit(1);
                    }
                    sts = AWAITING_TYPE;
                }
                */
                status_check(&sts, f_d, req);
            }
            printf("%s\n", req.path);
            if (req.type == REGFILE || req.type == REGDIR) {
                struct stat buffer;
                if (stat(req.path, &buffer) == 0) {
                    FILE *fs;
                    fs= fopen(req.path, "r");
                    char *hsh = hash(fs);
                    if ((S_ISREG(buffer.st_mode) && S_ISDIR(req.mode)) || (S_ISREG(buffer.st_mode) && S_ISDIR(req.mode))) {
                        if (write(f_d, ERROR, sizeof(int)) < 0) {
                            perror("write");
                            exit(1);
                        }
                    fprintf(stderr, "%s\n", "Different Files");
                    }
                    if (req.type == REGDIR) {
                        mkdir(req.path, req.mode);
                        if (write(f_d, OK, sizeof(int)) < 0) {
                            perror("write");
                            exit(1);
                        }
                    }
                    else if (req.size == buffer.st_size && check_hash(fs, req.hash) == 0) {  // check this, you should pass 2 hash arrays into param of check_hash
                        if (write(f_d, OK, sizeof(int)) < 0) {
                            perror("write");
                            exit(1);
                        }
                    }
                    else {
                        if (write(f_d, SENDFILE, sizeof(int)) < 0) {
                            perror("write");
                            exit(1);
                        }
                    }
                }
            }
            else if (req.type == TRANSFILE) {
                if (read(f_d, &req.type, sizeof(int)) < 0) {
                    perror("read");
                    exit(1);
                }
                else {
                    if (write(f_d, OK, sizeof(int)) < 0) {
                        perror("write");
                        exit(1);
                    }
                }
            }
            close(f_d);
            }
        }
}

int socket_receive(int *num, int file_descriptor) {
    
    int32_t ret;
    char *dt = (char*)&ret;
    int l = sizeof(ret);
    int rc;
    
    while (l > 0) {
        rc = read(file_descriptor, dt, l);
        if (ret <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be readable again
            }
            else if (errno != EINTR) {
                return -1;
            }
        }
        else {
            dt += rc;
            l -= rc;
        }
    }
    
    *num = ntohl(ret);
    return 0;
}
//reuse ex10
int prepare(void) {
    
    int num = 1;
    struct sockaddr_in self;
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Make sure we can reuse the port immediately after the server terminates.
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                                 &num, sizeof(num)) == -1) {
        perror("setsockopt -- REUSEADDR");
    }

    memset(&self, '\0', sizeof(self));
    self.sin_family = AF_INET;
    self.sin_addr.s_addr = INADDR_ANY;
    self.sin_port = htons(PORT);
    printf("Listening on %d\n", PORT);

    if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
        perror("bind"); // probably means port is in use
        exit(1);
    }

    if (listen(listenfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}


int main(int argc, char **argv) {
    
    if (argc != 2) {
        printf("Usage:\n\t%s rcopy_server PATH_PREFIX\n", argv[0]);
        printf("\t PATH_PREFIX - The absolute path on the server that is used as the path prefix\n");
        printf("\t        for the destination in which to copy files and directories.\n");
        exit(1);
    }
    /* NOTE:  The directory PATH_PREFIX/sandbox/dest will be the directory in
     * which the source files and directories will be copied.  It therefore 
	 * needs rwx permissions.  The directory PATH_PREFIX/sandbox will have 
	 * write and execute permissions removed to prevent clients from trying 
	 * to create files and directories above the dest directory.
     */
    
    // create the sandbox directory
    char path[MAXPATH];
    strncpy(path, argv[1], MAXPATH);
    strncat(path, "/", MAXPATH - strlen(path) + 1);
    strncat(path, "sandbox", MAXPATH - strlen(path) + 1);
    
    if (mkdir(path, 0700) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr, "couldn't open %s\n", path);
            perror("mkdir");
            exit(1);
        }
    }
    
    // create the dest directory
    strncat(path, "/", MAXPATH - strlen(path) + 1);
    strncat(path, "dest", MAXPATH - strlen(path) + 1);
    if (mkdir(path, 0700) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr, "couldn't open %s\n", path);
            perror("mkdir");
            exit(1);
        }
    }
    
    // change into the dest directory.
    //chdir(path);

    // remove write and access perissions for sandbox
    if (chmod("..", 0400) == -1) {
        perror("chmod");
        exit(1);
    }
    
    /* IMPORTANT: All path operations in rcopy_server must be relative to
     * the current working directory.
     */
    rcopy_server(PORT);

    // Should never get here!
    fprintf(stderr, "Server reached exit point.");
    return 1;
}

