/*
 * Use: Constantly get options from the user and fork of server processes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define SIZE 1024
int main()
{
    int option = 0;
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
                   while ((nread = read(pfd[0], buf, SIZE)) != 0)
                             printf("CHILD SIGNAL: %s\n", buf);
                   close(pfd[0]);
//dup2(pfd[1],1);
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
