#include	"acc.h"
#include	"signal.h"
#include	"pthread.h"
#include 	"string.h"
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


#define	MAXLINE	4096


void doit(void *arg);/* each thread executes this function */

//Declraing Global Varibles=================================================================================================================

static int cli_count = 0;
static int uid = 0;
static char w_message[MAXLINE];
static int MAX_CLIENTS=0;
static int max_idle_time=0;

//
typedef struct{
   struct sockaddr_in address;
   int sockfd;
   int uid;
   char nickname[20];
   char name[20];
} client_info;

//==============================================================================================================================================
//startup setups

client_info *clients[10];
struct timeval timeout; 

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//==============================================================================================================================================

//Braodacast message to every user==============================================================================================================
void broadcaster(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

//sends message to particualr person===============================================================================================================
void messenger(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid==uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}



/* Add clients*/
void queue_add(client_info *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients */
void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/*==================================================================================================================================================*/
int main(int argc, char **argv)
{
	MAX_CLIENTS=atoi(argv[1]);
	timeout.tv_sec=atoi(argv[2]);
//--------------Server side list setup for request--------------------------------------------------------------------------------------------
	int			listenfd, iptr;
	pthread_t		tid;

	socklen_t		clilen, addrlen, len;
	struct sockaddr_in	 servaddr;
	struct sockaddr_in cliaddr;
	char		buff[MAXLINE];
	
	
	void			sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

//---------------Client accepting in server side-----------------------------------------------------------------------------------------------


	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_TCP_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	while(1)
	 {
		len = sizeof (cliaddr);
		iptr = (int *) Malloc(sizeof(int));
		iptr = Accept(listenfd, (SA *) &cliaddr, &len);

		char buffer[512];

		//The sending maximum clients reached warning to the clients
		char maxCli_warn[] = "\nSorry! Maximum client count reached. Try next time\n";

		//handling the number of clients
		if(cli_count  == MAX_CLIENTS){
		printf("Maximum clients connected. Client %d Rejected \n",cli_count+1);
		send(iptr,&maxCli_warn,strlen(maxCli_warn),0);
		close(iptr);
		continue;
		}
//====================setting up array pointer and saving data to array
		client_info *cli = (client_info *)malloc(sizeof(client_info));
		cli->address=cliaddr;
		cli->sockfd=iptr;
		cli->uid=uid++;

		queue_add(cli);//client added pointer to que of that client
		Pthread_create(&tid, NULL, &doit, (void*)cli);
		
		
		
	}
	
	
}
/*===============================================================================================================================================*/
void doit(void *arg)
{

	int i;
	char buffer[4096];//recive send buffer
	char str[100];
	char buff_out[4096];//broadcast buffer
	int len;
	int		n;
	int leave_flag=0;
	char arg1[10],arg2[20],arg3[10],arg4[10],realname[20];     
    timeout.tv_usec = 0;
	cli_count++;
	client_info *cli=(client_info *)arg;


	//The welcome message displayed at the start
	char wMessage[] = "\n====================Welcome to Simple Network Chat ==================== \n";
	
	Send(cli->sockfd,&wMessage,strlen(wMessage),0);
	//Send the welcome message to client
	
	
	

	for ( ; ; ) {
		if(leave_flag==1){
			break;
		}
		//Receiving	
  		bzero(&buffer,sizeof(buffer));
		len=recv(cli->sockfd,&buffer,sizeof(buffer),0);

		//timeout section for client
    
         if (setsockopt (cli->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                    sizeof timeout) < 0)
            error("setsockopt failed\n");

         if (setsockopt (cli->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
            error("setsockopt failed\n");
/*================================================================================================================================================*/
		if(len>0){//decision making
//================== command seprator
		sscanf(buffer,"%s %s %s %s",arg1,arg2,arg3,arg4);
//===================JOIN section
		if(strcasecmp(arg1,"JOIN")==0){
//===================checking if user already exist or not
			sprintf(realname,"%s %s",arg3,arg4);
			if(strlen(arg2)!=20 && strlen(realname)!=20){
				for(i=0;i<MAX_CLIENTS;++i){
					if(clients[i]){
						if (strcasecmp(clients[i]->nickname,arg2)==0)
						{
							sprintf(buffer,"This nickname is already used please try a new one");
							leave_flag=1;
						
						}
					}
				}
				
				if(leave_flag!=1){
//================================saving nickname and name
					strcpy(cli->nickname,arg2);
					strcpy(cli->name,realname);
					sprintf(buffer,"%s ,Welcome to Simple Network Chat",cli->nickname);
//====================================================================informing others that he has joined the chat and the server
					bzero(&buff_out,MAXLINE);
					sprintf(buff_out,"%s has joined SNC",cli->nickname);
					printf("%s\n",buff_out);
					broadcaster(buff_out,cli->uid);
					bzero(buff_out,MAXLINE);
					}
				send(cli->sockfd,&buffer,sizeof(buffer),0);
			}else{
				sprintf(buffer,"nickname or real name is less than or greater than 20 please check  try again");
				send(cli->sockfd,&buffer,sizeof(buffer),0);
				leave_flag=1;
			}
//=========================================time
		}else if(strcasecmp(arg1,"TIME")==0){
			time_t t=time(NULL);
			sprintf(buffer,"Time : %s",ctime(&t));
			send(cli->sockfd,&buffer,sizeof(buffer),0);	
//======================================who is 	
		}else if(strcasecmp(arg1,"WHOIS")==0){
			for (i=0;i<MAX_CLIENTS;++i){
				if(clients[i])
				{
					if (strcasecmp(clients[i]->nickname,arg2)==0)
					{
						sprintf(buffer,"uid : %d,nickname : %s, name : %s",clients[i]->uid,clients[i]->nickname,clients[i]->name);
						send(cli->sockfd,&buffer,sizeof(buffer),0);
					}
				}
			}
//=====================================message 
		}else if(strcasecmp(arg1,"MSG")==0){
			for (i=0;i<MAX_CLIENTS;++i){
				if(clients[i])
				{
					if (strcasecmp(clients[i]->nickname,arg2)==0)
					{
						sprintf(buffer,"%s : %s",arg2,arg3);
						messenger(buffer,clients[i]->uid);
					}
				}
			}
//====================client exit
		}else if(strcasecmp(arg1,"QUIT")==0){
			sprintf(buff_out,"%s has stopped chatting",cli->nickname);
			sprintf(buffer,"bye %s",cli->nickname);
			send(cli->sockfd,&buffer,sizeof(buffer),0);
			broadcaster(buff_out,cli->uid);
			printf("%s\n",buff_out);
			bzero(buffer,MAXLINE);
			bzero(buff_out,MAXLINE);
			leave_flag=1;
//=====================Alive
		}else if(strcasecmp(arg1,"ALIVE") == 0){
				sprintf(buffer,"OK, %s!",cli->name);
				send(cli->sockfd,&buffer,sizeof(buffer),0);
			}
//========================timeout	
	}else if(len==0){
		sprintf(buff_out,"%s has stopped chatting",cli->nickname);
		broadcaster(buff_out,cli->uid);
		printf("%s\n",buff_out);
		bzero(buff_out,MAXLINE);
		leave_flag=1;
	}else{
		sprintf(buff_out,"%s left Simple Network Chatroom for time ",cli->nickname);
		sprintf(buffer,"SNC timeout");
		send(cli->sockfd,&buffer,sizeof(buffer),0);
		printf("%s\n",buff_out);
		broadcaster(buff_out,cli->uid);
		bzero(buff_out,MAXLINE);
		leave_flag=1;
	}	
			
		
/* ======================Delete client from queue and yield thread =============================================================*/
	close(cli->sockfd);
  	queue_remove(cli->uid);
  	free(cli);
  	cli_count--;
  	pthread_detach(pthread_self());
	return(NULL);
}
}

