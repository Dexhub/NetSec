/*
 * Use: Constantly get options from the user and fork of server processes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "globals.h"
#include "unp.h"

int main()
{
    int option = 0;
    struct hostent *he;
    struct in_addr **addr_list;

 //   if (argc != 2) //Second argument for socket
//       err_quit("usage: client <IPaddress/Domainaddress>");

    //Get host name:
  //  if ( (he = gethostbyname( argv[1] ) ) == NULL)
  //  {

   // }
    // false case -  IP Address is supplied
  //  char * ip = 

    // print information about this host:
//    printf("Official name is: %s\n", he->h_name);
//    printf("    IP addresses: ");
//    addr_list = (struct in_addr **)he->h_addr_list;
//    for(i = 0; addr_list[i] != NULL; i++) {
//        printf("%s ", inet_ntoa(*addr_list[i]));
//    }
//    printf("\n");
//
//    return 0;
//}

    while(1)
    {
        printf("\n Choose from the following options");
        printf("\n 1: Echo Client\n 2: Daytime Client\n 3:Exit\n");
        int temp = scanf("%d",&option);
        if (option > 3 || option < 1)
        {
            printf("Invalid option entered\n");
        }
        else if (option == 3)
        {
            printf("Exiting Program");
            break;
        }
        else
        {
           if(option == 1) // Echo client selected
           {
               int pfd[2];
               char buf[SIZE];
               int pid, nread;
               char pfd_buffer[10];
               if (pipe(pfd) == -1)
               {
                 perror("pipe failed");
                 exit(1);
               }
               if ((pid = fork()) < 0)
               {
                 perror("fork failed");
                 exit(2);
               }
               if (pid == 0)// Child process
               {
                   /* child */
                   close(pfd[0]);
                   int n = sprintf(pfd_buffer, "%d",pfd[1]); 
                   execlp("xterm","xterm", "-e","./echo_cli","127.0.0.1",pfd_buffer, (char*)0);
                   close(pfd[1]);
                   //  close(pfd[1]); // After child exits -> add in signal handler for ctrl+c

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
                           printf("\n Please use xterm to enter input");
                       }
                       else if(FD_ISSET(pfd[0], &rset))
                       {
                           while ((nread = read(pfd[0], buf, SIZE)) != 0)
                               printf("CHILD SIGNAL: %s\n", buf);
                           break;
                       }
                    }
                    close(pfd[0]);
                   // Read from the pipe
               }

           }
           else // daytime client selected
           {
               int timepid = Fork();
               if (timepid == 0)// Child process
               {

               }
               else if(timepid > 0) // Parent Process code
               {
                   // Read from the pipe

               }
               else // Error forking
               {

               }


           } 
        }

    }
    exit(0);
}
