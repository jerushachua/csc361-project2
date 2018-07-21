#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>


int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);


/* TCP Server */

int main(int argc, char *argv[])
{

    // Checks that 2 parameters are given
    //
    char ip_addr[16];
    int port_no;
    if (argc == 3){
        strcpy(ip_addr, argv[1]);
        port_no = atoi(argv[2]);
        printf("IP: %s\nPort Number: %d\n", ip_addr, port_no);
    }else{
        printf("Incorrect number of arguments given. \n");
        return(EXIT_FAILURE);
    }

    // Socket initialization variables
    //
    int server_socket, client_socket;
    struct sockaddr_in sa, client_addr;
    socklen_t sock_len;

    // Initialization of TCP server socket
    //
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("Error creating server socket. \n");
        return(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port_no);
    sa.sin_addr.s_addr = inet_addr(ip_addr);

    // Bind Socket
    //
    if( (bind(server_socket, (struct sockaddr *)&sa, sizeof(sa))) == -1){
        printf("Error binding socket. \n");
        return(EXIT_FAILURE);
    }

    // Listen for incoming connections
    //
    if( (listen(server_socket, 5)) == -1){
        printf("Error while listening for incoming connections. \n");
        return(EXIT_FAILURE);
    }else{
        printf("Listening... \n");
    }
    sock_len = sizeof(struct sockaddr_in);

    // Accept incoming connections
    //
    while(1){

        printf("... ");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sock_len);
        if(client_socket == -1){
            printf("Error accepting socket connection. \n");
            return(EXIT_FAILURE);
        }else{
            printf("Sucessfully connected to client. \n");
        }

        // Receive the file from the client
        //
        char receive_buffer[1024];
        int bytes_received;
        int total_bytes = 0;

        // Open a new file to store the data from the client
        //
        FILE *fp;
        fp = fopen("temp.zip","w");
        if(fp == NULL){
          printf("Error creating file.\n");
          return(EXIT_FAILURE);
        }

        // Store the data from the client
        //
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

                // Wait for the client again
                //
                usleep(1000000);
                while(1){
                    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sock_len);
                    if(client_socket == -1){
                        printf("Error accepting socket connection. \n");
                        return(EXIT_FAILURE);
                    }else{
                        printf("Sucessfully connected to client. \n");
                        break;
                    }
                }
                break;
            }
        }

        fclose(fp);

        // Unzip the file and send back to the client
        //
        char unzip[50];
        strcpy(unzip, "unzip -p temp.zip > temp.txt");
        system(unzip);

        // Open a new pointer to the newly created file
        //
        FILE *new;
        new = fopen("temp.txt","r");
        if(new == NULL){
          printf("Error opening file.\n");
          return(EXIT_FAILURE);
        }

        // Send the upzipped file back to the client
        //
        char send_buffer[1024];
        int bytes_sent = 0;
        int nread;
        total_bytes = 0;
        while(!feof(new)){
            //memset(send_buffer, 0, sizeof(send_buffer));
            bzero(send_buffer, 1024);
            nread = fread(send_buffer, 1, 1024, new);
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
    }

    return(EXIT_SUCCESS);
}
