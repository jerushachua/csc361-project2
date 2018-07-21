#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>


/* TCP Client */

int main(int argc, char *argv[])
{

    // Input parameter variables
    //
    char ip_addr[16];
    int port_no;
    char filename[64];

    // Checks that 3 parameters are given
    //
    if (argc == 4){
        strcpy(ip_addr, argv[1]);
        port_no = atoi(argv[2]);
        strcpy(filename, argv[3]);
        printf("IP: %s\nPort Number: %d\nFilename: %s\n", ip_addr, port_no, filename);
    }
    else{
        printf("Incorrect number of arguments given. \n");
        return(EXIT_FAILURE);
    }

    // Socket initialization variables
    //
    int client_socket;
    struct sockaddr_in sa;
    socklen_t sock_len;

    // Initialization of TCP socket
    //
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        printf("Error creating socket. \n");
        return(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    inet_pton(AF_INET, ip_addr, &(sa.sin_addr));

    // Connect to the server
    //
    sock_len = sizeof(sa);
    if( connect(client_socket, (struct sockaddr *)&sa, sock_len) == -1){
        printf("Error connecting to socket. \n");
        return(EXIT_FAILURE);
    }else{
        printf("Sucessfully connected to server. \n");
    }

    // Open the requested file
    //
    FILE *fp;
    fp = fopen(filename,"r");
    if(fp == NULL){
      printf("Error opening file.\n");
      return(EXIT_FAILURE);
    }
    printf("file opened, sending file!\n");

    // Start timer
    //
    printf("Start timer!\n");
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Send the file to the server
    //
    char send_buffer[1024];
    int bytes_sent = 0;
    int nread;
    int total_bytes = 0;
    while(!feof(fp)){
        bzero(send_buffer, 1024);
        nread = fread(send_buffer, 1, 1024, fp);
        bytes_sent = send(client_socket, send_buffer, nread, 0);
        if (bytes_sent < 0){
            printf("Error in sending file to client.\n");
            return(EXIT_FAILURE);
        }
        printf("Number of bytes sent: %d\n", bytes_sent);
        total_bytes = total_bytes + bytes_sent;
    }
    printf("done sending all packets\n");
    printf("total bytes sent: %d\n", total_bytes);
    memset(send_buffer, 0, sizeof(send_buffer));
    bytes_sent = send(client_socket, send_buffer, strlen(send_buffer), 0);
    printf("bytes in last packet sent: %d\n", bytes_sent);
    fclose(fp);
    close(client_socket);

    // Initialization of another TCP socket
    //
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        printf("Error creating socket. \n");
        return(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    inet_pton(AF_INET, ip_addr, &(sa.sin_addr));

    // Connect to the server again
    //
    sock_len = sizeof(sa);
    if( connect(client_socket, (struct sockaddr *)&sa, sock_len) == -1){
        printf("Error connecting to socket. \n");
        return(EXIT_FAILURE);
    }else{
        printf("Sucessfully connected to server. \n");
    }

    // Open a new file pointer to store the incoming data
    //
    char receive_buffer[1024];
    int bytes_received = 0;
    FILE *new;
    new = fopen("unzipped.txt","w");
    if(new == NULL){
      printf("Error opening file.\n");
      return(EXIT_FAILURE);
    }

    // Receive the message from the server
    //
    total_bytes = 0;
    while(1){

        memset(receive_buffer, 0, sizeof(receive_buffer));
        bytes_received = recv(client_socket, receive_buffer, sizeof receive_buffer, 0);

        // Error in receiving packet
        if(bytes_received < 0) {
            printf("Error in receiving file data.\n");
            return(EXIT_FAILURE);

        // Sucessfully received packet
        }else if(bytes_received >0){
            printf("Number of bytes received: %d\n", bytes_received);
            total_bytes = total_bytes + bytes_received;
            if(fwrite(receive_buffer, 1, bytes_received, fp) < 0){
                printf("Error writing to file.\n");
                return(EXIT_FAILURE);
            }
        }else if(bytes_received == 0){
            printf("done receiving all packets\n");
            printf("total bytes received: %d\n", total_bytes);
            fclose(new);
            break;
        }
    }

    // Stop timer
    //
    gettimeofday(&end, NULL);
    double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    printf("Stop timer!\n");
    printf("Time taken to send and receive file: %f milliseconds\n", delta*1000);


    return(EXIT_SUCCESS);
}
