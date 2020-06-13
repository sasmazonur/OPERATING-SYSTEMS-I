//Author: Onur Sasmaz
//This program performs exactly like otp_enc_d, in syntax and usage.
//otp_dec_d will decrypt ciphertext it is given, using the passed-in ciphertext and key.
//Thus, it returns plaintext again to otp_dec.
//Sources: Lecture Slides
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
    char key_buffer[ARR_SIZE], crip_buffer[ARR_SIZE], user_text[ARR_SIZE]; // buffers
    int portNumber, listenSocketFD,establishedConnectionFD;
    int charsRead = 0, charsWritten = 0;
    struct sockaddr_in serverAddress;
    socklen_t sizeOfClientInfo;
    pid_t pid;//fork
    int text_size, key_size,receive_size;
    
    
    //check if there is enough argument
    if(argc < 2)
    {
        error("otp_dec_d ERROR: wrong terminal command");
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
        error("otp_dec_d ERROR opening socket");
    }
    
    //2.Bind the socket to a port number with bind()
    // Connect socket to port
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("otp_dec_d ERROR on binding");
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
            error("otp_dec_d ERROR reading from socket");
        }
        
        pid = fork();
        if(pid < 0)
        {
            error("otp_dec_d ERROR forking");
        }
        
        else if(pid == 0)
        {
            
            //---------------------HANDSHAKE-------------------///
            // Get the message from the client
            char handshake[4];
            memset(handshake, '\0', sizeof(handshake));//clear the buffer
            charsRead = recv(establishedConnectionFD, handshake, sizeof(handshake) - 1, 0); //recv message
            if (charsRead < 0){ error("otp_dec_d ERROR reading from socket"); }
            if(strcmp(handshake, "enc") != 0)
            {
                fprintf(stderr, "otp_dec_d: ERROR could not contact client on port %d\n", portNumber);
                break;
                
            }
            
            // Send a Success message back to the client
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_dec_d ERROR writing to socket");
            }
            //---------------------END- HANDSHAKE-------------------///
            
            //---------------------RECEIVE MESSAGE/KEY SIZE-------------------///
            
            
            receive_size = recv(establishedConnectionFD, &text_size, sizeof(text_size),0);
            if(receive_size< 0){ error("ERROR: receiving text size");}
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_dec_d ERROR writing to socket");
            }
            
            receive_size = recv(establishedConnectionFD, &key_size, sizeof(key_size),0);
            if(receive_size< 0){ error("ERROR: receiving text size");}
            
            //---------------------END - RECEIVE MESSAGE/KEY SIZE-------------------///
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_dec_d ERROR writing to socket");
            }
            
            //---------------------RECEIVE MESSAGE/KEY -------------------///
            //receive text
            memset(crip_buffer, '\0', sizeof(crip_buffer));//clear the buffer
            receive_size = recv(establishedConnectionFD, crip_buffer, sizeof(crip_buffer),0);
            //            printf("HELLO4");fflush(stdout);
            if (receive_size < 0) { error("otp_dec_d: Error sending message to server\n");}
            
            charsRead = send(establishedConnectionFD, "success", 8, 0);
            if (charsRead < 0)
            {
                error("otp_dec_d ERROR writing to socket");
            }
            
            
            //receive key
            memset(key_buffer, '\0', sizeof(key_buffer));//clear the buffer
            receive_size = recv(establishedConnectionFD, key_buffer, sizeof(key_buffer),0);
            if (receive_size < 0) { error("otp_dec_d: Error sending message to server\n");}
            //---------------------END MESSAGE/KEY -------------------///
            
            //---------------------DECRYPT - KEY -------------------///
            // Decode cipher
            int i, hold_val;
            for (i = 0; i < sizeof(crip_buffer) - 1; i++) {
                // Replace space chars with ASCII 91 for proper modulus
                if(key_buffer[i] == 32) key_buffer[i] = 91;
                if(crip_buffer[i] == 32) crip_buffer[i] = 91;
                
                // Modulus and account for negative numbers
                hold_val = (crip_buffer[i] - key_buffer[i]) % 27;
                if(hold_val < 0) hold_val += 27;
                
                // Add 65 to reach uppercase alphabet chars
                user_text[i] = (char)(hold_val + 65);
                
                // Replace all 91s with space chars
                if (user_text[i] == 91) {
                    user_text[i] = ' ';
                }
            }
            //---------------------END DECRYPT - KEY -------------------///
            
            
            //---------------------SEND CHIPERTEXT -------------------///
            //send chipertext to client
            
            charsWritten = send(establishedConnectionFD, user_text, sizeof(user_text), 0);
            if (charsWritten < 0)
            {
                error("otp_dec_d ERROR sending chipertext to client");
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

