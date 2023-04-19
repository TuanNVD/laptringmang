#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    
    char *ipAddr = argv[1];
    int port = atoi(argv[2]);

    // Tao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipAddr);
    addr.sin_port = htons(port);

    // Ket noi den server
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1) {
        perror("Ket noi that bai");
        exit(1);
    }

    // Hien thi cau chao tu server 
    char hello_buf[1024];
    read(client, hello_buf, 1024);
    puts(hello_buf);

    //  Nhap du lieu tu ban phim va gui toi may chu
    char buf[1024];
    printf("Nhap du lieu gui toi may chu(nhan q de thoat):\n");

    while(1) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);

        int ret = send(client, buf, sizeof(buf), 0);
        if(buf[0] == 'q') {
            printf("Gui du lieu thanh cong!\n");
            break;
        }
    }

    close(client);
    return 0;
}