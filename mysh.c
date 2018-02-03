//Bodie Malik

//TODO
//The history command needs a lot of work
//I don't think the prof understand how bad the current framework is.
//It can do recursive history calls. It may even get stuck in an infinite loop...


/*
PROCESS IDEA
1- get input string from user
2- copy string into history
2- start tokenizing from the front.
	if you run out of tokens, process the command, and output
	if you run into a pipe token, process command and store in file. Then continue
	
	every command removes its-self from the Command string.
	the history command may add more to the front of the Command string
*/


#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 15
#define TEMP_CHUNK 64
#define HISTORY_SIZE 100

//Used for debugging.
#define SHOW_TOKENS false

//FUNCTION DECLARATIONS
void ExecuteCommand( char *commandin);
void ClearAllTokens();
void PrintTokens();
void SaveCommand( char *com );
void ShowHistory();
void ClearHistory();
char * GetHistory(int num);


int ExitShell = 0;

char *History[HISTORY_SIZE];
int HistoryIndex = 0;


//-------------------------MAIN FUNCTION-----------------------------

int main(){
	printf("Welcome to Bodie's OS\n");
	
	//This loop needs to break at some point.
	while(ExitShell == 0)
	{
		printf("$");
		
		//Get input string
		int CommandSize = TEMP_CHUNK;
		char *Command = malloc(CommandSize);
		char c;
		int i = 0;
		while((c = getchar()) != '\n' && c != EOF)
		{
			Command[i++] = c;
			
			if(i == CommandSize)
			{
				CommandSize += TEMP_CHUNK;
				Command = realloc(Command, CommandSize);
			}
		}
		Command[i] = '\0';
		
		//--------------------------SAVE STRING TO HISTORY--------------------------------
		//make sure to save a copy, not just the pointer since the string will be deconstructed
		SaveCommand(Command);
		
		ExecuteCommand(Command);
		
		free(Command);
	}
	
	//Clear up memory?
	
	

	return 0;
}



//Executes the command.
void ExecuteCommand(char *commandin)
{	
	if(commandin == NULL || strlen(commandin) == 0)
		return;
	
	//printf("EXECUTING COMMAND: %s\n", commandin);

	//TURN COMMAND INTO TOKENS
	char *command = malloc( strlen(commandin));
	strcpy(command, commandin);
	
	char *tPoint[MAX_TOKENS + 1];
	int tNum = 0;
	
	char *ttok = strtok(command, " ");
	
	while(ttok != NULL)
	{
		if( strcmp(ttok, "|") == 0)
		{
			//turn on pipe
			break;
		}
		else if(tNum < MAX_TOKENS)
		{
			tPoint[tNum++] = ttok;
		}
		else
		{
			printf("Hit max tokens.\n");
		}
		
		ttok = strtok(NULL, " ");
	}
	
	tPoint[tNum] = NULL;
	
	
	
	//RUN COMMAND
	
	
	if(tNum == 0) //if there are no tokens, skip
	{
		printf("That command has no arguments...\n");
	}
	else if(strcmp(tPoint[0], "exit") == 0) //EXIT
	{
		//BREAK FROM LOOP, ENDING SHELL
		ExitShell = 1;
		//return;
	}
	else if(strcmp(tPoint[0], "cd") == 0) //CHANGE DIRECTORY
	{
		if(tNum == 2)
		{
			int cdReturn = chdir(tPoint[1]);
			if(cdReturn != 0)
			{
				perror("CD Error");
			}
		}
		else if( tNum > 2) printf("cd only takes one argument.\n");
		else printf("cd requires an argument.\n");
	}
	else if(strcmp(tPoint[0], "history") == 0) //HISTORY
	{
		if(tNum == 1)
		{
			ShowHistory();
		}
		else if(tNum == 2 && strcmp(tPoint[1], "-c") == 0)
		{
			ClearHistory();
		}
		else if(tNum == 2 && atoi(tPoint[1]) > 0)
		{
			//historyCopy = atoi(tPoint[1]);
			char *historyCommand = GetHistory( atoi(tPoint[1]) );
			
			if(historyCommand != NULL)
				ExecuteCommand(historyCommand);
		}
		else
		{
			printf("That history command was not supported.\n");
		}
	}
	else if(access(tPoint[0], X_OK) == 0) //OTHER EXECUTABLE
	{
		//RUN EXECUTABLE USING FORK
		
		pid_t processId = fork();
		
		if(processId == 0) //child process
		{
			//printf("Child Starting...\n");
			int childReturn = execv(tPoint[0], tPoint);
			perror("child had an accident");
			exit(1);
		}
		
		//wait for child to finish
		//printf("Parent waiting for child...\n");
		wait(NULL);
		//printf("\nChild finished!\n");
	}
	else
	{
		//print error
		printf("Could not find executable file %s\n", tPoint[0]);
	}
	
	
	if(ttok != NULL)
	{
		//recursively execute command
		
		//this is a really gross line of code. It gets a pointer of the command string
		//starting after the most recent pipe.
		char *nextCommand = &ttok[ strlen(ttok) + 2];
		ExecuteCommand( nextCommand );
	}
	
	free(command);
}



//----------------------------HISTORY FUNCTIOBNS-------------------------------------

//This function is great because it never has to shuffle things around in the history.
//Although, this makes viewing history and getting history elements a tad more complicated.
void SaveCommand( char *com )
{
	int ti = HistoryIndex % HISTORY_SIZE;
	if(History[ti] != NULL)
	{
		free(History[ti]);
	}
	
	int l = strlen(com);
	History[ti] = malloc(l);
	strcpy(History[ti], com);
	
	HistoryIndex++;
}

//prints all history
void ShowHistory()
{
	for(int i = 0; i < HISTORY_SIZE; i++)
	{
		int ti = (i + HistoryIndex) % HISTORY_SIZE;
		int di = HistoryIndex - HISTORY_SIZE + i;
		if(History[ti] != NULL){
			printf("%d  %s\n",di,History[ti]);
			di++;
		}
	}
	
}

//clears all history and resets history index to 0
void ClearHistory()
{
	HistoryIndex = 0;
	for(int i = 0; i < HISTORY_SIZE; i++)
	{
		if(History[i] != NULL)
		{
			free(History[i]);
			History[i] = NULL;
		}
	}
}

//returns a string of a past command based on the offset number
char * GetHistory(int num)
{
	//the -1 in this if statement prevents recursive history calls that could freeze the program.
	if(num < HistoryIndex - 1 && num > HistoryIndex - HISTORY_SIZE) 
	{
		return History[num % HISTORY_SIZE];
	}
	
	printf("Invalid history offset.\n");
	return NULL;
}