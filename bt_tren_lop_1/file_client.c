#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main() {
    // Khai bao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi cua server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    // Ket noi den server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        printf("Khong ket noi duoc den server!");
        return 1;
    }
    char* fileName = "file01-02.txt";

    // gui noi dung file den server
    FILE * file = fopen(fileName, "rb");
    if(file == NULL) {
        perror("ERROR");
        exit(1);
    }

    char buf[64];

    while (1) {
        size_t numBytes = fread(buf, 1, 64, file);
        write(client, buf, numBytes);
        if(numBytes < 64) break;
    }
    fclose(file);

    // Ket thuc, dong socket
    close(client);

    return 0;
}