/*
 * Use: Constantly get options from the user and fork of server processes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "globals.h"
#include "unp.h"

int main(int argc, char *argv[])
{
    int option = 0;
    struct hostent *he;
    struct in_addr **addr_list;
    char dest_addr[SIZE];
    int s;
    
    if (argc != 2) //Second argument for socket
        err_quit("usage: client <IPaddress/Domainaddress>\n");
    
    //Get host name:
    struct in_addr ipv4addr;

    s = inet_pton(AF_INET, argv[1], &ipv4addr);

    if(s > 0)
        he = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    else
        he = gethostbyname(argv[1]);

    if (he != NULL)
    {
        if (inet_ntop(AF_INET, he->h_addr, dest_addr, sizeof(dest_addr)) == NULL)
            he = NULL;
    }
    if (he == NULL)
    {
        printf("Invalid Server Address\n");
        exit(0);
    }

    // print information about this host:
    printf("Host name is: %s\n", he->h_name);
    addr_list = (struct in_addr **)he->h_addr_list;
    printf("IP Address: %s \n", inet_ntoa(*addr_list[0]));

    printf("-------------- Client Started ------------------------\n");
    while(1)
    {
        printf("--------------------------------------------------\n");
        printf("\n Choose from the following options\n");
        printf("\n 1: Echo Client\n 2: Daytime Client\n 3: Exit\n");
        printf("--------------------------------------------------\n");
        printf("Selection : ");
        if( scanf("%d",&option) != 1)
        {
            char temp[SIZE];
            Fgets(temp, SIZE, stdin);
            printf("Invalid option entererd\n");
            continue;
        }

        if(option == 1 || option == 2) // Echo client selected
        {
            int pfd[2];
            char buf[SIZE];
            int pid, nread;
            char pfd_buffer[10];
            if (pipe(pfd) == -1)
            {
              perror("pipe failed\n");
              exit(1);
            }
            if ((pid = fork()) < 0)
            {
              perror("fork failed\n");
              exit(2);
            }
            if (pid == 0)// Child process
            {
                /* child */
                close(pfd[0]);
               
                int n = sprintf(pfd_buffer, "%d",pfd[1]); 
                if (option == 1)
                    execlp("xterm","xterm", "-e","./echo_cli", inet_ntoa(*addr_list[0]), pfd_buffer, (char*)0);
                else
                    execlp("xterm","xterm", "-e","./time_cli", inet_ntoa(*addr_list[0]), pfd_buffer, (char*)0);
                
                close(pfd[1]);

            }
            else // Parent Process code
            {
                close(pfd[1]);

                //Select on pfd[0] and stdin to see if someone has typed on main terminal
                fd_set rset;
                int maxfdp1;
                FD_ZERO(&rset); 
                while(1) 
                {
                    FD_SET(STDIN_FILENO, &rset);// Stdin
                    FD_SET(pfd[0], &rset); // Pipe
                    maxfdp1 = max(STDIN_FILENO, pfd[0]) + 1;
                    select(maxfdp1, &rset, NULL, NULL, NULL);
                    if (FD_ISSET(STDIN_FILENO, &rset))  /* socket is readable */
                    {
                        char sendline[MAXLINE+1];
                        char *temp = fgets(sendline, MAXLINE, stdin);
                        printf("\n Please use xterm to enter input\n");
                    }
                    else if(FD_ISSET(pfd[0], &rset))
                    {
                        while ((nread = read(pfd[0], buf, SIZE)) != 0)
                            printf("CHILD SIGNAL: %s\n", buf);
                        wait(NULL);
                        break;
                    }
                 }
                 close(pfd[0]);
                // Read from the pipe
            }

        } //Echo server finish
        else if (option == 3)// daytime client selected
        {
            printf("Exiting Program\n");
            break;
        }
        else
        {
            printf("Invalid option entererd\n");
        }

    }
    exit(0);
}
