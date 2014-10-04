#include "unp.h"
#include "globals.h"

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
	int sockfd, n, counter = 0;
	char recvline[MAXLINE + 1];
	struct sockaddr_in servaddr;

        if (argc != 3)
		err_quit("usage: a.out <IPaddress>\n");
       
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
	servaddr.sin_port   = htons(TIME_PORT);	/* daytime server */

	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", argv[1]);

        if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            char error_msg[] = "Client Connection error";
            if (sizeof(error_msg) != write(pipefd, error_msg,sizeof(error_msg)))
                exit(-1);
            printf("%s", error_msg);
            exit(2);
        }

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		counter++;
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			err_sys("fputs error\n");
	}

        if (n == 0)
        {
            char error_msg[] = "Server terminated Prematurely!";
            if (sizeof(error_msg) != (write(pipefd, error_msg,sizeof(error_msg)) ))
                exit(-1);
            exit(1);
        }

        else // n< 0
        {
            char error_msg[] = "Read error";
            if (sizeof(error_msg) != write(pipefd, error_msg,sizeof(error_msg)))
                exit(-1);
            printf("%s", error_msg);
            exit(2);
        }

	printf("counter = %d\n", counter);
	exit(0);
}
