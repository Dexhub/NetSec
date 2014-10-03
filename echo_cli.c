#include	"unp.h"

// Global Variables
int pipefd;
void signal_callback_handler(int signum)
{
    char error_msg[] = "Kill signal detected!";
    if (sizeof(error_msg) != (write(pipefd, error_msg,sizeof(error_msg)) ))
        exit(-1);
    exit(1);
}
int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 3) //Third argument for socket
		err_quit("usage: tcpcli <IPaddress>");

        pipefd = atoi(argv[2]); // Initialize Global Variable

         // Register signal and signal handler
        signal(SIGINT, signal_callback_handler);

	if ( (sockfd = socket(AF_INET, SOCK_STREAM,0 /*protocol*/)) < 0)
        {
            char error_msg[] = "Client Socket creation error";
            if (sizeof(error_msg) != write(pipefd, error_msg,sizeof(error_msg)))
                exit(-1);
            printf("%s", error_msg);
            exit(1);
        }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr); //TODO This is gonna change

        if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            char error_msg[] = "Client Connection error";
            if (sizeof(error_msg) != write(pipefd, error_msg,sizeof(error_msg)))
                exit(-1);
            printf("%s", error_msg);
            exit(2);
        }
       
        while(1)
        {
	    str_cli(stdin, sockfd);		/* do it all */
        }
	exit(0);
}

