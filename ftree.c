#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include "ftree.h"
#include "hash.h"


#ifndef PORT
#define PORT 66022
#endif
#define MAX_BACKLOG 5

int rcopy_client(char *src_path, char *dest_path, char *host_ip, int port) {
   printf("SRC: %s\n", src_path);
   printf("DEST: %s\n", dest_path);
   printf("IP: %s\n", host_ip);
   printf("PORT: %d\n", port);

  // Create the socket FD.
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
      perror("client: socket"); 
      return 1;
  }

  // Set the IP and port of the server to connect to.
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1) {
      perror("client: inet_pton");
      close(sock_fd);
      return 1;
  }

  // Connect to the server.
  if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
      perror("client: connect");
      close(sock_fd);
      return 1;
  }

  int id = htonl(CHECKER_CLIENT);
       if (write(sock_fd, &id, sizeof(int)) != sizeof(int)){
               perror("client: write");
                close(sock_fd);
               return 1;
       }
       struct stat sb, sc;
       if ((lstat(src_path, &sb)) == -1){
               perror("lstat");
               return 1;}


       if ((lstat(dest_path, &sc)) == -1){
               perror("lstat");
               return 1;
       }

  //Check if source is a link or not
       if (S_ISLNK(sb.st_mode)){
      return 0;
               }


  //Check if source is a file or not
       if S_ISREG(sb.st_mode){


      struct fileinfo *finfo = malloc(sizeof(struct fileinfo));
      // make the absolute math
      char *path=malloc(sizeof(char)*MAXPATH);
      strcpy(path,dest_path);
      strcat(path,"/");
      strcat(path,basename(strdup(src_path)));


               //get the mode of the source
               mode_t *mode = malloc(sizeof(mode_t));
               *mode = sb.st_mode;

      //opening file for hashing
       FILE *file = fopen(src_path, "rb");
       if (file == NULL){
               perror("fopen");
               return 1;
           }
       char *hash1 = malloc(sizeof(char)*8+sizeof(char));
       hash1 = hash(file);
      hash1[BLOCKSIZE] = '\0';

      //closing the file
      if(fclose(file)!=0){
          perror("fclose");
          return 1;
      }
      // geeting the size of the file
      size_t *size=malloc(sizeof(size_t));
      *size=sb.st_size;


      strcpy(finfo->path,path);
      finfo->mode=*mode;
      strcpy(finfo->hash, hash1);
      finfo->size = *size;

               printf("%s\r\n",finfo->path);
               printf("%d\r\n",finfo->mode);
               printf("%s\r\n",finfo->hash);
               printf("%lu\r\n",finfo->size);

               finfo->mode = htonl(*mode);
               finfo->size = htonl(*size);

               //send the struct to the server
               if ((write(sock_fd, finfo->path, sizeof(char)*MAXPATH)) != sizeof(char)*MAXPATH){
                       perror("client: write");
                         close(sock_fd);
                       return 1;
               }
               if (write(sock_fd, &(finfo->mode), sizeof(mode_t)) != sizeof(mode_t)){
                       perror("client: write");
                         close(sock_fd);
                       return 1;
               }

               printf("hash\n");
                if (write(sock_fd, finfo->hash, sizeof(char)*BLOCKSIZE) != sizeof(char)*BLOCKSIZE){
                 perror("client: write");
                         close(sock_fd);
                 return 1;
               }
               if (write(sock_fd, &(finfo->size), sizeof(size_t)) != sizeof(size_t)){
                       perror("client: write");
                         close(sock_fd);
                       return 1;
               }


}

return 0;

}





void rcopy_server(int port) {
   printf("PORT: %d\n", port);

  // creating socket 
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
      perror("server: socket");
      exit(1);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = INADDR_ANY;

  memset(&server.sin_zero, 0, 8);

  // binding port to socket
  if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
      perror("server: bind");
      close(sock_fd);
      exit(1);
  }

  // Listen
  if (listen(sock_fd, MAX_BACKLOG) < 0) {
      perror("server: listen");
      close(sock_fd);
      exit(1);
  }

  // int max_fd = sock_fd;
  // fd_set all_fds, listen_fds;
  // FD_ZERO(&all_fds);
  // FD_SET(sock_fd, &all_fds);

  while (1) {

      // Is it the original socket? Create a new connection ...{
          int client_fd = accept(sock_fd, NULL,NULL);
          if (client_fd<0) {
              perror("server: accept");
              close(client_fd);
              continue;
                       }
          printf("Accepted connection\n");
                   int id;
                   if (read(client_fd, &id, sizeof(int)) != sizeof(int)){
                           perror("server: read");
               printf("id\n");
                         close(client_fd);
                           continue;}

                   id = ntohl(id);

                   //check if the client is a checker
                   if (id == CHECKER_CLIENT){
                           printf("Client says: I'm the checker\r\n");

              struct fileinfo *finfo = malloc(sizeof(struct fileinfo));

              //recieving struct from the client
              if (read(client_fd, finfo->path, sizeof(char)*MAXPATH) != sizeof(char)*MAXPATH){
                  perror("server: read");
                  printf("path\n");
                      close(client_fd);
                  continue;
              }
              if (read(client_fd, &(finfo->mode), sizeof(mode_t)) != sizeof(mode_t)){
                  perror("server: read");
                  printf("mode\n");
                      close(client_fd);
                  continue;
              }
               if (read(client_fd, finfo->hash, sizeof(char)*BLOCKSIZE) != sizeof(char)*BLOCKSIZE){
                   perror("server: read");
                    printf("hash\n");
                       close(client_fd);
                   continue;
               }
              if (read(client_fd, &(finfo->size), sizeof(size_t)) != sizeof(size_t)){
                  perror("server: read");
                  printf("size\n");
                      close(client_fd);
                  continue;
              }

              finfo->mode = ntohl(finfo->mode);
              finfo->size = ntohl(finfo->size);

              printf("%s\r\n",finfo->path);
              printf("%d\r\n",finfo->mode);
              printf("%s\r\n",finfo->hash);
              printf("%lu\r\n",finfo->size);

      }


   exit(1);
} } 