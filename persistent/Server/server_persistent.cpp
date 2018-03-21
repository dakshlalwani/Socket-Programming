#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <zconf.h>
#include <iostream>
#include <dirent.h>
#include <sys/wait.h>

#define PORT 8080

char buffer[1024] = {0}, response[1024], siz[50];;
char dirname[1024], filename[1024], tfile[1024];
int server_fd, new_socket, valread;
int i=0,j=0,k=0,l=0;
char nofile[20]="File doesn't exist";
char retype[25]="Error: Unknown Command";
char correctlen[25]="Enter the filename";
char sendpack[25]="Successfully Transferred";
long sz=0;

void send_func(char msg[])
{
    strncpy(response, msg, 1024);
    send(new_socket, response, strlen(response), 0);
}
int main(int argc, char const *argv[]) {
    // int server_fd, new_socket, valread;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);
    printf("Enter name of the folder:\n");
    scanf("%s", dirname);
    char *fileList = (char *) malloc(1000 * sizeof(char));
    FILE *file = NULL;
    DIR *d;
    struct dirent *dir;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons(PORT);    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Listening...\n");
    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                             (socklen_t *) &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    else
        printf("connection established");

    while (1) {
        valread = read(new_socket, buffer, 1024);  // read infromation received into the buffer
        printf("$ %s\n", buffer);

        if (buffer[0] == 'd') 
        {
            for (i = 0; i < 1024; ++i)
            {
                if (buffer[i] == ' ')
                {
                    while (buffer[i] == ' ')
                    {
                        i++;
                    }
                    break;
                }
            }
            for (j = i; j < 1024 && buffer[j] != 0 && buffer[j] != '\n'; j++)
            {
                tfile[j - i] = buffer[j];
            }
            strncpy(filename, dirname, 1024);
            strcat(filename, "/");
            strcat(filename, tfile);
            file = fopen(filename, "rb");
            if (file==0) 
                send_func(nofile);
            else if (strlen(tfile) == 0) 
                send_func(correctlen);
            else 
            {
                fseek(file, 0L, SEEK_END);
                sz = ftell(file);
                sz = (sz + 999) / 1000;
                sprintf(siz, "%ld", sz);
                fclose(file);
                file = fopen(filename, "rb");
                // send_func(sendpack);
                strncpy(response, "Suc: OK Sending ", 16);
                strcat(response, siz);
                strcat(response, " packets.");
                send(new_socket, response, strlen(response), 0);
                usleep(5);
                j=0;
                while(j!=sz)
                {
                    usleep(5);
                    fread(response, 1, 1000, file);
                    j++;
                    write(new_socket, response, strlen(response));
                    memset(response, 0, sizeof(response));
                }
            }
        }
        else if (buffer[0] == 'l')
        {
            d = opendir(dirname);
            if (d==0) 
            {
                perror("opendir");
                return -1;
            }
            else 
            {
                while(1)
                {
                    if((dir = readdir(d)) == 0)
                        break;
                    else
                    {
                        if(dir->d_type == 8)
                        {
                            fileList = strcat(fileList, dir->d_name);
                            fileList = strcat(fileList, "\n");
                        }
                    }
                }
                closedir(d);
            }
            send_func(fileList);
        }
        else 
            send_func(retype);  // use sendto() and recvfrom() for DGRAM
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));
        memset(tfile, 0, sizeof(tfile));
        memset(fileList, 0, sizeof(fileList));
        memset(filename, 0, sizeof(filename));
        memset(siz, 0, sizeof(siz));
    }
    return 0;
}
