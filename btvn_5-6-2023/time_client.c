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

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Missing arguments\n");
    exit(1);
  }

  int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (clientSocket == -1)
  {
    perror("Create socket failed: ");
    exit(1);
  }
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addr.sin_port = htons(atoi(argv[2]));

  if (connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    printf("Connected failed\n");
    exit(1);
  }
  printf("Connected successfully\n");

  char buff[256];
  if (fork() == 0)
  {
    while (1)
    {
      fgets(buff, sizeof(buff), stdin);
      send(clientSocket, buff, strlen(buff), 0);
    }
  }
  else
  {
    while (1)
    {
      int ret = recv(clientSocket, buff, sizeof(buff), 0);
      if (ret <= 0)
        break;
      buff[ret] = 0;
      printf("%s", buff);
    }
  }

  printf("Disconnected\n");
  close(clientSocket);
  printf("Socket closed\n");
  killpg(0, SIGKILL);
  return 0;
}