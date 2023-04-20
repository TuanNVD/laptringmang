#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    printf("Waiting for a new client ...\n");

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (client == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", client);

    // Truyen nhan du lieu
    
    char str[11] = "0123456789";
    int res = 0;
    char buf[64];
    
    while (1) {
        char tmp[18];
        for(int i = 55, j = 0; i <= 63; i++, j++) tmp[j] = buf[i];
        int ret = read(client, buf, 64);
        buf[ret] = 0;

        int min = ret < 8 ? ret : 8;
        for(int i = 0, j = 9; i <= min; i++, j++) tmp[j] = buf[i];
        
        if(strstr(tmp, str) != NULL) res++;

        int buflength = strlen(buf);
        int strlength = strlen(str);
        for(int i = 0; i <= buflength - strlength; i++) {
            if(strncmp(buf+i, str, strlength) == 0) {
                res ++;
            }
        }
        if(ret < 64) {
            buf[ret] = 0;
            break;
        }
    }

    printf("Số lần xuất hiện xâu \"%s\" là: %d\n", str, res);

    // Dong ket noi
    close(client);
    close(listener);    

    return 0;
}