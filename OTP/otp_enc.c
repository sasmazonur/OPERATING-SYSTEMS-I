//Author: Onur Sasmaz
//Client
//This program connects to otp_enc_d, and asks it to perform a one-time pad style encryption.
//By itself, otp_enc doesn’t do the encryption - otp_enc_d does. The syntax of otp_enc is as follows:
//Sources: Lecture Slides
//Credits Office Hours of Justin Goins, Chris Pomeroy, Spencer Moss and Erich Kramer.
//Edited with Xcode.
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<netdb.h>
#include <ctype.h>

//define array size
#define ARR_SIZE 80000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]){
    int portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    int listenSocketFD, charsRead;
    int user_text_size,key_text_size;
    char buffer[ARR_SIZE], key_buffer[ARR_SIZE], ciphertext[ARR_SIZE];
    
    if (argc < 4) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(0); } // Check usage & args
    
    int user_text, key_text;
    
     //---------------------OPEN FILE-------------------///
    //open the file. If bad file throw error on the screen
    user_text = open(argv[1], O_RDONLY);
    if(user_text < 0){
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]); exit(1);
    }
    //read the file in buffer
    user_text_size = read(user_text,buffer, ARR_SIZE);
    if(user_text_size < 0){ fprintf(stderr, "Error: cannot read key file '%s'\n", argv[1]); exit(1); }
    //open the second file. If bad file throw error on the screen
    key_text = open(argv[2], O_RDONLY);
    if(key_text < 0){
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[2]); exit(1);
    }
    key_text_size = read(key_text,key_buffer, ARR_SIZE);
    if(user_text_size < 0){ fprintf(stderr, "Error: cannot read key file '%s'\n", argv[1]); exit(1);}
     //---------------------END OF OPEN FILE-------------------///
    
     //---------------------INPUT CHECK-------------------///
    //char check
    int i;
    for (i = 0; i < user_text_size - 1; i++)
    {
        //check if input between A-Z and [space]. source ascii: A=65 Z=90 [space]=32
        if (buffer[i] > 90 || (buffer[i] < 65 && buffer[i] != 32))
        {
            fprintf(stderr, "Error: Bad File '%s'\n", argv[1]); exit(1);
        }

    }
    
    for (i = 0; i < key_text_size - 1; i++)
    {
        if (key_buffer[i] > 90 || (key_buffer[i] < 65 && key_buffer[i] != 32))
        {
            fprintf(stderr, "Error: Bad File '%s'\n", argv[2]); exit(1);
        }
    }
    
    //check if key is big enough. If not throw error
    if(user_text_size > key_text_size)
    {
        error("otp_enc ERROR key is too short");
    }
    //---------------------END OF INPUT CHECK-------------------///
    
    //closed the open file
    close(user_text);
    close(key_text);
    
    

    // Set up the address struct for this process (the server)
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "OTP_ENC: ERROR, no such host\n"); exit(1); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocketFD < 0)
    {
        error("otp_enc ERROR opening socket");
    }
    
    // Connect to server
    if (connect(listenSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
    {
        error("otp_enc: ERROR connecting");
    }
    
    //---------------------HANDSHAKE-------------------///
    
    char handshake_back[ARR_SIZE];
    memset(handshake_back, '\0', sizeof(handshake_back)); //clear the buffer
    int r;
    //send handshake "enc"
    send(listenSocketFD, "enc", 4, 0);
    //check if handsake succesfull
    r = recv(listenSocketFD, handshake_back, 8,0);
    if (r < 0) {
        error("otp_enc: ERROR reading handshake");
    }
    if(strcmp(handshake_back, "success") != 0)
    {
        fprintf(stderr, "otp_enc: ERROR could not contact client on port %d\n", portNumber);
        exit(2);
    }
    //---------------------END- HANDSHAKE-------------------///

    
    //---------------------SEND MESSAGE/KEY SIZE-------------------///
    int send_size;
    memset(handshake_back, '\0', sizeof(handshake_back)); //clear the buffer
    r = recv(listenSocketFD, handshake_back, sizeof(handshake_back) - 1,0); //check if handshake successful
    if (r < 0) {
        error("otp_enc: ERROR reading handshake");
    }
    if(strcmp(handshake_back, "success") != 0)
    {
        fprintf(stderr, "otp_enc: ERROR could not contact client on port %d\n", portNumber);
        exit(2);
    }
    //send the size of the user input
    send_size = send(listenSocketFD, &user_text_size, sizeof(user_text_size), 0);
    if(send_size < 0){ error("ERROR Sending key size");}
    
    //check if still good with server
    memset(handshake_back, '\0', sizeof(handshake_back));//clear the buffer
    r = recv(listenSocketFD, handshake_back, sizeof(handshake_back) - 1,0); //check if handshake successful
    if (r < 0) {
        error("otp_enc: ERROR reading handshake");
    }
    if(strcmp(handshake_back, "success") != 0)
    {
        fprintf(stderr, "otp_enc: ERROR could not contact client on port %d\n", portNumber);
        exit(2);
    }
    
    
    //---------------------END -MESSAGE/KEY SIZE-------------------///
    
    
    //---------------------SEND MESSAGE/KEY-------------------///
    
    // Send message to server
    send_size = send(listenSocketFD, buffer, strlen(buffer), 0); // Write to the server
    if (send_size < 0) { error("otp_enc: Error sending message to server\n");}
    
    memset(handshake_back, '\0', sizeof(handshake_back)); //clear the buffer
    r = recv(listenSocketFD, handshake_back, sizeof(handshake_back) - 1,0);
    if (r < 0) {
        error("otp_enc: ERROR reading handshake");
    }
    if(strcmp(handshake_back, "success") != 0)
    {
        fprintf(stderr, "otp_enc: ERROR could not contact client on port %d\n", portNumber);
        exit(2);
    }
    
    // Send key to server
    send_size = send(listenSocketFD, key_buffer, strlen(key_buffer), 0); // Write to the server
    if (send_size < 0) { error("otp_enc: Error sending key to server\n");}
    
    
    //---------------------END  MESSAGE/KEY-------------------///

    
    // Get return encyrpted from server
    memset(ciphertext, '\0', sizeof(ciphertext)); // Clear out the buffer again
    charsRead = recv(listenSocketFD, ciphertext, sizeof(ciphertext) - 1, 0); // Read data from the socket, leaving \0 at end
    
    //place '\0' end of file
    ciphertext[sizeof(ciphertext) - 1] = '\0';
    
    if (charsRead < 0) {
        error("otp_enc: ERROR reading from socket");
    }
    
    //write cripted text in to file
    fprintf(stdout,"%s\n", ciphertext);
    
    close(listenSocketFD); //close the socket
    return 0;
}

