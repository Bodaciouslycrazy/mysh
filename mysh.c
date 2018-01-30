//Bodie Malik

//TODO
//figure out how to organize built in functions.


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

#define SHOW_TOKENS false

//FUNCTION DECLARATIONS
void ClearAllTokens();
void PrintTokens();
void ExecuteCommand();


//GLOBAL VARIABLES
//pointer to tokens
char *tPoint[MAX_TOKENS];
int tNum = 0;
int ExitShell = 0;


//-------------------------MAIN FUNCTION-----------------------------
int main(){
	printf("Welcome to Bodie's OS\n");
	
	//This loop needs to break at some point.
	while(ExitShell == 0)
	{
		printf("$");
		
		//Get input string
		
		int tempSize = TEMP_CHUNK;
		char *tempInput = malloc(tempSize);
		char c;
		int i = 0;
		while((c = getchar()) != '\n' && c != EOF)
		{
			tempInput[i++] = c;
			
			if(i == tempSize)
			{
				tempSize += TEMP_CHUNK;
				tempInput = realloc(tempInput, tempSize);
			}
		}
		tempInput[i] = '\0';
		//int debugStringLength = strlen(tempInput);
		//printf("%s=%d\n",tempInput, debugStringLength);
		
		
		//--------------------------SAVE STRING TO HISTORY--------------------------------
		//make sure to save a copy, not just the pointer since the string will be deconstructed
		
		
		ClearAllTokens();
		
		//Interpret Tokens!
		tNum = 0;
		char *tempToken = strtok(tempInput, " ");
		
		while( tempToken != NULL )
		{
			if( strcmp(tempToken, "|") == 0)
			{
				//execute command and output to file
				printf("NOT IMPLEMENTED PIPING YET! This command will be ignored.\n");
				
				//reset tokens
				ClearAllTokens();
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
					printf("You have reached the max arguments for this command.\n");
					break;
				}
			}
			
			tempToken = strtok(NULL, " ");
		}
		
		PrintTokens();
		
		//Execute command
		ExecuteCommand();
		
		//clean up memory
		free(tempInput);
	}
	
	//Clear up memory?
	
	

	return 0;
}


//Executes the command currently described by the tokens.
void ExecuteCommand()
{
	if(tNum == 0) //if there are no tokens, skip
		return;
	//BUILT IN COMMANDS
	if(strcmp(tPoint[0], "exit") == 0) //EXIT
	{
		//BREAK FROM LOOP, ENDING SHELL
		ExitShell = 1;
		return;
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
			printf("NOT IMPLEMENTED YET!\n");
		}
		else if(tNum == 2 && strcmp(tPoint[1], "-c") == 0)
		{
			printf("NOT IMPLEMENTED YET!\n");
		}
		//else if(){} //CONTINUE HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
}



//deletes all tokens.
void ClearAllTokens()
{
	for(int i = 0; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			
			//memmory does not need to be free since strtok does not allocate new memory.
			tPoint[i] = NULL;
		}
	}
	
	tNum = 0;
	
	#if SHOW_TOKENS
	/*
	printf("Tokens after clear...\n");
	PrintTokens();
	printf("Clear Complete\n");
	*/
	#endif
}


//Prints every token currently stored.
//Also prints their length.
//Used mainly for debugging.
void PrintTokens()
{
	//Debug code to show tokens.
	#if SHOW_TOKENS
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
