#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

volatile sig_atomic_t quit = 0; // can be safely accessed and modified by main thread and worker threads

void* handle_client(void* arg) {
    pthread_t id = pthread_self();
    printf("Thread ID: %lu\n", (unsigned long)id);

    int newsockfd = *(int*)arg;
    char buffer[4096];
    int n = recv(newsockfd, buffer, 4095, 0);
    buffer[n] = '\0'; // Add null terminator

    if (strcmp(buffer, "end") == 0) { // Check condition to stop server (could be signal, error, etc)
        printf("Received 'end' from client\n");
        quit = 1;
    }

    char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(newsockfd, response, strlen(response), 0);

    close(newsockfd);
}

int main() {
    // 1) Create Socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2) Bind Socket
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); // port number
    serv_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local address

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    // 3) Listen for Connections
    listen(sockfd, 5); // allow up to 5 pending connections

    // 4) Accept Connections
    struct sockaddr_in cli_addr;
    while(!quit) {
        socklen_t cli_len = sizeof(cli_addr); // Reset size before each call to accept
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        pthread_t thread_id;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        // Set thread to be in detached state to prevent zombie threads. A detached thread automatically cleans up its resources when it finishes execution
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        // 5) Receive and Send Data
        pthread_create(&thread_id, &attr, handle_client, &newsockfd);

        pthread_attr_destroy(&attr); // Destroy thread attributes object since we do not need it anymore

        // No need to call pthread_join() as server runs indefinitely so it does not need to wait for all client-handling threads to finish
    }
    
    // 6) Close connections
    close(sockfd);

    return 0;
}