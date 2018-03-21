#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <zconf.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 8080

char buffer[1024] = {0}, filename[1024], tfile[1024],tmpcmd[1024], siz[50];
int i=0,j=0,k=0,l=0,ct=0,sz=0;


int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0;
    ssize_t valread;
    struct sockaddr_in serv_addr;
    // char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
    // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    struct stat st = {0};

    if (stat("downloaded_files", &st) == -1) {
        mkdir("downloaded_files", 0700);
    }

    printf("Commands\n--------\n`l` for ls\n`d <filename>` to download the file\n");

    char *command = (char *) malloc(1000 * sizeof(char));
    FILE *file = NULL;
    size_t commandSize = 1000;


    while (1)
    {
        int counti=0;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }
        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
        {
            printf("\nConnection Failed \n");
            return -1;
        }
        printf("--> ");
        getline(&command, &commandSize, stdin);
        printf("\n");
        command[strlen(command) - 1] = 0;
        // if (command[0] == 'e')
        // {
        //     send(sock, command, strlen(command), 0);  // send the message.
        //     break;
        // }
        if (command[0] == 'd') {
            for (i = 0; i < 1024; ++i)
            {
                if (command[i] == ' ')
                {
                    while (i < strlen(command))
                    {
                        strncpy(tmpcmd, "download ", 9);
                        while (i < strlen(command) && command[i] == ' ')
                        {
                            i++;
                        }
                        for (j = i; command[j] != 0 && command[j] != ' '; j++) 
                        {
                            if(j < strlen(command))
                                tfile[j - i] = command[j];
                        }
                        
                        strcat(tmpcmd, tfile);
                        if(counti)
                        {
                            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                            {
                                printf("\n Socket creation error \n");
                                return -1;
                            }
                            if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
                            {
                                printf("\nConnection Failed \n");
                                return -1;
                            }
                        }
                        send(sock, tmpcmd, strlen(tmpcmd), 0);  // send the message.
                        valread = read(sock, buffer, 1024);  // receive message back from server, into the buffer
                        printf("%s\n", buffer);
                        if (buffer[0] == 'S' && buffer[1] == 'u')
                        {
                            strncpy(filename, "downloaded_files/", 17);
                            strcat(filename, tfile);
                            file = fopen(filename, "w");
                            // checkbuf();
                            while (l < 1024 && (buffer[l] < '0' || buffer[l] > '9')) {
                                l++;
                            }
                            while (buffer[j] >= '0' && buffer[j] <= '9') {
                                siz[ct++] = buffer[j++];
                            }
                            sz = atoi(siz);
                            memset(buffer, 0, 1024);
                            for (int k = 1; k <= sz; ++k) {
                                valread = read(sock, buffer, 1000);
                                fprintf(file, buffer);
                                memset(buffer, 0, 1024);
                            }
                            fclose(file);
                            printf("Download Complete.\n");
                        }
                        printf("\n");
                        i = j;
                        if(counti==0)
                            counti=1;
                        memset(tfile, 0, sizeof(tfile));
                        memset(tmpcmd, 0, sizeof(tmpcmd));
                        memset(buffer, 0, sizeof(buffer));
                        memset(filename, 0, sizeof(filename));
                        memset(siz, 0, 50);
                    }
                    break;
                }
            }
        } else {
            send(sock, command, strlen(command), 0);  // send the message.
            valread = read(sock, buffer, 1024);  // receive message back from server, into the buffer
            printf("%s\n", buffer);
        }
        memset(buffer, 0, sizeof(buffer));
    }
    return 0;
}
