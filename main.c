#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "shell.h"

struct command_t command;

// Greeting shell during startup
void init_shell()
{
    system("clear");
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****FATEH'S SHELL****");
    printf("\n\n\t       -WELCOME-");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1);
    system("clear");
}

// Function to print Current Directory.
void printPrompt()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n %s >> ", cwd);
}

// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO SHELL HELP***"
        "\nCopyright @ Fateh Ali Aamir"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
		"\n>mkdir"
		"\n>touch"
        "\n>all other general commands available in UNIX shell");
  
    return;
}

// EXit mesg
void exitmsg()
{
    printf("\n***BYE BYE!!!***"
        "\nCopyright @ Fateh Ali Aamir"
        "\n\n East or West, Waleed is the best");
  
    return;
}

// looks up path using environment variable PATH
char * lookupPath(char **argv, char **dir) 
{
	char *result;
	char pName[MAX_PATH_LEN];

	if( *argv[0] == '/') 
	{
		return argv[0];
	}
	else if( *argv[0] == '.') 
	{
		if( *++argv[0] == '.') 
		{
			 if(getcwd(pName,sizeof(pName)) == NULL)
				 perror("getcwd(): error\n");
			 else 
			 {
				 *--argv[0];
				 asprintf(&result,"%s%s%s",pName,"/",argv[0]);
			 }
			 return result;
		}
		*--argv[0];
		if( *++argv[0] == '/') 
		{
			if(getcwd(pName,sizeof(pName)) == NULL)
				perror("getcwd(): error\n");
			else 
			{
				asprintf(&result,"%s%s",pName,argv[0]);
			}
			return result;
		}
	}

	// look in PAH directories, use access() to see if the
	// file is in the dir
	int i;
	for(i = 0 ; i < MAX_PATHS ; i++ ) 
	{
		if(dir[i] != NULL) {
			asprintf(&result,"%s%s%s",  dir[i],"/",argv[0]);
			if(access(result, X_OK) == 0) 
			{
				return result;
			}
		}
		else continue;
	}

	fprintf(stderr, "%s: command not found\n", argv[0]);
	return NULL;
}

// this function populates "pathv" with environment variable PATH
int parsePath(char* dirs[])
{
	int debug = 0;
	char* pathEnvVar;
	char* thePath;
	int i;

	for(i=0 ; i < MAX_ARGS ; i++ )
		dirs[i] = NULL;

	pathEnvVar = (char*) getenv("PATH");
	thePath = (char*) malloc(strlen(pathEnvVar) + 1);
	strcpy(thePath, pathEnvVar);

	char* pch;
	pch = strtok(thePath, ":");
	int j=0;
	
	// loop through the thePath for ':' delimiter between each path name
	while(pch != NULL) 
	{
		pch = strtok(NULL, ":");
		dirs[j] = pch;
		j++;
	}
}

// this function parses commandLine into command.argv and command.argc
int parseCommand(char * commandLine, struct command_t * command) 
{
	int debug = 0;

	char * pch;
	pch = strtok (commandLine," ");
	int i=0;

	while (pch != NULL) 
	{
		command->argv[i] = pch;
		pch = strtok (NULL, " ");
		i++;
	}
	
	command->argc = i;
	command->argv[i++] = NULL;

	return 0;
}

// this function read user input and save to commandLine
int readCommand(char * buffer, char * commandInput) 
{
	int debug = 0;
	buf_chars = 0;


	while((*commandInput != '\n') && (buf_chars < LINE_LEN)) 
	{
		buffer[buf_chars++] = *commandInput;
		*commandInput = getchar();
	}

	buffer[buf_chars] = '\0';

	return 0;
}

// internal command clears terminal screen
//
// @return void
void clearScreen() 
{
	system("clear");
}

// internal command to change diractory
//
// return void
void changeDir() 
{
	if (command.argv[1] == NULL) 
	{
		chdir(getenv("HOME"));
	} 
	else 
	{
		if (chdir(command.argv[1]) == -1) 
		{
			printf(" %s: no such directory\n", command.argv[1]);
		}
	}
}

// This function check is for built in commands
// and processes if there any
//
// @return boolean
int checkInternalCommand() 
{

	if(strcmp("cd", command.argv[0]) == 0) 
	{
		changeDir();
		return 1;
	}

	if(strcmp("clear", command.argv[0]) == 0) 
	{
		clearScreen();
		return 1;
	}

	return 0;
}

// excuteCommand() executes regular command, commands which doesn't have > < |
// rediractions
//
// example: ls -il, cat filname
//
// @return 0 if exec if successful
int executeCommand() 
{
	int child_pid;
	int status;
	pid_t thisChPID;

	child_pid = fork();
	if(child_pid < 0 ) 
	{
		fprintf(stderr, "Fork fails: \n");
		return 1;
	}
	else if(child_pid==0) //child proc
	{
		execve(command.name, command.argv,0);
		printf("Child process failed\n");
		return 1;
	}
	else if(child_pid > 0) //parent proc
	{
		do 
		{
			thisChPID = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
            if (thisChPID == -1) 
			{
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) 
			{
                //printf("exited, status=%d\n", WEXITSTATUS(status));
                return 0;
            } else if (WIFSIGNALED(status)) 
			{
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) 
			{
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) 
			{
                printf("continued\n");
            }
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		return 0;
	}

}

int main(int argc, char* argv[]) 
{
	int i;
	int debug = 0;

	parsePath(pathv);
	init_shell();

	// main loop
	while(TRUE) 
	{
		printPrompt();

		commandInput = getchar(); //gets 1st char
		if(commandInput == '\n') 
		{ // if not input print prompt
			continue;
		}
		else 
		{
			readCommand(commandLine, &commandInput); // read command

			if((strcmp(commandLine, "exit") == 0) || (strcmp(commandLine, "quit") == 0))
				break;

			if(strcmp(commandLine, "help") == 0)
				openHelp();

			parseCommand(commandLine, &command); //parses command into argv[], argc

			if(checkInternalCommand() == 0) 
			{
				command.name = lookupPath(command.argv, pathv);

				if(command.name == NULL) 
				{
					printf("Stub: error\n");
					continue;
				}

				executeCommand();
			}
		}
	}

	printf("\n");
	exit(EXIT_SUCCESS);
}
