//
//  adventure.c
//  Xcode
//  Class: CS344
//  Created by Onur Sasmaz on 2/10/19.
//  Shoutout the all the TA's who helped.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //source:https://www.youtube.com/watch?v=GXXE42bkqQk&t=1333s

//GLOBAL VARIABLES
#define MAX_ROOM_NUM 7
#define MIN_CONNECTION 3
#define MAX_CONNECTION 6
#define TOT_ROOM_NUM 10
#define MAX_NAME_C 8

struct Room {
        int room_id;
        char room_name[20];
        char room_type[20];
        //struct Room* out_connections[MAX_CONNECTION];
        char out_connections[6][9]; //set before alloc so dont need to deal with Room*
        int num_o_connections;
    
};

typedef struct Room Room;
Room room[MAX_ROOM_NUM];

//Holds the start and end location
//num_rooms_visited  is a step counter
int start_location, end_location, num_rooms_visited = 0;

char newestDirName[256]; // global Holds the name of the newest dir that contains prefix
char* rooms_visited[60]; //global holds the path of the player


//Example code from 2.4
//Get the newest file starting with "sasmazo.rooms."
void get_file(){
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "sasmazo.rooms."; // Prefix we're looking for


    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL){ // Check each entry in dir
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL){ // If entry has prefix
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry
                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    closedir(dirToCheck); // Close the directory we opened
}


//prints the time on the screen
//source:https://stackoverflow.com/questions/13658756/example-of-tm-use
void* write_time(){
    
    FILE *time_file;

    time_file = fopen("currentTime.txt", "w"); //write (-w) currentTime.txt
    if(time_file == NULL) //Error handling
    {
        printf("Error!");
        exit(1);
    }
    pthread_mutex_lock(&mutex); //locks the mutex
    
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    
    pthread_mutex_unlock(&mutex); //unlocks the mutex

    strftime (buffer,80,"\n%I:%M%p, %A, %B %C, %G\n",timeinfo);
    fprintf(time_file,"%s", buffer);
    printf("%s\n", buffer);
    fclose(time_file);
    pthread_exit(NULL); //close thread
}



//read each line from file
//
//source:https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
//source:https://www.programiz.com/c-programming/examples/write-file
//sourcehttps://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program/17683417

void read_each(){
    DIR *d;
    FILE* read_file;
    struct dirent *dir;
    char *line = malloc(64);
    d = opendir(newestDirName);

    int i;
    char *end;


    int rooms_read=0;
    chdir(newestDirName);
    if (d)
    {
        
        while ((dir = readdir(d)) != NULL)
        {
            if (!strcmp (dir->d_name, ".")) //if see . in file ignore
                continue;
            if (!strcmp (dir->d_name, "..")) //if see .. in file ignore
                continue;

            read_file = fopen(dir->d_name, "r"); //open the files
            
            while(fgets(line, 64, (FILE*) read_file)!=NULL) {
                if(strstr(line, "ROOM NAME:")!=NULL){
                    if((end = strchr(line, '\n')))
                        *end = '\0'; //remove new line
                    strcpy(room[rooms_read].room_name, line+11); //Get the name after 10th character
                    room[rooms_read].num_o_connections=0;
                }
                //compare line with "CONNECTION" if exist cpy in struct
                if(strstr(line, "CONNECTION ")!=NULL){
                    if((end = strchr(line, '\n')))
                        *end = '\0'; //remove new line
                    strcpy(room[rooms_read].out_connections[room[rooms_read].num_o_connections], line+14 );
                    room[rooms_read].num_o_connections++; //Get the name after 14th character and increment connections
                }
                //compare line with "ROOM TYPE:" if exist cpy in struct
                if(strstr(line, "ROOM TYPE:")!=NULL){
                    if((end = strchr(line, '\n')))
                        *end = '\0'; //remove new line
                    strcpy(room[rooms_read].room_type, line + 11); //Get the name after 10th character
                    if(strstr(room[rooms_read].room_type, "START_ROOM") != NULL){
                        start_location = rooms_read; //if start room save the place in global val
                    }
                    else if(strstr(room[rooms_read].room_type, "END_ROOM") != NULL){
                        end_location = rooms_read; //if end room save the place in global val
                    }
                }
            }
            rooms_read++;
        }//end of while
    } //end of if(d)
        fclose(read_file); //close file
        chdir(".."); //exit from directory
}


//Game happens here
void Game(){
    struct Room * p_position; //create a temp room that holds player's position
    p_position = &room[start_location]; //point the start_room
    int player_won = 0; //keep track if player won
    char *buffer = NULL;
    size_t bufsize = 32;
    char *long_line = NULL;

    int i;
    //do while player not won
    while(player_won == 0){
        printf("\nCURRENT LOCATION: %s\n", p_position->room_name);//Print the start location saved in global variable
        printf("POSSIBLE CONNECTIONS: ");
        for(i=0; i<p_position->num_o_connections;i++){
            
            printf("%s", p_position->out_connections[i]);//Print the start location saved in global variable
            if(i <(p_position->num_o_connections-1)) {
                printf(", "); //print , betweeen each connect room name
            }
            printf("."); //print . after last connected room
        }
        //Ask user for input
        printf("\nWHERE TO? >");
        getline(&buffer,&bufsize,stdin); //source:https://c-for-dummies.com/blog/?p=1112

        if((long_line = strchr(buffer, '\n')))
            *long_line = '\0'; //remove new line

        //if input not equal the time do this function
        if(strcmp(buffer, "time")!=0){
            int z;
            for(z=0; z<p_position->num_o_connections; z++){
                if(strcmp(p_position->out_connections[z], buffer)==0){
                    int q;
                    //look room if matching name save the location on temp file
                    for(q=0; q<MAX_ROOM_NUM; q++){
                        if(strcmp(buffer, room[q].room_name)==0){
                            p_position = &room[q];
                        }
                    }
                    rooms_visited[num_rooms_visited] = p_position->room_name; //copy the room names in global room_visited
                    num_rooms_visited++; //increment the number of steps
                }
                if(strcmp(p_position->room_type,"END_ROOM")==0){
                    player_won = 1; //Player won is true. So, stop while loop
                   
                }
            }
            //Check if player won end print the win message
            if(player_won == 1){
                int t;
                printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",num_rooms_visited ); //prints the number of attemps from global variable
                for(t=0; t<num_rooms_visited; t++){
                    printf("%s\n", rooms_visited[t]); //print the rooms visited from array
                }
            }
        }
        //if user input time open time function
        else if(strcmp(buffer, "time")==0){
            pthread_t id1; // declare thread
            pthread_create(&id1, NULL,write_time, NULL );
            pthread_join(id1, NULL);
            //write_time();
        }
        //else user bad input handle
        else{
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        }
    }
    free(buffer); //free for next getline argument
}
void printmew(){
    int i,j;
    for(i=0; i<MAX_ROOM_NUM; i++){
        fprintf(stdout, "ROOM NAME: %s\n", room[i].room_name);
        for(j=0; j<room[i].num_o_connections; j++){
            fprintf(stdout, "CONNECTION %d: %s\n", (j+1), room[i].out_connections[j]);
        }
        fprintf(stdout, "ROOM TYPE: %s\n\n", room[i].room_type);
    }
}

int main(){
    get_file();
    read_each();
    // printmew(); enable if you want to see room layout
    Game();
}
