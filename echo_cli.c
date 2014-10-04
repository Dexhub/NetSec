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


void
wrap_str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	int		n;

	stdineof = 0;
	FD_ZERO(&rset);
	for ( ; ; ) {
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset)) {	/* socket is readable */
			if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1)
					return;		/* normal termination */
				else
                                {
                                    char error_msg[] = "Server terminated prematurely"; 
                                    if( write(pipefd,error_msg, sizeof(error_msg)) != sizeof(error_msg));
				        err_quit("str_cli: server terminated prematurely");
                                }
			}

			Write(fileno(stdout), buf, n);
		}

		if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
			if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				Shutdown(sockfd, SHUT_WR);	/* send FIN */
				FD_CLR(fileno(fp), &rset);
				continue;
			}

			Writen(sockfd, buf, n);
		}
	}
}



int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 3) //Third argument for socket
		err_quit("usage: tcpcli <IPaddress>\n");

        pipefd = atoi(argv[2]); // Initialize Global Variable

        printf("------------------------------\n");
        printf("\t Echo Client\n");
        printf("------------------------------\n");

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
	servaddr.sin_port = htons(ECHO_PORT);
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
       
        while(1)
        {
	    wrap_str_cli(stdin, sockfd);		/* do it all */
        }
	exit(0);
}

