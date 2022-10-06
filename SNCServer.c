#include	"acc.h"
#include	"signal.h"
#include	"pthread.h"
#include 	"string.h"
#include <ctype.h>
#include <stdio.h>


#define	MAXLINE	4096
#define MAX_CLIENTS 100

static void	*doit(void *arg);/* each thread executes this function */


static int cli_count = 0;
static int uid = 0;
static char w_message[MAXLINE];

//declaring the server arguments globally


/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char nickname[32];
	char realname[100];
} client_info;

client_info *clients[MAX_CLIENTS];

int main(int argc, char **argv)
{
	int			listenfd, connfd ,iptr;
	pthread_t		tid;
	socklen_t		clilen, addrlen, len;
	struct sockaddr_in	 servaddr;
	struct sockaddr	*cliaddr;
	char		buff[MAXLINE];
	
	
	void			sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

//--------------------------------------------------------------------------------------------------------------------------------------------


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
		if((cli_count ) == MAX_CLIENTS){
		printf("Maximum clients connected. Client %d Rejected \n",cli_count+1);
		send(iptr,&maxCli_warn,strlen(maxCli_warn),0);
		close(iptr);
		continue;
		}

		Pthread_create(&tid, NULL, &doit, iptr);
		
		cli_count++;
	}
	
	
}

static void *
doit(void *arg)
{

	int	connfd,len,j=0;
	connfd = connfd = (int *) arg;
	char buffer[4096];
	char str[100];
	char arg1[20],arg2[10],arg3[10];
	
	//The welcome message displayed at the start of the game to the client
	char wMessage[] = "\n====================Welcome to Simple Network Chat ==================== \n";
	
	//Send the welcome message to client
	send(connfd,&wMessage,strlen(wMessage),0);
	
	

	while(1){

		//Receiving	
  		bzero(&buffer,sizeof(buffer));
		len=recv(connfd,&buffer,sizeof(buffer),0);
		strcpy(str,buffer);

		sscanf(str,"%s %s %s",arg1,arg2,arg3);

		if(strcasecmp(toupper(arg1),"JOIN")==0){
			char nclient[]=" ,Welcome to Simple Network Chat";
			strcat(arg2,&nclient,strlen(nclient));
			send(connfd,&buffer);
		}    
				

			
		Close(connfd);	
	}		/* we are done with connected socket */
	
	return(NULL);
}

