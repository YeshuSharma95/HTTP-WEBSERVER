//HTTP WEBSERVER
//YESHU SHARMA- Student ID: 110238267
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_CLIENTS 1024
#define MAX_DATA 8192
#define time_out 10

void *thread(void *vargp);
void httpserver(int connfd);


// Thread creation
void * thread(void * vargp)
{
  int connfd = *((int *)vargp);
  pthread_detach(pthread_self());
  free(vargp);
  httpserver(connfd);
  close(connfd);
  return NULL;
}


// HTTP Request function
void httpserver(int connfd)
{
  struct stat stats;
  struct timeval timeout;
  char buffer_data[MAX_DATA];
  char request_method[10];
  char request_uri[40];
  char request_version[30];
  char version[30];
  char buffer_http[8192];
  char buffer_errhttp[512];
  char error_msg[512];
  char post_data[10] = "POST DATA";
  char file_path[100];
  int optval, port;
  int read_data_size;
  FILE* fp;
  off_t file_size = 0;
  int flag=0;

  while ((recv(connfd, buffer_data, MAX_DATA, 0)) > 0)
  {
      printf("SERVER: HTTP Request:\n%s\n", buffer_data);
      sscanf(buffer_data, "%s %s %s", request_method, request_uri, request_version);
      printf("HTTP Method: %s\n", request_method);
      printf("HTTP URI: %s\n", request_uri);
      printf("HTTP Version: %s\n", request_version);
      memset(buffer_errhttp, 0, sizeof(buffer_errhttp));
      memset(error_msg, 0, sizeof(error_msg));
      memset(version, 0, sizeof(version));
      memset(file_path, 0, sizeof(file_path));

      strcpy(version, request_version);

      //Connection: Keep-alive
      char *pipeline = strstr(buffer_data, "Connection: Keep-alive");
      if (pipeline)
				{
					printf("SERVER: Connection Status: %s\n", pipeline);
					timeout.tv_sec = time_out;
					setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(struct timeval));
				}
			else
        {
          printf("SERVER: Connection Status: %s\n", pipeline);
					timeout.tv_sec = 0;
					setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(struct timeval));
				}

      // Connection:Close - request
      char *closepipeline = strstr(buffer_data, "Connection: Close");
      if(closepipeline)
        {
          printf("Connection Close Request: %s\n", closepipeline);
          sprintf(buffer_errhttp, "%s 200 OK\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n""Connection: %s\r\n\r\n", version, "text/html", 512, "Connection: Close");
          strcpy(error_msg, "<!DOCTYPE html><html><title>HTTP Request Connection Close</title>""<pre><h1>200 Connection Close</h1></pre>""</html>\r\n");
          send(connfd, buffer_errhttp, strlen(buffer_errhttp), 0);
          //printf("%s buffer error", buffer_errhttp);
          send(connfd, error_msg, strlen(error_msg), 0);
          close(connfd); // client close
        }

      //Check HTTP Request method
      if((strcmp(request_method, "GET") == 0)||(strcmp(request_method, "POST") == 0))
      {
         //printf("Validated method. Inside.\n");
         //Check the HTTP Request version
         if((strcmp(request_version, "HTTP/1.1") == 0)||(strcmp(request_version, "HTTP/1.0") == 0))
         {
           printf("SERVER: HTTP Request version is valid.\n");
         }
         else
         {
           printf("SERVER: HTTP Request version is invalid.\n");
           // Connection: keep alive
           if (pipeline)
             {
               sprintf(buffer_errhttp, "HTTP/1.1 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n""Connection: %s\r\n\r\n", "text/html", 512, "Keep-alive");
             }
           else
             {
               sprintf(buffer_errhttp, "HTTP/1.1 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n\r\n", "text/html", 512);
             }
           strcpy(error_msg, "<!DOCTYPE html><html><title>Invalid HTTP Request</title>""<pre><h1>500 Internal Server Error</h1></pre>""</html>\r\n");
           send(connfd, buffer_errhttp, strlen(buffer_errhttp), 0);
           //printf("%s buffer error", buffer_errhttp);
           send(connfd, error_msg, strlen(error_msg), 0);
           close(connfd);
           exit(1);
         }

         // Setting path to default folder
         strcpy(file_path, "./www");
         if (strcmp(request_uri,"/") == 0)
          {
            strcat(file_path, "/index.html");
          }
         else
          {
            strcat(file_path, request_uri);
          }

          printf("==========================\nRequest Processing Details\n");

         //check file access is okay
         if(access(file_path, F_OK) == 0)
         {

          printf("SERVER: Access file ok \n");


          // check - ???
         if(strcmp(request_method, "POST") == 0)
         {
           flag = 1;
           printf("SERVER: HTTP POST Request received. Flag for POST method is set to 1.\n");
         }


         if((strcmp(request_method, "GET") == 0)||(flag == 1))
          {
            long int file_size = 0;

            // getting the extension of the file
            const char ch = '.';
            char *file_ext;
            file_ext = strrchr(file_path, ch);
            printf("SERVER: File Extension: %s\n", file_ext);
            printf("SERVER: File Path is: %s\n", file_path);

            //check the type of extension
            if(strcmp(file_ext, ".html") == 0)
            {
              file_ext= "text/html";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".txt") == 0)
            {
              file_ext= "text/plain";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".png") == 0)
            {
              file_ext= "image/png";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".gif") == 0)
            {
              file_ext= "image/gif";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".jpg") == 0)
            {
              file_ext= "image/jpg";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".css") == 0)
            {
              file_ext= "text/css";
              printf("SERVER: Content type: %s\n", file_ext);
            }
            else if(strcmp(file_ext, ".js") == 0)
            {
            file_ext= "application/javascript";
            printf("SERVER: Content type: %s\n", file_ext);
            }
            else
            {
              file_ext= "text/html";
              printf("SERVER: Content type: %s\n", file_ext);
            }

            //calculating the size of the file
            stat(file_path, &stats);
            file_size = stats.st_size;
            printf("SERVER: File size: %ld\n", file_size);

            printf("==========================\n");

            if(pipeline)
            {
              //sprintf(buffer_errhttp, "HTTP/1.1 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n""Connection: %s\r\n\r\n", "text/html", 512, "Keep-alive");
              if(flag==1) // POST Method
                {
                  sprintf(buffer_http, "%s 200 OK\r\n""Content-Type: %s\r\n""Content-Length: %ld\r\n""Connection: %s\r\n""<html><body><pre><h1>%s</h1></pre>\r\n\r\n",version, file_ext, file_size, "Keep-alive", post_data);
                }
              else
                {
                  sprintf(buffer_http, "%s 200 OK\r\n""Content-Type: %s\r\n""Content-Length: %ld\r\n""Connection: %s\r\n",version, file_ext, file_size, "Keep-alive");
                }
              printf("SERVER: HTTP Response\n");
              //printf("Buffer header %s\n",buffer_http);
              send(connfd, buffer_http, strlen(buffer_http), 0);
            }

            else
            {
              //sprintf(buffer_errhttp, "HTTP/1.1 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n\r\n", "text/html", 512);
              if(flag==1) // POST METHOD
              {
                sprintf(buffer_http, "%s 200 OK\r\n""Content-Type: %s\r\n""Content-Length: %ld\r\n\r\n""<html><body><pre><h1>%s</h1></pre>\r\n\r\n",version, file_ext, file_size, post_data);
              }
              else
              {
                sprintf(buffer_http, "%s 200 OK\r\n""Content-Type: %s\r\n""Content-Length: %ld\r\n\r\n",version, file_ext, file_size);
              }
              //printf("Buffer header %s\n",buffer_http);
              send(connfd, buffer_http, strlen(buffer_http), 0);
            }

            //send the requested file to the client
            fp = fopen( file_path, "r");
            printf("SERVER: FILE OPENED %s\n",file_path);
            char *buf = (char *) malloc(file_size*sizeof(char));
            read_data_size = fread(buf, 1, file_size, fp);
            //printf("size %d \n", read_data_size);
            send(connfd, buf, read_data_size, 0);
            printf("SERVER: HTTP Response sent.\n");
            fclose(fp);
            free(buf);
          }
         }

         // Else Block: File access check failed
         else
         {
           // Connection: keep alive logic
           if (pipeline)
             {
               sprintf(buffer_errhttp, "%s 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n""Connection: %s\r\n\r\n",version, "text/html", 512, "Keep-alive");
             }
           else
             {
               sprintf(buffer_errhttp, "%s 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n\r\n",version, "text/html", 512);
             }
           printf("SERVER: File access check failed. Path received: %s\n", file_path);
           strcpy(error_msg, "<!DOCTYPE html><html><title>Invalid HTTP Request</title>""<pre><h1>500 Internal Server Error</h1></pre>""</html>\r\n");
           send(connfd, buffer_errhttp, strlen(buffer_errhttp), 0);
           //printf("%s buffer error", buffer_errhttp);
           send(connfd, error_msg, strlen(error_msg), 0);
         }

       }

       // Else Block: Unknown HTTP request method.
       else
       {
         printf("SERVER: Received Unknown Method\n");
         // Connection: Keep alive logic
         if (pipeline)
           {
             sprintf(buffer_errhttp, "%s 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n""Connection: %s\r\n\r\n",version, "text/html", 512, "Keep-alive");
           }
         else
           {
             sprintf(buffer_errhttp, "%s 500 Internal Server Error\r\n""Content-Type: %s\r\n""Content-Length: %d\r\n\r\n",version, "text/html", 512);
           }
        strcpy(error_msg, "<!DOCTYPE html><html><title>Invalid HTTP Request</title>""<pre><h1>500 Internal Server Error</h1></pre>""</html>\r\n");
        send(connfd, buffer_errhttp, strlen(buffer_errhttp), 0);
        //printf("%s buffer error\n", buffer_errhttp);
        send(connfd, error_msg, strlen(error_msg), 0);
       }

    }

  }


int main(int argc, char **argv)
  {
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    struct stat stats;
    //char data[MAX_DATA];
    //char buffer_data[MAX_DATA];
    char request_method[10];
    char request_uri[40];
    char request_version[30];
    //char version[30];
    //char buffer_http[8192];
    //char buffer_errhttp[512];
    int sockfd;
    int *connfd;
    int sockaddr_len = sizeof(struct sockaddr_in);
    //int data_len;
    int optval, port;
    pthread_t tid;

    //Checking the command line arguments
    if (argc != 2)
      {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
      }

    //Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
      {
        perror("ERROR on creating socket");
        exit(1);
      }
    printf("SERVER: Creation of socket is completed. Descriptor = %d\n", sockfd);

    optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int)) < 0)
      {
        perror("ERROR on setsockopt");
        exit(1);
      }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    port = atoi(argv[1]);

    //binding the socket
    if((bind(sockfd, (struct sockaddr *)&serveraddr, sockaddr_len)) < 0)
      {
        perror("ERROR on bind");
        exit(1);
      }
    printf("SERVER: Socket created and Bind done on port: %d\n", port);

    //After bind we listen on the port
    if((listen(sockfd, MAX_CLIENTS)) < 0)
      {
        perror("ERROR on listening");
        exit(1);
      }
    printf("SERVER: Listening....\n");

    while(1)
      {
        memset(request_method, 0, sizeof(request_method));
        memset(request_uri, 0, sizeof(request_uri));
        memset(request_version, 0, sizeof(request_version));
        connfd = malloc(sizeof(int));

        if((*connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &sockaddr_len)) < 0)
          {
            perror("ERROR on accept");
            continue;
          }
        printf("************ NEW CLIENT ************\n");
        printf("SERVER: New client connected from port no: %d and IP: %s\n", ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr));
        pthread_create(&tid, NULL, thread, connfd);

      }
      close(sockfd);
      exit(1);
    }

