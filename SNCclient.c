#include	"acc.h"

int
main(int argc, char **argv)
{
	int			sockfd,len,s;
	struct sockaddr_in	servaddr;
	char buffer[4096];
	char file[4096];

	if (argc != 3)
		err_quit("usage: tcpcli <IPaddress> <port number>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(52001);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	//Get welcome message
	len=recv(sockfd,&buffer,sizeof(buffer),0);
	printf("%s",buffer);
	memset(&buffer,sizeof(buffer),'\0');
	
	//Getting help,stat or start from the user
	bzero(&buffer,sizeof(buffer));
	if ( fgets( buffer, sizeof(buffer), stdin ) != NULL ) buffer[strcspn( buffer, "\n" )] = '\0';
	send(sockfd,&buffer,sizeof(buffer),0);
	
	len=recv(sockfd,&file,sizeof(file),0);
	printf("%s",file);//print howtoplay and stat in the client terminal
	
	//receive the message of requesting guest codes
	bzero(&buffer,sizeof(buffer));
	len=recv(sockfd,&buffer,sizeof(buffer),0);
	printf("%s",buffer);
	
	bzero(&buffer,sizeof(buffer));	
	scanf("%c %c %c %c %c %c %c %c %c %c",&buffer[0],&buffer[1],&buffer[2],&buffer[3],&buffer[4],&buffer[5],&buffer[6],&buffer[7],&buffer[8],&buffer[9]);
	buffer[10] = '\n';
	send(sockfd,&buffer,sizeof(buffer),0);
	


	exit(0);
}

