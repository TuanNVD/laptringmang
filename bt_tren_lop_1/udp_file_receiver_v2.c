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


    
    while(1) {
        
        char buf[1024];
        char file_name[64];

        struct sockaddr_in sender_addr;
        int sender_addr_len = sizeof(sender_addr);

        int ret = recvfrom(receiver, buf, sizeof(buf), 0,
            (struct sockaddr*)&sender_addr, &sender_addr_len);
        buf[ret] = '\0';
        printf("%d bytes nhận từ địa chỉ %s:%d\n", ret,
        inet_ntoa(sender_addr.sin_addr), sender_addr.sin_port);
        
        sprintf(file_name, "%d.txt", sender_addr.sin_port);
        FILE *file = fopen(file_name, "w");

        fwrite(buf, 1, strlen(buf), file);
        fclose(file);
    }

    return 0;
}