#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[1024];

    char file_name[64], file_content[960];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);

    recvfrom(receiver, file_name, sizeof(file_name), 0,
        (struct sockaddr*)&sender_addr, &sender_addr_len);
    int ret = recvfrom(receiver, file_content, sizeof(file_content), 0,
        (struct sockaddr*)&sender_addr, &sender_addr_len);
    printf("%d bytes nhận từ file %s địa chỉ %s:%d\n", ret, file_name, 
            inet_ntoa(sender_addr.sin_addr), sender_addr.sin_port);
    
    file_name[strlen(file_name)] ='\n';
    file_name[strlen(file_name) + 1] = 0;

    strcpy(buf, file_name);
    strcat(buf, file_content);

    FILE *file = fopen("bt01-03.txt", "w");
    fwrite(buf, 1, strlen(buf), file);
    fclose(file);

}