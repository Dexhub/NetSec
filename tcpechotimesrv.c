#include "unp.h"
#include "unpthread.h"
#include <time.h>
#include "globals.h"



static void * 
echo_child_function(void *arg)
{
	int connfd;
	connfd = *((int *) arg);
        printf("Fullfiling echo request\n");
	str_echo(connfd);		/* same function as before */
	close(connfd);			/* done with connected socket */
        printf("Echo client terminated successfully\n");
        pthread_exit(arg);
        exit(0);
}

static void * 
time_child_function(void *arg)
{
	int connfd;
	connfd = *((int *) arg);
        printf("Fulfilling time request\n");

        fd_set rset;
        FD_ZERO(&rset);
        struct timeval timeout;

        char buff[MAXLINE];
        timeout.tv_sec = 0;

        while(1)
        {
            FD_SET(connfd,&rset);
            int rc = select(connfd+1, &rset, NULL, NULL, &timeout);
            if (rc == -1)
            {
                perror("select failed\n");
                return;
            }

            if(FD_ISSET(connfd, &rset))
            {
                printf("Time client terminated successfully\n");
                if(close(connfd) == -1) 
                {
                    err_sys("Time thread closing socket error\n");
                }
                FD_CLR(connfd, &rset);
                pthread_exit(arg);
                exit(0);
            }
            else  // Timeout
            {
                time_t ticks = time(NULL);
                snprintf(buff, sizeof(buff), "%.24s\r\n",ctime(&ticks));
                if (write(connfd, buff, strlen(buff)) != strlen(buff))
                {
                    err_sys("Time thread write error\n");
                    return;
                }
                timeout.tv_sec = 5;

            }

        }
}


int
main(int argc, char **argv)
{
	int listenfdecho,listenfdtime, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
        printf("\n-------------Starting Server-------------\n");

         if ( (listenfdecho = socket(AF_INET, SOCK_STREAM, 0/* protocol */)) < 0)
         {
             printf("Error in creating Echo Socket\n");
             exit(1);
         }
         
         if ( (listenfdtime = socket(AF_INET, SOCK_STREAM, 0/* protocol */)) < 0)
         {
             printf("Error in creating Time Socket\n");
             exit(1);
         }

       int option = 1; 
        if(setsockopt(listenfdtime,SOL_SOCKET, SO_REUSEADDR, &option,sizeof(option)) < 0)
        {
            printf("setsockopt on time port failed\n");
            close(listenfdtime);
            exit(2);
        }
       
        option = 1; 
        
        if(setsockopt(listenfdecho,SOL_SOCKET, SO_REUSEADDR , &option,sizeof(option)) < 0)
        {
            printf("setsockopt on echo port failed\n");
            close(listenfdecho);
            exit(2);
        }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(ECHO_PORT);
        
        if (bind(listenfdecho, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("Echo server Bind Error\n");
            exit(2);
        }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(TIME_PORT);
        printf("Server is up!!\n");
        
        if (bind(listenfdtime, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("Time server Bind Error\n");
            exit(2);
        }
        
        int backlog = LISTENQ;
        char *ptr;
	if ( (ptr = getenv("LISTENQ")) != NULL)
        {
            backlog = atoi(ptr);
        }

	if (listen(listenfdecho, backlog) < 0)
        {
            err_sys("listen echo error\n");
            exit(3);
        }

	if (listen(listenfdtime, backlog) < 0)
        {
            err_sys("listen time error\n");
            exit(3);
        }

        clilen = sizeof(cliaddr);
       
        fd_set rset;
        FD_ZERO(&rset);
        int maxfd1 = max(listenfdecho, listenfdtime) + 1;

	while(1)
        {
            FD_SET(listenfdecho, &rset);
            FD_SET(listenfdtime, &rset);
            select(maxfd1, &rset, NULL, NULL, NULL);

            if(FD_ISSET(listenfdecho, &rset)) // Code for echo server
            {
                int res, err;
                pthread_attr_t attr;
                pthread_t thread_echo;
                res = pthread_attr_init(&attr);
                char strbuf[SIZE];
                if (res != 0) 
                {
                    perror("Attribute init failed - Echo server thread\n");
                    exit(EXIT_FAILURE);
                }
                res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                if (res != 0) 
                {
                    perror("Setting detached state failed - Echo server thread\n");
                    exit(EXIT_FAILURE);
                }

		connfd = Accept(listenfdecho, (SA *) &cliaddr, &clilen); // JHOL
                char*  ptr;
                
                Inet_ntop(AF_INET, &cliaddr.sin_addr, strbuf, sizeof(strbuf));
                printf("Connection from %s\n", strbuf);
            
                res = pthread_create(&thread_echo, &attr, echo_child_function, (void*)&connfd);
                if (res != 0) 
                {
                    perror("Creation of echo server thread failed\n");
                    exit(EXIT_FAILURE);
                }
            }
             // Code for handling time server
            else if(FD_ISSET(listenfdtime, &rset)) // Code for time server
            {
                int res, err;
                pthread_attr_t attr;
                pthread_t thread_echo;
                char strbuf[SIZE];
                res = pthread_attr_init(&attr);

                if (res != 0) 
                {
                    perror("Attribute init failed - Time server thread\n");
                    exit(EXIT_FAILURE);
                }
                res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                if (res != 0) 
                {
                    perror("Setting detached state failed - Time server thread\n");
                    exit(EXIT_FAILURE);
                }

		connfd = Accept(listenfdtime, (SA *) &cliaddr, &clilen);

                Inet_ntop(AF_INET, &cliaddr.sin_addr, strbuf, sizeof(strbuf));
                printf("Connection from %s\n", strbuf);
            
                res = pthread_create(&thread_echo, &attr, time_child_function, (void*)&connfd);
                if (res != 0) 
                {
                    perror("Creation of Time server thread failed\n");
                    exit(EXIT_FAILURE);
                }

            }
            else
            {
                printf("Error in Select\n");
                exit(-1);
            }
        
        }
}

