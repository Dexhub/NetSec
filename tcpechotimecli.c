/*
 * Use: Constantly get options from the user and fork of server processes
 */
int main()
{
    int option = 0;
    while(1)
    {
        printf("\n Choose from the following options");
        printf("\n 1: Echo Client\n 2: Daytime Client\n 3:Exit\n");
        scanf("%d",&option);
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
               int echopid = Fork();
               if (echopid == 0)// Child process
               {

               }
               else if(echopid > 0) // Parent Process code
               {
                   // Read from the pipe

               }
               else // Error forking
               {

               }

           }
           else // Echo server selected
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
