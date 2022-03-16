//Shawn Diaz
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define IP_ADDR_LOCAL "127.0.0.1"
#define PORT_NUM 10502
#define BUFFLEN 20
#define MAX_CLIENTS 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char buf[BUFFLEN];

void *handle_client(void *arg) {
  int conn = *((int*)arg);
  int len;

  if (conn == -1) {
    perror("accept failed");
    close(conn);
    exit(EXIT_FAILURE);
  }
  printf("client connected.\n");

  len = recv(conn, buf, BUFFLEN, 0);
  if(len > 0) {
    printf("Recieved: ");
    fflush(stdout);
    pthread_mutex_lock(&mutex);
    write(0, buf, len);
    write(conn, buf, len);
    pthread_mutex_unlock(&mutex);
    printf("\n");
  }

  if (shutdown(conn, SHUT_RDWR) == -1) {
    perror("shutdown failed");
    close(conn);
    exit(EXIT_FAILURE);
  }
  return 0;
}

int main()
{
  struct sockaddr_in server_addr;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_NUM);
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR_LOCAL);

  if (bind(sock,(struct sockaddr *)&server_addr, sizeof server_addr) == -1) {
    perror("bind failed");
    close(sock);
    exit(EXIT_FAILURE);
  }

  if (listen(sock, MAX_CLIENTS) == -1) {
    perror("listen failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("listening...\n");

  pthread_t tid[MAX_CLIENTS];
  int thread_num = 0;

  for (;;) {
    int conn = accept(sock, NULL, NULL);
    if(pthread_create(&tid[thread_num++], NULL, handle_client, (void*)&conn) != 0) {
      perror("thread creation failed\n");
      exit(EXIT_FAILURE);
    }

    if(thread_num >= 3) {
      thread_num = 0;
      while(thread_num < 3) {
        pthread_join(tid[thread_num++], NULL);
      }
      break;
    }
  }
  close(sock);
  return EXIT_SUCCESS;  
}
