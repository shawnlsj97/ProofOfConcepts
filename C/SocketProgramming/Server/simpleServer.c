#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // 1) Create Socket (similar to client)
    //      3 arguments for socket function:
    //          1) Domain of socket (AF_INET for IPv4, AF_INET for IPv6)
    //          2) Type of socket (SOCK_STREAM for TCP, SOCK_DGRAM for UDP)
    //          3) Protocol (0 to automatically choose correct protocol based on type of socket)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2) Bind Socket
    //      - Bind socket to specific IP address and port number
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

    // 4) Accept Connections Sequentially
    //      3 arguments for accept function:
    //          1) Socket file descriptor
    //          2) Pointer to sockaddr struct to hold client's address
    //          3) Pointer to socklen_t to hold size of client's address
    int running = 1; // Control flag
    struct sockaddr_in cli_addr;
    while(running) {
        socklen_t clilen = sizeof(cli_addr); // Reset size before each call to accept
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        // 5) Receive and Send Data
        //      - Use send and recv functions similar to client side
        //      - Use new file descriptor returned by accept function
        char buffer[4096];
        int n = recv(newsockfd, buffer, 4095, 0);
        buffer[n] = '\0'; // Add null terminator

        if (strcmp(buffer, "end") == 0) { // Check condition to stop server (could be signal, error, etc)
            printf("Received 'end' from client\n");
            running = 0;
        }

        char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(newsockfd, response, strlen(response), 0);

        // 6) Close Connections
        //      - Close client connections first
        close(newsockfd);
    }
    
    close(sockfd);

    return 0;
}