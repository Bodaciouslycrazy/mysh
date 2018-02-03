/* Bodie Malik - 2018

Notes to grader:

Only executed programs will write into pipes. Any internal command will just write to stdout
I don't think this is a problem since this is stated in the assignment.

Getting the history [offset] command to work was a pain. I got it to work mostly, but
there may still be glitches. I have a feeling that if your history is full,
trying to execute command "history [oldestCommand]" will not work. This is because the
oldest command is overwritten with this command before it runs. This will then get the program
stuck in a loop.

this was a crazy program to write. I think I went through at least 10 revisions
before I really nailed the process. I hope you like this program, because I put
my best effort into it.

Have a great day!
*/


/*
PROCESS IDEA
1- get input string from user
2- copy string into history
3- send to ExecuteCommand with null pipein and pipeout

ExecuteCommand recursively goes through every command starting from the front.
If it sees "|" it will create a pipe, write to it, and send it as the input to the next recursive call.
If the user uses a history offset call, it calls ExecuteCommand with the same input/output pipes as the current call.
*/

#include <fcntl.h>
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
void ExecuteCommand( char *commandin, int pipein[2], int pipeout[2]);
void ClearAllTokens();
void PrintTokens();
void SaveCommand( char *com );
void ShowHistory();
void ClearHistory();
char * GetHistory(int num);

//Global Variables
int ExitShell = 0;

char *History[HISTORY_SIZE];
int HistoryIndex = 0;


//-------------------------MAIN FUNCTION-----------------------------

int main(){
	printf("Welcome to Bodie's OS\n");
	
	//This loop breaks when ExitShell == 1.
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
		//teminate input string
		Command[i] = '\0';
		
		//----------SAVE STRING TO HISTORY----------
		//make sure to save a copy, not just the pointer since the string will be deconstructed
		
		SaveCommand(Command);
		
		//----------EXECUTE COMMAND-----------------
		//this recursive function takes care of everything else.
		
		ExecuteCommand(Command, NULL, NULL);
		
		//unallocate the command string
		free(Command);
	}
	
	//Clear up memory
	ClearHistory();
	
	return 0;
}



//Executes the command.
//called recursively for every pipe
//PARAMETERS:
//String commandin
//		The command you execute. Can include multiple commands, arguments, and pipes.
//int[2] pipein
//		File descriptors of the pipe the command should use as input.
//int[2] pipeout
//		File descriptors of the pipe the command should output to.
//		if this is null, but a pipe token is found, it will create its own pipe to output to.
void ExecuteCommand(char *commandin, int pipein[2], int pipeout[2])
{	
	//this is the pipe we output to if we need it
	int usemypipeout = 0;
	int mypipeout[2];
	
	//printf("EXECUTING COMMAND: %s\n", commandin);

	//------------------------------TURN COMMAND INTO TOKENS-----------------------------
	char *command = malloc( strlen(commandin));
	strcpy(command, commandin);
	
	//List of tokens
	char *tPoint[MAX_TOKENS + 1];
	int tNum = 0;
	
	//temp token
	char *ttok = strtok(command, " ");
	
	while(ttok != NULL)
	{
		if( strcmp(ttok, "|") == 0)
		{
			//turn on pipe
			//if pipeout is null, make new pipe
			if( pipe(mypipeout) != 0)
			{
				perror("Pipe error:");
			}
			else
			{
				//printf("Created pipe: [%d, %d]\n",mypipeout[0],mypipeout[1]);
				usemypipeout = 1;
			}
			
			//break since this is end of subcommand.
			break;
		}
		else if(tNum < MAX_TOKENS)
		{
			tPoint[tNum++] = ttok;
		}
		else
		{
			printf("Hit max tokens. Argument \"%s\" ignored.\n", ttok);
		}
		
		ttok = strtok(NULL, " ");
	}
	
	tPoint[tNum] = NULL;
	
	
	if(pipeout != NULL && usemypipeout == 0)
	{
		usemypipeout = 1;
		mypipeout[0] = pipeout[0];
		mypipeout[1] = pipeout[1];
	}
	
	//------------------------------------RUN TOKENS----------------------------------------
	if(tNum == 0) //if there are no tokens, skip
	{
		printf("That command has no arguments...\n");
	}
	else if(strcmp(tPoint[0], "exit") == 0) //EXIT
	{
		//When this command ends, the main loop will break.
		ExitShell = 1;
	}
	else if(strcmp(tPoint[0], "cd") == 0) //CHANGE DIRECTORY
	{
		if(tNum == 2)
		{
			//I hope this doesn't count as a system() call... does it?
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
			char *historyCommand = GetHistory( atoi(tPoint[1]) );
			
			if(historyCommand != NULL)
			{
				if(usemypipeout == 1) ExecuteCommand(historyCommand, pipein, mypipeout);
				else ExecuteCommand(historyCommand, pipein, pipeout);
			}
			else printf("Could not find history command!\n");
			
			//remember to not switch the pipes!
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
			
			//Change input to pipein
			if(pipein != NULL)
			{
				//closes currentin and replaces it with pipein
				dup2(pipein[0], 0);
			}
			//change output to mypipeout
			if(usemypipeout == 1)
			{
				//closes stdout and replaces it with mypipeout
				dup2(mypipeout[1], 1);
			}
			
			//printf("Child Starting...\n");
			int childReturn = execv(tPoint[0], tPoint);
			perror("child had an accident");
			exit(1);
		}
		
		//wait for child to finish
		//printf("Parent waiting for child...\n");
		wait(NULL);
		
		//close the pipe input and output that the child was using.
		if(pipein != NULL) close(pipein[0]);
		if(usemypipeout == 1) close( mypipeout[1] );
		//printf("\nChild finished!\n");
	}
	else
	{
		//print error
		printf("Could not find executable file %s\n", tPoint[0]);
	}
	
	int pipereturn[2];
	if(ttok != NULL)
	{
		//recursively execute command
		
		//this is a really gross line of code. It gets a pointer of the command string
		//starting after the most recent pipe.
		char *nextCommand = &ttok[ strlen(ttok) + 1];
		ExecuteCommand( nextCommand, mypipeout , pipeout);
	}
	
	free(command);
	
	return;
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
	if(num < HistoryIndex - 1 && num  > HistoryIndex - HISTORY_SIZE) 
	{
		return History[(num) % HISTORY_SIZE];
	}
	
	printf("Invalid history offset.\n");
	return NULL;
}