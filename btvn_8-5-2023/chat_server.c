#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdbool.h>
#include <time.h>

#define MAX_USER 64

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("Create socket failed: ");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);;
    addr.sin_port = htons(9000);

    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Binding Failed\n");
        exit(1);
    }

    if (listen(serverSocket, 5) == -1) {
        printf("Listening Failed\n");
        exit(1);
    }
    printf("Waiting for client connecting ...\n");

    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);

    time_t t;
    t = time(NULL);
    struct tm tm = *localtime(&t);
    

    struct pollfd fds[MAX_USER];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;
    int nfds = 1;

    int users_socket[MAX_USER];
    char* users_id[MAX_USER];
    int userNumber = 0;

    char buff[256];
    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            printf("Poll Failed\n");
            break;
        }

        if (fds[0].revents & POLLIN)  {
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);

            if (userNumber < MAX_USER) {
                printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                

                fds[nfds].fd = clientSocket;
                fds[nfds].events = POLLIN;
                nfds++;


                char *message = "Please sign in to continue (client_id: client_name)\n";
                send(clientSocket, message, strlen(message), 0);
            }
            else {
                printf("Exceed maxmimum connection\n");
                char *message = "Exceed maxmimum connection\n";
                send(clientSocket, message, strlen(message), 0);
                close(clientSocket);
            }
        }

        for (int i = 1; i < nfds; ++i) {
            if (fds[i].revents & (POLLIN | POLLERR)) {
                int client = fds[i].fd;
                ret = recv(client, buff, sizeof(buff), 0);
                buff[ret] = 0;
                if (buff[ret - 1] == '\n')
                    buff[ret - 1] = 0;
                printf("Receive from %d: %s\n", client, buff);

                if (ret <= 0) {
                    int j = 0;
                    for (; j < userNumber; ++j) {
                        if (users_socket[j] == client)
                            break;
                    }
                    users_socket[j] = users_socket[userNumber - 1];
                    users_id[j] = users_id[userNumber - 1];
                    userNumber--;

                    fds[i] = fds[nfds - 1];
                    nfds--;

                    close(client);

                    printf("Client %d has disconnected\n", client);
                    i--;
                    continue;
                }

                int j = 0;
                for (j; j < userNumber; ++j) {
                    if (users_socket[j] == client)
                        break;
                }

                if (j == userNumber) {

                    char cmd[32], id[32], tmp[32];
                    ret = sscanf(buff, "%s%s%s", cmd, id, tmp);
                    
                    if (ret == 2) {
  
                        if (strcmp("client_id:", cmd) == 0) {
                            bool flag = false;
                            for (int tmp = 0; tmp < userNumber; ++tmp) {
                                if (strcmp(users_id[tmp], id) == 0) {

                                    char *message = "Client id has already existed\n";
                                    send(client, message, strlen(message), 0);
                                    flag = true;
                                    break;
                                }
                            }
                            if (flag) continue;

                            users_socket[userNumber] = client;
                            users_id[userNumber] = malloc(strlen(id) + 1);
                            strcpy(users_id[userNumber], id);
                            userNumber++;
                            char *message = "Login successfully (format to chat private: PRIVATE_TO_clientid message)\n";
                            send(client, message, strlen(message), 0);
                            continue;
                        }
                    }

                    char *message = "Wrong format\n";
                    send(client, message, strlen(message), 0);
                }
                else {

                    if (strncmp("PRIVATE_TO_", buff, 11) == 0)
                    {

                        char* tmp = strchr(buff, ' ');
                        if (tmp == NULL) {

                            char *message = "Wrong format to private chat (PRIVATE_TO_clientid message)\n";
                            send(client, message, strlen(message), 0);
                            continue;
                        }

                        int size = tmp - buff - 11;                        
                        char id[32];
                        memcpy(id, buff + 11, size);
                        id[size] = 0;

                        int des = 0;
                        for (des; des < userNumber; ++des) {
                            if (strcmp(users_id[des], id) == 0)
                                break;    
                        }

                        if (des == userNumber) {
                            // Can't find client_id
                            char *message = "Client id do not exist\n";
                            send(client, message, strlen(message), 0);
                            continue;
                        }

                        char message[512];
                        strcpy(message, users_id[j]);
                        strcat(message, ":");
                        strcat(message, tmp);
                        strcat(message, "\n");
                        send(users_socket[des], message, strlen(message), 0);
                    }
                    else {
                        // boardcast message
                        char message[256];
                        sprintf(message, "%04d/%02d/%02d %02d:%02d:%02d ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
                        tm.tm_hour, tm.tm_min, tm.tm_sec);
                        strcat(message, users_id[j]);
                        strcat(message, ": ");
                        strcat(message, buff);
                        strcat(message, "\n");
                        
                        for (int k = 0; k < userNumber; ++k) {
                            if (k != j) {
                                send(users_socket[k], message, strlen(message), 0);
                            }
                        }

                    }
                }
            }
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}