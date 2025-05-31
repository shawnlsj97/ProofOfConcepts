#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // 1) Create Socket
    //      3 arguments for socket function:
    //          1) Domain of socket (AF_INET for IPv4, AF_INET for IPv6)
    //          2) Type of socket (SOCK_STREAM for TCP, SOCK_DGRAM for UDP)
    //          3) Protocol (0 to automatically choose correct protocol based on type of socket)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2) Connect to Server
    //      3 arguments for connect function:
    //          1) Socket file descriptor
    //          2) Address to connect to
    //          3) Size of address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); // port number
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // IP address (localhost in this example)

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // 3) Send Data
    //      4 arguments for send function:
    //          1) Socket file descriptor
    //          2) Data to send
    //          3) Length of data
    //          4) Flag (0 for default behavior)
    char* request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    send(sockfd, request, strlen(request), 0); 
    
    // 4) Receive Response
    //      4 arguments for recv function:
    //          1) Socket file descriptor
    //          2) Buffer to store received data
    //          3) Buffer size
    //          4) Flag (0 for default behavior)
    char response[4096];
    int n = recv(sockfd, response, 4095, 0);
    response[n] = '\0'; // Add null terminator


    // 5) Close Socket
    close(sockfd);

    return 0;
}