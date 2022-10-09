#include	"acc.h"

#define LENGTH 2048

volatile flag = 0;
int sockfd = 0;

void str_overwrite_stdout() {
  printf("\n%s", "20906531> ");
  fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
  char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, LENGTH, stdin);
    str_trim_lf(message, LENGTH);

    if (strcmp(message, "QUIT") == 0) {
    	sprintf(buffer, "%s\n", message);
      send(sockfd, buffer, strlen(buffer), 0);
    	exit_recive(sockfd);
			break;
    } else {
      sprintf(buffer, "%s\n", message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

		bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = {};
  while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {
      printf("%s\n", message);
      str_overwrite_stdout();
      if(strcasecmp(message,"\nSorry! Maximum client count reached. Try next time\n")==0){
      	catch_ctrl_c_and_exit(2);
      }
    } else if (receive == 0) {
			break;

    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
  catch_ctrl_c_and_exit(2);
}

void exit_recive(int sockfd){
	int len;
	char buffer[4096];
	len=recv(sockfd,&buffer,sizeof(buffer),0);
	printf("%s",buffer);
}

main(int argc, char **argv)
{
	int		len,s;
	struct sockaddr_in	servaddr;
	char buffer[4096];
	char file[4096];

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(52001);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
	//Get welcome message
	len=recv(sockfd,&buffer,sizeof(buffer),0);
	printf("%s",buffer);

	str_overwrite_stdout();
	bzero(&buffer,sizeof(buffer));
	if ( fgets( buffer, sizeof(buffer), stdin ) != NULL ) buffer[strcspn( buffer, "\n" )] = '\0';
	send(sockfd,&buffer,sizeof(buffer),0);
	
	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd); 

	exit(0);
}

