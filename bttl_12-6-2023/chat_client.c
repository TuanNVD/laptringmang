#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_FILENAME_SIZE 256
#define MAX_BUFFER_SIZE 1024

void receive_file_list(int server_socket)
{
  char response[MAX_BUFFER_SIZE];
  ssize_t bytes_received = recv(server_socket, response, sizeof(response) - 1, 0);
  if (bytes_received > 0)
  {
    response[bytes_received] = '\0';

    // Check if there are files to download
    if (strncmp(response, "OK", 2) == 0)
    {
      int file_count;
      sscanf(response, "OK%d", &file_count);

      printf("Available files:\n");
      for (int i = 0; i < file_count; i++)
      {
        bytes_received = recv(server_socket, response, sizeof(response) - 1, 0);
        if (bytes_received > 0)
        {
          response[bytes_received] = '\0';
          printf("- %s", response);
        }
      }
    }
    else
    {
      printf("No files available for download.\n");
    }
  }
}

void receive_file(int server_socket, const char *filename)
{
  char response[MAX_BUFFER_SIZE];
  ssize_t bytes_received = recv(server_socket, response, sizeof(response) - 1, 0);
  if (bytes_received > 0)
  {
    response[bytes_received] = '\0';

    // Check if file exists
    if (strncmp(response, "OK", 2) == 0)
    {
      long file_size;
      sscanf(response, "OK %ld", &file_size);

      FILE *file = fopen(filename, "wb");
      if (file != NULL)
      {
        char buffer[MAX_BUFFER_SIZE];
        long bytes_remaining = file_size;
        ssize_t bytes_written;

        while (bytes_remaining > 0)
        {
          ssize_t bytes_to_read = (bytes_remaining < sizeof(buffer)) ? bytes_remaining : sizeof(buffer);
          bytes_received = recv(server_socket, buffer, bytes_to_read, 0);
          if (bytes_received > 0)
          {
            bytes_written = fwrite(buffer, 1, bytes_received, file);
            if (bytes_written < bytes_received)
            {
              printf("Error writing to file.\n");
              break;
            }

            bytes_remaining -= bytes_received;
          }
          else
          {
            printf("Error receiving file data.\n");
            break;
          }
        }

        fclose(file);
        printf("File received successfully.\n");
      }
      else
      {
        printf("Error creating file.\n");
      }
    }
    else
    {
      printf("File not found.\n");
    }
  }
}

int main()
{
  int client_socket;
  struct sockaddr_in server_address;

  // Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0)
  {
    perror("Failed to create socket");
    return 1;
  }

  // Set up server address
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
  server_address.sin_port = htons(9000);                   // Server port

  // Connect to server
  if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    perror("Failed to connect to server");
    return 1;
  }

  // Receive file list
  receive_file_list(client_socket);

  // Prompt for file name
  char filename[MAX_FILENAME_SIZE];
  printf("Enter file name to download (or press Enter to exit): ");
  fgets(filename, sizeof(filename), stdin);
  filename[strcspn(filename, "\r\n")] = '\0'; // Remove trailing newline characters

  if (strlen(filename) > 0)
  {
    // Send file request to server
    send(client_socket, filename, strlen(filename), 0);

    // Receive requested file
    receive_file(client_socket, filename);
  }

  // Close client socket
  close(client_socket);

  return 0;
}
