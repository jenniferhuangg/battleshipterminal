#ifndef BOARD_H
#define BOARD_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "player.h"

// check who got bombed
int bombcheck(player *player, int x, int y);

// check if ship lost
int lose(player *player);

// send messages to all clients
player *broadcast(player *head, char *message);

//free all clients
player *freeall(player *head);

// disconnect player 
player *disconnect(char broadmsg[], player *clientbomb, player *head, size_t size);

// delete players after indicated
player *sweepup(player *head);



#endif