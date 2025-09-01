#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "player.h"
#include "board.h"

// check who got bombed
int bombcheck(player *player, int x, int y)
{
    for (int i=0; i<5; i++)
    {
        if (player->shipx[i]==x&&player->shipy[i]==y) // if coordinates are the same
        {
            player->hit[i]=1; // indicate ship got hit
            return 1;
        }
    }
    return 0;
}
// check if ship lost
int lose(player *player)
{
    int count=0;
    for (int i=0; i<5; i++)
    {
        if (player->hit[i]==1)
        {
            count++;
        }
    }
    if (count==5)
    {
        return 1;
    }
    return 0;
}

player *broadcast(player *head, char *message)
{
    ssize_t msgstat;
    player *p=head;
    player *next=NULL;
    if (head==NULL)
    {
        return NULL;
    }
    while (p!=NULL)
    {
        next=p->next;
        if (p->reg==1) // if valid client
        {
            msgstat=write(p->fd,message,strlen(message));
            if (msgstat==-1) // if message didn't send
            {
                fprintf(stderr,"error sending %s message to client",message);
                close(p->fd);
                p->fd=-2;
                p->reg=0;
            }
        }
        p=next;
    }
    return head;
}

player *freeall(player *head)
{
    player *p=NULL;
    player *h=NULL;
    p=head;
    if (head==NULL)
    {
        return NULL;
    }
    while(p!=NULL)
    {
        if (p->fd>=0)
        {
            close(p->fd);
        }
        h=p->next;
        free(p);
        p=h;
    }
    return NULL;
}
player *disconnect(char broadmsg[], player *clientbomb, player *head, size_t size)
{
    memset(broadmsg, 0, size);
    snprintf(broadmsg,size,"GG %.20s\n",clientbomb->name); // form GG broadcast message
    broadcast(head,broadmsg); // send message 
    close(clientbomb->fd); 
    head=deleteplayer(head, clientbomb);
    return head;
}
player *sweepup(player *head)
{
    player *p=head;
    if (head==NULL)
    {
        return NULL; 
    }
    while (p!=NULL)
    {
        if (p->fd==-2)
        {
            head=deleteplayer(head,p);
        }
        p=p->next;
    }
    return head;
}
