#include "unp.h"
#include "globals.h"
#include <time.h>

static void * 
echo_child_function(void *arg)
{
	int connfd;
	connfd = *((int *) arg);
        printf("In echo thread\n");
	str_echo(connfd);		/* same function as before */
	close(connfd);			/* done with connected socket */
        printf("Client terminated successfully");
	return(NULL);
}

static void * 
time_child_function(void *arg)
{
	int connfd;
	connfd = *((int *) arg);
        printf("In time thread\n");

        fd_set rset;
        FD_ZERO(&rset);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        char buff[MAXLINE];

        while(1)
        {
            FD_SET(connfd,&rset);
            time_t ticks = time(NULL);
            int rc = select(connfd+1, &rset, NULL, NULL, &timeout);
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
            if (rc == -1)
            {
                perror("select failed");
                return;
            }

            if(FD_ISSET(connfd, &rset))
            {
                printf("Time client terminated successfully");
                if(close(connfd) == -1) 
                {
                    err_sys("Time thread closing socket error");
                    return;
                }
            }
            else  // Timeout
            {
                snprintf(buff, sizeof(buff), "%.24s\r\n",ctime(&ticks));
                if (write(connfd, buff, strlen(buff)) != strlen(buff))
                {
                    err_sys("Time thread write error");
                    return;
                }

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

         if ( (listenfdecho = socket(AF_INET, SOCK_STREAM, 0/* protocol */)) < 0)
         {
             printf("Error in creating Echo Socket");
             exit(1);
         }
         
         if ( (listenfdtime = socket(AF_INET, SOCK_STREAM, 0/* protocol */)) < 0)
         {
             printf("Error in creating Time Socket");
             exit(1);
         }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(ECHO_PORT);
        
        if (bind(listenfdecho, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("Echo server Bind Error");
             exit(2);
        }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(TIME_PORT);
        
        if (bind(listenfdtime, (SA *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("Time server Bind Error");
            exit(2);
        }
        
       int option; 
        if(setsockopt(listenfdtime,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
        {
            printf("setsockopt on time port failed\n");
            close(listenfdtime);
            exit(2);
        }
        
        if(setsockopt(listenfdecho,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
        {
            printf("setsockopt on echo port failed\n");
            close(listenfdecho);
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
            err_sys("listen echo error");
            exit(3);
        }

	if (listen(listenfdtime, backlog) < 0)
        {
            err_sys("listen time error");
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
                if (res != 0) 
                {
                    perror("Attribute init failed - Echo server thread");
                    exit(EXIT_FAILURE);
                }
                res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                if (res != 0) 
                {
                    perror("Setting detached state failed - Echo server thread");
                    exit(EXIT_FAILURE);
                }

		connfd = Accept(listenfdecho, (SA *) &cliaddr, &clilen); // JHOL
                char*  ptr;
                if ( (ptr = sock_ntop((SA *) &cliaddr, clilen) == NULL) )
                {
                    perror("Sock_ntop error");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("Connection from %s\n", ptr);
                }
            
                res = pthread_create(&thread_echo, &attr, echo_child_function, (void*)&connfd);
                if (res != 0) 
                {
                    perror("Creation of echo server thread failed");
                    exit(EXIT_FAILURE);
                }
            }
             // Code for handling time server
            else if(FD_ISSET(listenfdtime, &rset)) // Code for echo server
            {
                int res, err;
                pthread_attr_t attr;
                pthread_t thread_echo;
                res = pthread_attr_init(&attr);
                if (res != 0) 
                {
                    perror("Attribute init failed - Time server thread");
                    exit(EXIT_FAILURE);
                }
                res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                if (res != 0) 
                {
                    perror("Setting detached state failed - Time server thread");
                    exit(EXIT_FAILURE);
                }

		connfd = Accept(listenfdtime, (SA *) &cliaddr, &clilen); // JHOL
                char*  ptr;
                if ( (ptr = sock_ntop((SA *) &cliaddr, clilen) == NULL))
                {
                    perror("Sock_ntop error");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("Connection from %s\n", ptr);
                }
            
                res = pthread_create(&thread_echo, &attr, time_child_function, (void*)&connfd);
                if (res != 0) 
                {
                    perror("Creation of Time server thread failed");
                    exit(EXIT_FAILURE);
                }

            }
            else
            {
                printf("Error in Select");
                exit(-1);
            }
        
        }
}

