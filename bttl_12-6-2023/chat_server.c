#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>

#define MAX_FILENAME_SIZE 256
#define MAX_BUFFER_SIZE 1024

void send_file_list(int client_socket)
{
  DIR *dir;
  struct dirent *ent;
  char response[MAX_BUFFER_SIZE];
  int file_count = 0;
  size_t response_len = 0;

  dir = opendir(".");
  if (dir != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      if (ent->d_type == DT_REG)
      { // Only regular files
        file_count++;
        response_len += snprintf(response + response_len, MAX_BUFFER_SIZE - response_len, "%s\r\n", ent->d_name);
      }
    }
    closedir(dir);

    if (file_count > 0)
    {
      char header[MAX_BUFFER_SIZE];
      snprintf(header, MAX_BUFFER_SIZE, "OK%d\r\n", file_count);
      send(client_socket, header, strlen(header), 0);
      send(client_socket, response, response_len, 0);
      send(client_socket, "\r\n", 2, 0);
      return;
    }
  }

  // No files to download
  send(client_socket, "ERROR No files to download\r\n", 28, 0);
  close(client_socket);
}

void send_file(int client_socket, const char *filename)
{
  FILE *file = fopen(filename, "rb");
  if (file != NULL)
  {
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char header[MAX_BUFFER_SIZE];
    snprintf(header, MAX_BUFFER_SIZE, "OK %ld\r\n", file_size);
    send(client_socket, header, strlen(header), 0);

    char buffer[MAX_BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
      send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
  }
  else
  {
    // File not found
    send(client_socket, "ERROR File not found\r\n", 22, 0);
    return;
  }

  close(client_socket);
}

void handle_client(int client_socket)
{
  char request[MAX_FILENAME_SIZE];
  ssize_t bytes_received = recv(client_socket, request, sizeof(request) - 1, 0);
  if (bytes_received > 0)
  {
    request[bytes_received] = '\0';

    // Remove trailing newline characters
    char *newline = strchr(request, '\n');
    if (newline != NULL)
    {
      *newline = '\0';
    }

    // Send file list
    if (strlen(request) == 0)
    {
      send_file_list(client_socket);
      return;
    }

    // Send requested file
    send_file(client_socket, request);
  }
}

int main()
{
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_len;

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Failed to create socket");
    return 1;
  }

  // Set up server address
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(9000);

  // Bind socket to address
  if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    perror("Failed to bind socket");
    return 1;
  }

  // Listen for incoming connections
  if (listen(server_socket, 5) < 0)
  {
    perror("Failed to listen for connections");
    return 1;
  }

  printf("Server listening on port 9000...\n");

  while (1)
  {
    client_address_len = sizeof(client_address);

    // Accept client connection
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
    if (client_socket < 0)
    {
      perror("Failed to accept client connection");
      return 1;
    }

    // Handle client in a separate process
    pid_t pid = fork();
    if (pid < 0)
    {
      perror("Failed to create child process");
      return 1;
    }
    else if (pid == 0)
    {
      // Child process
      close(server_socket);
      handle_client(client_socket);
      exit(0);
    }
    else
    {
      // Parent process
      close(client_socket);
    }
  }

  // Close server socket
  close(server_socket);

  return 0;
}
