//
//  buildroom.c
//  Xcode
//  Class: CS344
//  Created by Onur Sasmaz on 2/9/19.
//  Shoutout the all the TA's who helped.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


//GLOBAL VARIABLES
#define MAX_ROOM_NUM 7
#define MIN_CONNECTION 3
#define MAX_CONNECTION 6
#define TOT_ROOM_NUM 10
#define MAX_NAME_C 8

char* room_names[TOT_ROOM_NUM] = {
    "Room_1",
    "Room_2",
    "Room_3",
    "Room_4",
    "Room_5",
    "Room_6",
    "Room_7",
    "Room_8",
    "Room_9",
    "Room_10"
};

//3 Possible room types
char* room_type[MIN_CONNECTION] = {
    "START_ROOM",
    "MID_ROOM",
    "END_ROOM"
};

//Contains room information
struct Room
{
    int room_id;
    char room_name[MAX_NAME_C];
    char* room_type;
    struct Room* out_connections[MAX_CONNECTION];
    int num_o_connections;
};
typedef struct Room Room;
Room room[MAX_ROOM_NUM]; //create the room globaly

// Helper function to check if room name used before. If not return 0.
int room_check(char *r_name){
    int i; //C89 style for loop
    
    for(i=0; i< MAX_ROOM_NUM; i++){ //compare with strcmp
        if(strcmp(room[i].room_name, r_name)==0){
            return 1;
        }
    }
    return 0;
}

//These Functions: 2.2 Program Outlining in Program 2
// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
int IsGraphFull(){
    int full_flag = 1; //flag for room connections
    int i; //C89 style for loop
    for (i=0; i<MAX_ROOM_NUM; i++){
        if(room[i].num_o_connections < 3){
            full_flag = 0;//check if each room has 3 connection. if its flag=0
        }
    }
    return full_flag;
}


// Returns a random Room, does NOT validate if connection can be added
Room* GetRandomRoom()
{
    return &room[rand()%MAX_ROOM_NUM];
}


// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
int  CanAddConnectionFrom(Room *x)
{
    if(x->num_o_connections < 6){ return 1; }
    else{ return 0; }
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
int ConnectionAlreadyExists(Room* x,Room* y){
    int i;
    for(i=0; i<x->num_o_connections; i++){
        if(x->out_connections[i]->room_name == y->room_name){
            return 1;
        }
    }
    return 0;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(Room* x, Room* y)
{
        x->out_connections[x->num_o_connections] = y;
        x->num_o_connections++;
}
    
//Check if the rooms have the same name
int IsSameRoom(Room* x, Room* y)
{
    if(x->room_name == y->room_name){ return 1; }
    else { return 0; }
}


void AddRandomConnection()
{
    Room *A;  // Maybe a struct, maybe global arrays of ints
    Room *B;
    while(1)
    {
        A = GetRandomRoom();
        
        if (CanAddConnectionFrom(A) == 1)
            break;
    }
    
    do
    {
        B = GetRandomRoom();
    }
    while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);
    
    ConnectRoom(A, B);  // TODO: Add this connection to the real variables,
    ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
    
}
//checks the rooms are full. If not call the function to add rooms.
void check_add(){
    while (IsGraphFull() == 0){ AddRandomConnection(); }
}


void create_rooms(){
    int i,rand_temp; //c-style
    int used_flag = 1;
    for(i=0; i<MAX_ROOM_NUM; i++){
        while(used_flag){
            rand_temp = rand() % TOT_ROOM_NUM; //generate random in range 0 to 10
            if((room_check(room_names[rand_temp])==0)){ used_flag =0;}
        }
        strcpy(room[i].room_name,room_names[rand_temp]); //copy random num to room
        room[i].num_o_connections = 0; //assign connection to 0
        room[i].room_id = i; //assign room number
        if(i==0){
            room[i].room_type = room_type[0]; //if begging of array assign START_ROOM
        }
        else if(i == 6){
            room[i].room_type = room_type[2];//if end of array assign END_ROOM
        }
        else{
            room[i].room_type = room_type[1];//if else assign MID_ROOM
        }
        used_flag=1;
    }
}

void create_dir(){
    //Make directory
    char first_dir[20]; // Holds the name of the newest dir that contains prefix
    memset(first_dir, '\0', sizeof(first_dir));
    sprintf(first_dir, "sasmazo.rooms.%d", getpid()); //create file name containing <username>
    mkdir(first_dir,0755); //create directory w/ mkdir
    chdir(first_dir); //change working directory first_dir
    
    
    FILE* room_file = NULL; // Holds the file we're starting in
                            //source: https://www.geeksforgeeks.org/data-type-file-c/
    
    int i,j; //CS89
    for(i=0; i<MAX_ROOM_NUM; i++){
        room_file = fopen(room[i].room_name, "w"); //pointing room_file to write to it
        fprintf(room_file, "ROOM NAME: %s\n", room[i].room_name); //write room name in file
        //  fprintf(stdout, "ROOM NAME: %s\n", room[i].room_name);
        
        //add room connections. to till num_o_connections
        for(j=0; j<room[i].num_o_connections; j++){
            fprintf(room_file, "CONNECTION %d: %s\n", (j+1), room[i].out_connections[j]->room_name);
            // fprintf(stdout, "CONNECTION %d: %s\n", (j+1), room[i].out_connections[j]->room_name);
        }
        fprintf(room_file, "ROOM TYPE: %s\n", room[i].room_type);//write room type in file
        // fprintf(stdout, "ROOM TYPE: %s\n", room[i].room_type);
    }
    fclose(room_file);
}

//Helper function to check rooms created
void printmew(){
    int i,j;
    for(i=0; i<MAX_ROOM_NUM; i++){
        fprintf(stdout, "ROOM NAME: %s\n", room[i].room_name);
        for(j=0; j<room[i].num_o_connections; j++){
            fprintf(stdout, "CONNECTION %d: %s\n", (j+1), room[i].out_connections[j]->room_name);
        }
        fprintf(stdout, "ROOM TYPE: %s\n\n", room[i].room_type);
    }
}

//Call the functions
int main(){
    srand(time(NULL));
    create_rooms();
    check_add();
    create_dir();
//    printmew(); print the rooms on terminal
}
