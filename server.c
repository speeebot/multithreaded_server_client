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
#include <semaphore.h>

#define IP_ADDR_LOCAL "127.0.0.1"
#define PORT_NUM 10502
#define BUFFLEN 20
#define NUM_CLIENTS 3

sem_t sem;
char buf[BUFFLEN];

void *handle_client(void *arg) {
  int conn = *((int*)arg);
  int len;

  len = recv(conn, buf, BUFFLEN, 0); 
  sem_wait(&sem);
  if(len > 0) { 
    printf("Recieved: ");
    fflush(stdout);
    write(0, buf, len);
    sleep(2);
    write(conn, buf, len);
  }
  else if(len == 0) {
    close(conn);
    return 0;
  } 
  else {
    perror("recv() failed");
    exit(EXIT_FAILURE); 
  }
  sem_post(&sem);
  close(conn);
  return 0;
}

int main()
{
  int thread_num = 0, on = 1;
  pthread_t tid[NUM_CLIENTS];
  struct sockaddr_in server_addr;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror("setsockopt() failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_NUM);
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR_LOCAL);

  if (bind(sock,(struct sockaddr *)&server_addr, sizeof server_addr) == -1) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(sock, NUM_CLIENTS) == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
  printf("listening...\n");

  sem_init(&sem, 0, 1);

  for (;;) {  
    int conn = accept(sock, NULL, NULL);
    if (conn == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    printf("\nclient connected.\n");
  
    if(pthread_create(&tid[thread_num], NULL, handle_client, (void*)&conn) != 0) {
      perror("thread creation failed\n");
      exit(EXIT_FAILURE);
    }   
    
    pthread_join(tid[thread_num], NULL);

    thread_num++;
    if(thread_num == NUM_CLIENTS)
      break;
  }
  close(sock);
  sem_destroy(&sem);
  return EXIT_SUCCESS;  
}
