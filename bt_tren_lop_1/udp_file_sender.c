#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    char *file_name = argv[3];

    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // Gửi tên file
    sendto(sender, file_name, strlen(file_name), 0,
         (struct sockaddr*)&addr, sizeof(addr));

    char buf[1024];
    FILE *file = fopen(file_name, "r");
    fread(buf, 1, 1024, file);
    int ret = sendto(sender, buf, strlen(buf), 0,
        (struct sockaddr*)&addr, sizeof(addr));
    printf("%d bytes đã gửi\n", ret);

    fclose(file);

}