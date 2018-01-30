//Bodie Malik

//TODO
//implement getcwd and chdir as cd command.

//figure out how to organize built in functions.
//SHELL Source Code
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
char * TrimToken(char *orig, int length);
void GetTokens();
void ClearAllTokens();


//GLOBAL VARIABLES
//pointer to tokens
char *tPoint[MAX_TOKENS];
int tNum = 0;


//-------------------------MAIN FUNCTION-----------------------------
int main(){
	printf("Welcome to Bodie's OS\n");
	
	//This loop needs to break at some point.
	while(true)
	{
		
		GetTokens();
		
		//TOKENS HAVE BEEN RECIEVED. EXECUTE COMMAND
		
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
		
		
		//EXECUTE COMMAND
		if(tNum == 0) //if there are no tokens, skip rest of loop
			continue;
		//BUILT IN COMMANDS
		if(strcmp(tPoint[0], "exit") == 0) //EXIT
		{
			//BREAK FROM LOOP, ENDING SHELL
			break;
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
		
		//ADD COMMAND TO HISTORY
		printf("NOT IMPLEMENTED: ADD COMMAND TO HISTORY\n");
		
	}
	
	//Clean up memory.
	for(int i = 0; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			free(tPoint[i]);
		}
	}
	

	return 0;
}

//Reads a line from the user and splits inputs into tokens.
//All output will be in tPoint and tNum
void GetTokens()
{
	ClearAllTokens();
	printf("$");
	
	//temp variables.
	char *tempToken = malloc(TEMP_CHUNK);
	int tempSize = TEMP_CHUNK;
	int c = EOF;
	int charCount = 0;
	int tokenCount = 0;
	bool startedToken = false;
	
	//read input until end line/file
	do
	{
		c = getchar();
		
		if((c == EOF || c == '\n' || c == ' ') && startedToken) //complete token
		{	
			//Point to new token
			tPoint[tokenCount] = TrimToken(tempToken, charCount);

			//Reset temp token
			tempSize = TEMP_CHUNK;
			tempToken = realloc(tempToken, tempSize);

			//reset other variables
			tokenCount++;
			if(tokenCount >= MAX_TOKENS)
			{
				//do not read any more arguments
				c = EOF;
			}

			startedToken = false;
			charCount = 0;
		}
		else if( c != ' ' )//Token has not been completed, add to temp
		{
			tempToken[charCount++] = (char)c;

			//reallocate tempToken if it is not big enough.
			if(charCount == tempSize)
			{
				tempSize += TEMP_CHUNK;
				tempToken = realloc(tempToken, tempSize);
			}
			
			startedToken = true;
		}

	}while(c != '\n' && c != EOF);

	tNum = tokenCount;
	
	//free the temp token.
	free(tempToken);
	
	//delete tokens from previous commands.
	for(int i = tokenCount; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			free(tPoint[i]);
			tPoint[i] = NULL;
		}
	}
}

void ClearAllTokens()
{
	for(int i = 0; i < MAX_TOKENS; i++)
	{
		if(tPoint[i] != NULL)
		{
			free(tPoint[i]);
			tPoint[i] = NULL;
		}
	}
}

//Trims a string to a certain length and returns a new pointer.
char * TrimToken( char *orig, int length)
{
	
	//printf("trimming to %d chars.\n",length);
	char *trimmed = malloc(length + 1);

	for(int i = 0; i < length; i++)
	{
		trimmed[i] = orig[i];
	}
	
	trimmed[length] = '\0';

	return trimmed;
}
