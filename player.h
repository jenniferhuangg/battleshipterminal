#ifndef PLAYER_H
#define PLAYER_H
#include <stdio.h>
#include <string.h>

// struct
typedef struct servplayer
{
    char name[20]; // name of player
    int x; //x coordinate of middle ship
    int y; //y coordinate of middle ship
    char d; // position of ship
    int shipx[5]; // to store x coordinates of ship
    int shipy[5]; // to store y coordinates of ship
    int fd; // player socket
    int hit[5]; // to track how much of player's ship got hit
    int reg; // registered
    char inputbuf[1024]; // buffer to accumulate input
    int used; // to store number of used input
    struct servplayer *next; // next player for linked list

} player;

//initalize values into player
void initialize(player *person, int x, int y, char name[20], char d);

// add player to linked list
player *addplayer(player *head, player *new);

// make a new player
player *makeplayer();

// delete player
player *deleteplayer(player *head, player *person);

//valid name
int validname(char *name);

//check if names are the same
int samename(player *head,char *name);


#endif
