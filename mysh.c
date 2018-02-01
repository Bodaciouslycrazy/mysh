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
	
*/


#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TOKENS 16
#define TEMP_CHUNK 64
#define HISTORY_SIZE 100

#define SHOW_TOKENS false

//FUNCTION DECLARATIONS
void ClearAllTokens();
void PrintTokens();
void SaveCommand( char *com );
void ExecuteCommand();
void ShowHistory();
void ClearHistory();
char * GetHistory(int num);


//GLOBAL VARIABLES
//pointer to tokens
char *Command;
int CommandSize = 0;

char *tPoint[MAX_TOKENS];
int tNum = 0;
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
		CommandSize = TEMP_CHUNK;
		Command = malloc(CommandSize);
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
		
		//Tokenize!!!
		ClearAllTokens();
		char *tempToken = strtok(Command, " ");
		int finished = 0;
		do
		{
			if( tempToken == NULL)
			{
				//execute without pipe
				ExecuteCommand( "\0" );
				
				if(Command == NULL || strlen(Command) <= 0) //nothing more to execute
				{
					finished = 1;
				}
				else //last command added something to command string. Continue executing.
				{
					ClearAllTokens();
					
					tempToken = strtok(Command, " ");
				}
			}
			else if( strcmp(tempToken, "|") == 0)
			{
				printf("NOT IMPLEMENTED PIPING YET! This command will be run without a pipe.\n");
			
				//execute with pipe
				ExecuteCommand( &tempToken[2] );
				
				//reset tokens
				ClearAllTokens();
				
				tempToken = strtok(Command, " ");
			}
			else
			{
				//Add token to token list.
				if(tNum < MAX_TOKENS)
				{
					tPoint[tNum++] = tempToken;
				}
				else
				{
					printf("You have reached the max arguments for this command. Argument ignored.\n");
				}
				
				tempToken = strtok(NULL, " ");
			
			}
		}while(finished == 0);
		
		//clean up memory
		free(Command);
		Command = NULL;
	}
	
	//Clear up memory?
	
	

	return 0;
}


//Executes the command currently described by the tokens.
void ExecuteCommand(char *commandEnd)
{
	PrintTokens();
	
	int historyCopy = -1;
	
	if(tNum == 0) //if there are no tokens, skip
	{
		printf("That command has not arguments...\n");
	}
	if(strcmp(tPoint[0], "exit") == 0) //EXIT
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
			historyCopy = atoi(tPoint[1]);
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
	
	
	//delete string from Command.
	char *tempCommand = Command;
	Command = malloc(CommandSize);
	strcpy(Command, commandEnd);
	free(tempCommand);
	
	if(historyCopy > 0)
	{
		
		char *hp = GetHistory(historyCopy);
		if(hp != NULL)
		{
			tempCommand = Command;
			CommandSize += strlen(hp) + 3;
			Command = malloc(CommandSize);
			strcpy(Command, hp);
			strcat(Command, tempCommand);
			free(tempCommand);
		}
	}
	
	//debug stuff
	//printf("NEW COMMAND: %s\n", Command);
}



//deletes all tokens.
void ClearAllTokens()
{
	for(int i = 0; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			tPoint[i] = NULL;
		}
	}
	
	tNum = 0;
}


//Prints every token currently stored.
//Also prints their length.
//Used mainly for debugging.
void PrintTokens()
{
	//Debug code to show tokens.
	#if SHOW_TOKENS
	printf("Printing Tokens:\n");
	for(int i = 0; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			int tl = strlen(tPoint[i]);
			printf("%s-%d\n", tPoint[i], tl);
		}
	}
	#endif
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

void ShowHistory()
{
	int di = 0;
	for(int i = 0; i < HISTORY_SIZE; i++)
	{
		int ti = (i + HistoryIndex) % HISTORY_SIZE;
		if(History[ti] != NULL){
			printf("%d  %s\n",di,History[ti]);
			di++;
		}
	}
}

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

char * GetHistory(int num)
{
	int di = 0;
	for(int i = 0; i < HISTORY_SIZE; i++)
	{
		int ti = (i + HistoryIndex) % HISTORY_SIZE;
		
		if(History[ti] != NULL && di == num)
		{
			return History[ti];
		}
		else if(History[ti] != NULL)
		{
			di++;
		}
	}
	
	printf("We could not find that history command.\n");
	return NULL;
}