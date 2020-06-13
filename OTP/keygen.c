//Author: Onur Sasmaz
//Generate random text between A-Z and spaces.
//Edited with Xcode
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

int main(int argc, char* argv[]) {
    int key_len, rand_num, key_char;
    
    //check the argumant count. If not met throw error
    if(argc < 2)
    {
        printf("Error. Ex: ./keygen <num>");
        
    }
    
    srand(time(NULL));
    
    //get the key lenght from the user
    key_len = atoi(argv[1]);
    
    //addapred from:https://codereview.stackexchange.com/questions/138703/simple-random-password-generator
    int i;
    // only accept the 26 letters of alphabet and the "space" character as valid for encrypting and decrypting
    for(i = 0; i<key_len; i++)
    {
        rand_num = rand() % 27;
        if(rand_num < 26){
        // 65 = A source:ASCII
        // 65+25 = 90, 90 = Z
        key_char = 65 + rand_num;
        printf("%c", key_char);
    }
    //Add Space
    //Do not create spaces every five characters, as has been historically done.
    else{
        printf(" ");
    }
    }
    printf("\n");
    return 0;

}
