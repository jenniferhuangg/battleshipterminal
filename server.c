#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "player.h"
#include "board.h"
#include <ctype.h>
#define MY_PORT 54175

// ignore SIGPIPE signals/when trying to write() to socket closed by client
void ignoresigpipe(void)
{
  struct sigaction myaction;
  memset(&myaction, 0, sizeof(myaction)); 
  myaction.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &myaction, NULL);
}

int main(int argc,char *argv[])
{
    char *end; // collect first non convert number part of string (checks if it is '\0' as it should be)
    if (argc!=2) // if more then 2 arguments
    {
        fprintf(stderr,"Wrong # of arguments\n");
        return 1;
    }
    int port=strtol(argv[1],&end,10); // convert port to a decimal 
    if (*end!='\0'||port<1024||port>65535) // check if port # correct 
    {
        fprintf(stderr,"Service name %s does not exist\n",argv[1]);
        return 1;
    }
    fd_set readfds; // file descriptors set to read
    player *head=NULL;
    player *clientaccpt=NULL; // to loop for clients to accept
    player *clientact=NULL; // to loop for client actions
    player *clientbomb=NULL; // to loop for client bombing
    // to keep loop going in case that client gets deleted
    player *next=NULL;
    player *nextbomb=NULL; 
    char name[21];
    int x;
    int y;
    char d;
    char broadmsg[1024]; // message to broadcast to clients
    int hitcount=0; // keep track of how many hits occurred
    ssize_t msgstat; // status of messages sent to client 
    char line[1024];
    char *newline;
    socklen_t saddrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in address;
    ignoresigpipe(); // to close bad clients
    int servfd=socket(AF_INET, SOCK_STREAM, 0); // create the socket
    if (servfd==-1)
    {
        fprintf(stderr,"error creating socket\n");
        return 1;
    }
    int maxfd=servfd; // set maxfd
    // set up socket to accept connections on all IP addresses on the machine
    memset(&address, 0, sizeof(struct sockaddr_in)); // clear out sockaddr_in struct
    // set up socket to specified port 
    address.sin_family = AF_INET;
    address.sin_port = htons(MY_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    // bind server check
    if (bind(servfd, (struct sockaddr *)&address, sizeof(address))==-1) // error checking to binding
    { 
        fprintf(stderr,"error binding\n");
        return 1;
    }
    int listend=listen(servfd,1024); // listen
    // listen check
    if (listend==-1)
    {
        fprintf(stderr,"error with listen\n");
        return 1;
    }
    // accept and interact with clients
    for (;;)
    {
        FD_ZERO(&readfds); // clear read set
        FD_SET(servfd, &readfds); // set server file descriptor to readfds
        clientaccpt=head;
        while (clientaccpt!=NULL)
        {   //
            if (clientaccpt->fd>0) // if valid file descriptor, add to set to read
            {
                FD_SET(clientaccpt->fd,&readfds);
            }
            if (clientaccpt->fd>maxfd) // to find max file descriptor
            {
                maxfd=clientaccpt->fd;
            }
            clientaccpt=clientaccpt->next;
        }
        // use select to wait for ready file descriptors to read
        int waitactive=select(maxfd+1,&readfds,NULL,NULL,NULL);
        if (waitactive==-1) // if select has error
        {
            fprintf(stderr, "select error\n");
            close(servfd);
            head=freeall(head);
            return 1;
        }
        if (FD_ISSET(servfd,&readfds)) // if a file descriptor is ready to be checked
        {
            // to manage new connection
            int new = accept(servfd, (struct sockaddr *)&address, &saddrlen);
            if (new==-1) // new connection failed
            {
                fprintf(stderr,"accept failed, no connection\n");
                close(servfd);
                head=freeall(head);
                return 1;
            }
            // add new client
            player *newplayer=makeplayer();
            newplayer->fd=new;
            head=addplayer(head,newplayer);
        }
        // check if each client done anything
        clientact=head;
        while (clientact!=NULL)
        {
            next=clientact->next; // incase clientact gets deleted
            // if client exists and it is ready
            if (FD_ISSET(clientact->fd,&readfds)) 
            {
                char command[1024]; // to store possible command
                int readclient=read(clientact->fd,command,sizeof(command)-1); // read to see if/what they said
                if (readclient<=0) //client disconnected
                {
                    head=disconnect(broadmsg,clientact,head,sizeof(broadmsg));
                }
                else 
                {
                    if (clientact->used+readclient>=sizeof(clientact->inputbuf)-1) // check if buffer overflows
                    {
                        fprintf(stderr,"buffer overflow\n");
                        head=disconnect(broadmsg,clientact,head,sizeof(broadmsg));
                        continue;
                    }
                    if (clientact->used+readclient>100)  // if no '\n' within 100 bytes
                    {
                        head=disconnect(broadmsg,clientact,head,sizeof(broadmsg));
                        continue;
                    }
                    memcpy(clientact->inputbuf+clientact->used,command,readclient); // copy input into buffer
                    clientact->used+=readclient; // increase amount of used space as input added to buffer
                    newline=memchr(clientact->inputbuf,'\n',clientact->used);
                    if (newline==NULL) // if no '\n'
                    { 
                        continue; // wait for full line
                    }
                    int lengthline=newline-(clientact->inputbuf)+1; // calculate length of input
                    memcpy(line,clientact->inputbuf,lengthline); // store new line
                    line[lengthline-1]='\0'; // null terminate it
                    memmove(clientact->inputbuf,clientact->inputbuf+lengthline,clientact->used-lengthline); // move to next processed line 
                    clientact->used-=lengthline; //remove previous processed line space
                    char *action=strtok(line,"\n"); // split command by '\n' 
                    while (action!=NULL) // parse through all potiential strings
                    {
                        char actioncheck[100]; 
                        snprintf(actioncheck,sizeof(actioncheck),"%s\n",action); // add \n back to sscanf
                        if (sscanf(actioncheck, "REG %20s %d %d %c", name, &x, &y, &d)==4)
                        {
                            if (samename(head,name)==1) // if client has same name
                            {
                                msgstat=write(clientact->fd,"TAKEN\n",6);
                                if (msgstat==-1) // if message didn't send
                                {
                                    fprintf(stderr,"error sending TAKEN message to client\n");
                                }
                            }
                            else if (x<10&&x>-1&&y<10&&y>-1&&strlen(name)<=20&&validname(name)==0&&(d=='-'&&x-2>=0&&x+2<=9||d=='|'&&y-2>=0&&y+2<=9))// possible clients
                            {
                                //initialize player
                                initialize(clientact,x,y,name,d);
                                msgstat=write(clientact->fd,"WELCOME\n",8); // welcome client
                                if (msgstat==-1) // if message didn't send
                                {
                                    fprintf(stderr,"error sending WELCOME message to client\n");
                                }
                                memset(broadmsg, 0, sizeof(broadmsg));
                                snprintf(broadmsg,sizeof(broadmsg),"JOIN %.20s\n",name); // form JOIN broadcast message
                                broadcast(head,broadmsg); // send message 
                            }
                            else 
                            {
                                msgstat=write(clientact->fd,"INVALID\n",8); 
                                if (msgstat==-1) // if message didn't send
                                {
                                    fprintf(stderr,"error sending INVALID message to client\n");
                                }  
                            }
                        }
                        // bomb
                        else if (sscanf(actioncheck, "BOMB %d %d\n", &x, &y)==2)
                        {
                            if (clientact->reg==0)
                            {
                                msgstat=write(clientact->fd,"INVALID\n",8); 
                                if (msgstat==-1) // if message didn't send
                                {
                                    fprintf(stderr,"error sending INVALID message to client\n");
                                }
                            }
                            else if (clientact->reg==1) {  // if client is registered, can bomb
                                hitcount=0; // reset hit count
                                clientbomb=head;
                                while(clientbomb!=NULL) // check all clients to see if they got bombed
                                {
                                    nextbomb=clientbomb->next;
                                    if (clientbomb->reg==1)
                                    {
                                        if (bombcheck(clientbomb,x,y)==1) // if valid player got bombed
                                        {
                                            memset(broadmsg, 0, sizeof(broadmsg));
                                            snprintf(broadmsg,sizeof(broadmsg),"HIT %.20s %d %d %.20s\n",clientact->name,x,y,clientbomb->name); // form HIT broadcast message
                                            broadcast(head,broadmsg); // send message 
                                            hitcount++; // to indicate ship was hit
                                        }
                                        if (lose(clientbomb)==1) // check if a valid player lost/got their ship hit
                                        {
                                            head=disconnect(broadmsg,clientbomb,head,sizeof(broadmsg));
                                        }
                                    }
                                    clientbomb=nextbomb;
                                }
                                if (hitcount==0) // if no ships got hit AND clients exist
                                {
                                    snprintf(broadmsg,sizeof(broadmsg),"MISS %.20s %d %d\n",clientact->name,x,y); // form MISS broadcast message
                                    broadcast(head,broadmsg); // send message 
                                }
                                
                            }
                            
                        }
                        else // invalid command
                        {
                            msgstat=write(clientact->fd,"INVALID\n",8); 
                            if (msgstat==-1) // if message didn't send
                            {
                                fprintf(stderr,"error sending INVALID message to client\n");
                            }
                        }
                        action=strtok(NULL,"\n");
                    }
                }
            }
            sweepup(head); // delete any clients if needed
            clientact=next; // to continue loop
        }
    }
}
