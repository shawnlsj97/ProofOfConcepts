#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// Server uses OS mechanisms to monitor multiple fidle descriptors and gets notified when one of them is ready for reading/writing, so single thread can manage multiple clients
// Technically processing each client one at a time but in very quick succession, so from client perspective, they are being serviced simultaneously
// select(), poll(), epoll() to check which sockets have data ready to be read or written

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
    //      2 arguments for listen function:
    //          1) Socket file descriptor
    //          2) Max size of queue of pending connections (backlog)
    listen(sockfd, 5); // allow up to 5 pending connections

    // 4) Accept Connections
    fd_set read_fds;
    FD_ZERO(&read_fds); // initialize read_fds to empty set
    FD_SET(sockfd, &read_fds); // add sockfd to read_fds for select() to monitor it
    int running = 1; // Control flag
    struct sockaddr_in cli_addr;
    while(running) {
        fd_set temp_fds = read_fds; // make a copy because select changes the set

        // 5 arguments for select function:
        //      1) Highest-numbered file descriptor in any of the sets + 1 (FD_SETSIZE is the highest number fd_set can hold)
        //      2) readfds: File descriptor set to be checked for readability
        //      3) writefds: File descriptor set to be checked for writability
        //      4) exceptfds: File descriptor set to be checked for errors
        //      5) Timeout: max interval to wait for select to complete. If NULL, can block indefinitely. If set to 0, select will return immediately without blocking.
        // No need to monitor writefds in this case because when we write using send(), data doesn't get sent over network immediately, instead it goes into OS's send buffer and will be sent when possible
        if (select(FD_SETSIZE, &temp_fds, NULL, NULL, NULL) == -1) { // select() blocks until one or more fds are ready for I/O
            perror("ERROR on select");
            exit(1);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &temp_fds)) {
                if (i == sockfd) {
                    // new connection
                    socklen_t cli_len = sizeof(cli_addr); // Reset size before each call to accept
                    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
                    if (newsockfd < 0) {
                        perror("ERROR on accept");
                        exit(1);
                    }
                    FD_SET(newsockfd, &read_fds);
                } else {
                    // 5) Receive and Send Data
                    //      - i corresponds to client file descriptor
                    char buffer[4096];
                    int n = recv(i, buffer, 4095, 0);
                    buffer[n] = '\0'; // Add null terminator

                    if (strcmp(buffer, "end") == 0) { // Check condition to stop server (could be signal, error, etc)
                        printf("Received 'end' from client\n");
                        running = 0;
                    }

                    char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
                    send(i, response, strlen(response), 0);

                    close(i); // any data left in the send buffer will continue to transmit
                }
            }
        }
    }
    
    // 6) Close Connections
    close(sockfd);

    return 0;
}