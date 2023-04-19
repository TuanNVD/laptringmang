#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

int main() 
{
    // Tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener != -1)
        printf("Socket created: %d\n", listener);
    else
    {
        printf("Failed to create socket.\n");
        exit(1);
    }

    // Khai bao cau truc dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan dia chi voi socket
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        printf("bind() failed.\n");
        exit(1);
    }

    if (listen(listener, 5)) 
    {
        printf("listen() failed.\n");
        exit(1);
    }

    printf("Waiting for a new client ...\n");

    // Chap nhan ket noi
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        printf("accept() failed.\n");
        exit(1);
    }
    printf("New client connected: %d\n", client);
    

    // nhan data tu client
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;

    int pos = 0;
    char pcname[64];
    char disk;
    short int diskSize;
    
    strcpy(pcname, buf);
    pos += strlen(pcname) + 1;
    printf("+Tên máy tính: %s\n", pcname);

    int numDisk = (ret - pos) / 3;

    printf("+Số ổ đĩa %d\n", numDisk);
    for(int i = 0; i < numDisk; i++) {
        char drive_letter;
        short int drive_size;

        drive_letter = buf[pos];
        pos++;

        memcpy(&drive_size, buf + pos, sizeof(drive_size));
        pos += sizeof(drive_size);

        printf("  %c - %hd GB\n", drive_letter, drive_size);
    }
    
    // Dong ket noi
    close(client);
    close(listener);    

    return 0;
}
