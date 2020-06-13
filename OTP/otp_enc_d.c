//Author: Onur Sasmaz
//Sources: Lecture Slides
//This program will run in the background as a daemon.
//Its function is to perform the actual encoding. This program will listen on a particular port/socket, assigned when it is first ran.
//When a connection is made, encryption happens.
//Credits Office Hours of Justin Goins, Chris Pomeroy, Spencer Moss and Erich Kramer.
//Edited with Xcode.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define ARR_SIZE 80000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    char key_buffer[ARR_SIZE], text_buffer[ARR_SIZE], ciphertext[ARR_SIZE]; //buffers
    int portNumber, listenSocketFD,establishedConnectionFD;
    int charsRead = 0, charsWritten = 0;
    struct sockaddr_in serverAddress;
    socklen_t sizeOfClientInfo;
    pid_t pid;
    int text_size, key_size,receive_size;
    
    
    //check if there is enough argument
    if(argc < 2)
    {
        error("otp_enc_d ERROR: wrong terminal command");
    }
    portNumber = atoi(argv[1]);
    
   
    
    // Set up the address struct for this process (the server)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocketFD < 0)
    {
        error("otp_enc_d ERROR opening socket");
    }
    
    //2.Bind the socket to a port number with bind()
    // Connect socket to port
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("otp_enc_d ERROR on binding");
    }
    
    //3.Start listening for connections on that socket/port with listen()
    // Flip the socket on - it can now receive up to 5 connections
    if(listen(listenSocketFD, 5) == -1)
    {
        error("Error on listen");
    }
    
    
    while(1)
    {
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(serverAddress);
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&serverAddress, &sizeOfClientInfo); // Accept
        if(establishedConnectionFD < 0)
        {
            error("otp_enc_d ERROR reading from socket");
        }
        
        pid = fork();
        if(pid < 0)
        {
            error("otp_enc_d ERROR forking");
        }
        
        else if(pid == 0)
        {
            
            //---------------------HANDSHAKE-------------------///
            // Get the message from the client
            char handshake[4];
            memset(handshake, '\0', sizeof(handshake)); //clear the buffer
            charsRead = recv(establishedConnectionFD, handshake, sizeof(handshake) - 1, 0);
            if (charsRead < 0){ error("otp_enc_d ERROR reading from socket"); }
            if(strcmp(handshake, "enc") != 0)
            {
                fprintf(stderr, "otp_enc_d: ERROR could not contact client on port %d\n", portNumber);
                exit(2);
                
            }
            
            // Send a Success message back to the client
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_enc_d ERROR writing to socket");
            }
            //---------------------END- HANDSHAKE-------------------///
            
            //---------------------RECEIVE MESSAGE/KEY SIZE-------------------///
            receive_size = recv(establishedConnectionFD, &text_size, sizeof(text_size),0);
            if(receive_size< 0){ error("ERROR: receiving text size");}
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_enc_d ERROR writing to socket");
            }
            
            receive_size = recv(establishedConnectionFD, &key_size, sizeof(key_size),0);
            if(receive_size< 0){ error("ERROR: receiving text size");}
            //---------------------END - RECEIVE MESSAGE/KEY SIZE-------------------///
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_enc_d ERROR writing to socket");
            }
            
            //---------------------RECEIVE MESSAGE/KEY -------------------///
            //receive text
            memset(text_buffer, '\0', sizeof(text_buffer));
            receive_size = recv(establishedConnectionFD, text_buffer, sizeof(text_buffer),0);
            if (receive_size < 0) { error("otp_enc_d: Error sending message to server\n");}
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_enc_d ERROR writing to socket");
            }
            
            
            //receive key
            memset(key_buffer, '\0', sizeof(key_buffer));
            receive_size = recv(establishedConnectionFD, key_buffer, sizeof(key_buffer),0);
//            printf("HELLO5");fflush(stdout);
            if (receive_size < 0) { error("otp_enc_d: Error sending message to server\n");}
            //---------------------END MESSAGE/KEY -------------------///
            
            
            //---------------------ENCRYPT - KEY -------------------///
            int i, chris = 0;
            for(i=0; i<key_size - 1; i++)
            {
                chris++;
                if(text_buffer[i] == ' ')
                {
                    text_buffer[i] = 91;
                }
                
                if(key_buffer[i] == ' ')
                {
                    key_buffer[i] = 91;
                }
                
                //Add text and key together and mod 27 the result
                ciphertext[i] = (char)((text_buffer[i] + key_buffer[i] - 130) % 27 + 65);
                // Replace 91s with space chars
                if (ciphertext[i] == 91) {
                    ciphertext[i] = ' ';
                }
                
            }
            //---------------------END ENCRYPT - KEY -------------------///

            //---------------------SEND CHIPERTEXT -------------------///
            //send chipertext to client
            
            charsWritten = send(establishedConnectionFD, ciphertext, sizeof(ciphertext), 0);
            if (charsWritten < 0)
            {
                error("otp_enc_d ERROR sending chipertext to client");
            }
            //---------------------END SEND CHIPERTEXT -------------------///

            close(establishedConnectionFD);
            exit(0);
 
        }//pid==0 end
        else
        {
            close(establishedConnectionFD);
        }
        
        
    }//while loop ends
    close(listenSocketFD);
    return 0;
}
