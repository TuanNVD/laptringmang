#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>

char* getFileName(char *filepath)
{
    char *pos = strrchr(filepath, '/');
    if (pos == NULL)
    {
        if (strchr(filepath, '.') == NULL)
            return NULL;
        else
        {
            char *res = malloc(strlen(filepath) + 1);
            strcpy(res, filepath);
            res[strlen(res)] = 0;
            return res;
        }
    }

    int length = strlen(filepath) - (pos + 1 - filepath);
    if (length <= 0) return NULL;
    char *res = malloc(length + 1);
    memcpy(res, pos + 1, length);
    res[length] = 0;
    return res;
}

int main(int argc, char* argv[])
{
    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Connect to server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(21);
    
    if (connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Connected failed\n");
        exit(1);
    }
    printf("Connected successfully\n");

    char buff[1024];
    int ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
    buff[ret] = 0;
    printf("%s\n", buff);
    while (1)
    {
        char tmp[64];
        
        printf("Enter username: ");
        scanf("%s", tmp);
        sprintf(buff, "USER %s\r\n", tmp);
        send(clientSocket, buff, strlen(buff), 0);
        recv(clientSocket, buff, sizeof(buff), 0);

        printf("Enter password: ");
        scanf("%s", tmp);
        sprintf(buff, "PASS %s\r\n", tmp);
        send(clientSocket, buff, strlen(buff), 0);

        ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
        buff[ret] = 0;

        printf("%s\n", buff);
        if (strncmp(buff, "230", 3) == 0)
            break;
    }
    
    while (1)
    {
        printf("Enter file path to upload: ");
        scanf("%s", buff);
        if (strncmp(buff, "exit", 4) == 0)
            break;

        char *file_name = getFileName(buff);
        if (file_name == NULL)
        {
            printf("Invalid file path\n");
            continue;
        }

        FILE *f = fopen(buff, "rb");
        if (f == NULL)
        {
            printf("Can't open file\n");
            continue;
        }

        // PASV
        send(clientSocket, "PASV\r\n", 6, 0);
        ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
        buff[ret] = 0;

        char *pos1 = buff;
        int count = 0, first_byte_len = 0, second_byte_len = 0;
        
        while (count != 4)
        {
            pos1 = strchr(pos1, ',');
            pos1 = pos1 + 1;
            count++;
        }
        char *pos2 = strchr(pos1, ',');
        first_byte_len = pos2 - pos1;
        second_byte_len = strlen(buff) - (pos2 - buff) - 4;

        char *first_byte = malloc(first_byte_len + 1);
        char *second_byte = malloc(second_byte_len + 1);
        memcpy(first_byte, pos1, first_byte_len);
        first_byte[first_byte_len] = 0;
        memcpy(second_byte, pos2 + 1, second_byte_len);
        second_byte[second_byte_len] = 0;

        int port = 256 * atoi(first_byte) + atoi(second_byte);
        free(first_byte);
        free(second_byte);

        int dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(argv[1]);
        data_addr.sin_port = htons(port);

        if (connect(dataSocket, (struct sockaddr *)&data_addr, sizeof(data_addr)) == -1)
        {
            printf("Connected failed\n");
            fclose(f);
            continue;
        }
        sprintf(buff, "STOR %s\r\n", file_name);
        send(clientSocket, buff, strlen(buff), 0);
        ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);

        printf("Uploading ...\n");
        unsigned long int uploaded = 0;
        while (!feof(f))
        {
            ret = fread(buff, 1, sizeof(buff), f);
            if (ret <= 0) break;

            send(dataSocket, buff, ret, 0);
            uploaded += ret;
        }
        fclose(f);
        close(dataSocket);
        printf("Uploaded %lu bytes\n", uploaded);

        ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
    }

    send(clientSocket, "QUIT\r\n", 6, 0);

    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}