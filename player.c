#include <stdio.h>
#include <string.h>
#include "player.h"
#include <ctype.h>
#include <stdlib.h>

player *makeplayer()
{
    player *person=(player *)calloc(1,sizeof(player));
    person->fd=-1;
    person->x=-1;
    person->y=-1;
    person->d=' ';
    person->reg=0;
    person->next=NULL;
    return person; 
}
player *addplayer(player *head, player *new)
{
    // add new players from bottom
    new->next=head;
    return new;
}
void initialize(player *person, int x, int y, char name[20], char d)
{
    person->x=x;
    person->y=y;
    strcpy(person->name,name);
    person->name[19]='\0'; // ensure null termination
    person->d=d;
    person->reg=1;
    for (int i=0; i<5; i++)
    {
        person->hit[i]=0;
    }
    if (d=='|')
    {
        int starty=y-2;
        for (int i=0; i<5; i++)
        {
            person->shipx[i]=x;
            person->shipy[i]=starty;
            starty++;
        }
    }
    else if (d=='-')
    {
        int startx=x-2;
        for (int j=0; j<5; j++)
        {
            person->shipx[j]=startx;
            person->shipy[j]=y;
            startx++;
        }
    }
}

player *deleteplayer(player *head, player *person)
{
  player *p=NULL;
  player *pnext=NULL;
  int fd=person->fd;
  if (head==NULL) // if list has nothing
  {
    return NULL;
  }
  p=head;
  pnext=head->next;
  if (head->fd==fd) // if head is the one to delete
  {
    free(head);
    return pnext;
  }
  while (pnext!=NULL) // search for player to delete
  {
    if (fd==pnext->fd)
    {
        p->next=pnext->next;
        free(pnext);
        break;
    }
    p=p->next;
    pnext=pnext->next;
  }
  return head;
}

int validname(char *name)
{
    int i=0;
    while (name[i]!='\0')
    {
        if (!(isalnum(name[i]))&&name[i]!='-')
        {
            return 1;
        }
        i++;
    }
    return 0;
}
int samename(player *head,char *name)
{
    player *p=head;
    while (p!=NULL)
    {
        if (strcmp(p->name,name)==0)
        {
            return 1;
        }
        p=p->next;
    }
    return 0;
}