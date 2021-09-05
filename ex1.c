// Yoav Berger

#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

//struct to save the commands data - the string and the pid
typedef struct{
    char varCommand[100];
    pid_t commandId;
} inputCommand;

void main()
{
    //initialize variabels for the program
    //buffer for the input and ptr for input without \n
    char commandBuffer[100];
    char* commandNoN;
    char commandCopy[100];
    //array for the different part of the command, counter for the parts and part holder
    char* commandParts[100];
    int numParts;
    char* part;
    //array for history of commands and counter for the amount of commands
    inputCommand historyArr[100] = {};
    int numCommand = 0;
    //background and built-in flag
    int backgroundFlag = 0;
    int builtInFlag = 0;
    //cd command arguments - paths, cd operator, chdir's return value, home and input path
    char originalPath[1024] = {};
    char newPath[1024] = {};
    char cdOperator[1024] = {};
    int chdirVal = 0;
    char* inputPath;
    char* homePath;
    //regular command arguments
    pid_t IdOfCommand;
    int execVal, waitPidStatus;

    //printing the prompt and getting the user input
    printf("$ ");
    fflush(stdout);

    //properly getting the command string
    fgets(commandBuffer,100,stdin);
    //removing \n 
    commandNoN = strtok(commandBuffer,"\n");
    //printf("check %s\n", commandNoN);//******************************

    //starting the handelling of the command
    while (strcmp("exit", commandNoN) != 0)
    {
        //saving the command into the history array
        strcpy(commandCopy, commandNoN);
        //printf("check2 %s\n", commandCopy);

        //properly inserting the command string into array
        numParts = 0;
        memset(&commandParts[0],0, sizeof(commandParts));
        part = strtok(commandNoN, " ");
        while (part != NULL)
        {
            commandParts[numParts] = part;
            part = strtok(NULL," ");
            numParts++;
        }

        //saving the command into the history array
        // strcpy(historyArr[numCommand].varCommand, commandNoN);
        //printf("check1 %s\n", commandNoN);
        //strcpy(historyArr[numCommand].varCommand, commandCopy);

        //checking for background char, if exist remove it and turn on its flag
        int lastPartInd = numParts -1;
        if ((commandParts[lastPartInd] != NULL) && (strcmp("&", commandParts[lastPartInd]) == 0))
        {
            backgroundFlag = 1;
            commandParts[lastPartInd] = NULL;

            //also deleting & from command copy that goes to history
            int copyLen = strlen(commandCopy);
            commandCopy[copyLen - 1] = '\0';
            commandCopy[copyLen - 2] = '\0';
        }
        
        //saving the command into the history array
        // strcpy(historyArr[numCommand].varCommand, commandNoN);
        //printf("check1 %s\n", commandNoN);
        strcpy(historyArr[numCommand].varCommand, commandCopy);

        //checking if the command is cd, and operate
        if (strcmp("cd", commandParts[0]) == 0)
        {
            //saving the pid because we need it for waitpid
            historyArr[numCommand].commandId = getpid();   
            
            //check that cd command legal
            if (numParts >= 3)
            {
                printf("Too many arguments\n");
            }
            else
            {
                //save and get the current path so we can use it
                strncpy(originalPath,newPath, sizeof(newPath));
                if(getcwd(newPath, sizeof(newPath)) == NULL)
                {
                    printf("An error occurred\n");
                }

                //get the argument after cd
                strcpy(cdOperator,commandParts[1]);
                
                //checking the operators
                //check if argument is .. and operate
                if (strcmp("..", commandParts[1]) == 0)
                {
                    chdirVal = chdir("..");
                }
                
                //check if argument is - and operate, also checking original path exist
                else if (strcmp("-", commandParts[1]) == 0)
                {
                    if(strlen(originalPath)!=0){
                        chdirVal = chdir(originalPath);
                    }
                }

                //chek if argument is ~ alone and operate
                else if (strcmp("~", commandParts[1]) == 0)
                {
                    chdirVal = chdir(getenv("HOME"));
                }

                //check if argument is ~ and a path and operate
                else if ('~' == cdOperator[0])
                {
                    //getting the home path, extracting the input path, combine them and go there
                    homePath = getpwuid(getuid())->pw_dir;
                    inputPath = strtok(cdOperator,"~");
                    strcat(homePath, inputPath);
                    chdirVal = chdir(homePath);
                }

                //regular case cd
                else
                {
                    chdirVal = chdir(commandParts[1]);
                }

                //checking if there been error in chdir and print it
                if (chdirVal == -1)
                {
                    printf("chdir failed\n");
                }
                
            }
            
            //setting the built-in flag
            builtInFlag = 1;
        }
        

        //checking if the command is history, and operate
        if (strcmp("history", commandParts[0]) == 0)
        {
            //saving the pid because we need it for waitpid
            historyArr[numCommand].commandId = getpid();
            
            //going over the whole history of commands array and print them with their status
            int j;
            for (j = 0; j <= numCommand; j++)
            {
                printf("%s", historyArr[j].varCommand);
                //fflush(stdout);
                if ((waitpid(historyArr[j].commandId,NULL,WNOHANG) == 0) || (j == numCommand))
                {
                    printf(" RUNNING\n");
                }
                else
                {
                    printf(" DONE\n");
                }
                
            }
            
            //setting the built-in flag
            builtInFlag = 1;
        }
        

        //checking if the command is jobs, and operate
        if (strcmp("jobs", commandParts[0]) == 0)
        {
            //saving the pid because we need it for waitpid
            historyArr[numCommand].commandId = getpid();
            
            //going over the whole history of commands array and print only the ones who run
            int j;
            for (j = 0; j <= numCommand; j++)
            {
                if (!waitpid(historyArr[j].commandId,NULL,WNOHANG))
                {
                    printf("%s\n", historyArr[j].varCommand);
                }
            }
            
            //setting the built-in flag
            builtInFlag = 1;
        }

        //regular command
        if (builtInFlag != 1)
        {
            //in case command is echo, check if had "" and remove them
            if (strcmp("echo", commandParts[0]) == 0)
            {
                if (*commandParts[1] == '"')
                {
                    commandParts[1] = strtok(commandParts[1],"\"");
                    commandParts[numParts - 1] = strtok(commandParts[numParts - 1],"\"");
                }
                
            }
            
            //creating fork and checking if son was born, if it was, sending the command to the son, and check if send failed
            IdOfCommand = fork();
            if (IdOfCommand == -1)
            {
                printf("fork failed\n");
            }
            else if (IdOfCommand == 0)
            {
                execVal = execvp(commandParts[0], commandParts);
                if (execVal == -1)
                {
                    printf("exec failed\n");
                    return;
                }
                
            }

            //in the father proccess checking if its foreground and update id in the array
            else
            {
                if (backgroundFlag == 0)
                {
                    waitpid(IdOfCommand, &waitPidStatus, 0);
                }
                historyArr[numCommand].commandId = IdOfCommand;
            }
        }

        //end of loop - settings for the next input
        //printing the prompt and getting the user input
        printf("$ ");
        fflush(stdout);

        //properly getting the command string, initiating the buffer and removing \n 
        memset(&commandParts[0],0, sizeof(commandParts));
        fgets(commandBuffer,100,stdin);
        commandNoN = strtok(commandBuffer,"\n");
        //printf("check %s\n", commandNoN);

        //increasing the commands counter and reseting the flags
        numCommand++;
        backgroundFlag = 0;
        builtInFlag = 0;
    }
    
    //in case input was exit exiting the program
    exit(0);
}
