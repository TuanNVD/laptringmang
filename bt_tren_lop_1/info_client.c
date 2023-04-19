#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

void clean_stdin() {
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

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
    
    char buf[256];
    char pcname[64];
    char diskName;
    short int diskSize;
    int numDisk;

    printf("Nhập dữ liệu:\n");
    printf("Nhập tên máy tính: ");
    scanf("%s", pcname);
    getchar(); // xoa ki tu \n

    strcpy(buf, pcname);

    int pos = strlen(pcname);
    buf[pos] = 0; pos++;

    printf("Nhập số ổ đĩa: ");
    scanf("%d", &numDisk);
    getchar();

    for(int i = 0; i <numDisk; i++) {
        printf("Tên ổ đĩa thứ %d: ", i+1);
        scanf("%c", &diskName);
        printf("Dung lượng ổ đĩa: ");
        scanf("%hd", &diskSize);

        getchar();
        buf[pos] = diskName;
        pos++;
        buf[pos] = diskSize;
        memcpy(buf + pos, &diskSize, sizeof(diskSize));
        pos += sizeof(diskSize);
        
    }

    // Gui du lieu den server
    send(client, buf, pos, 0);

    // Ket thuc, dong socket
    close(client);

    return 0;
}