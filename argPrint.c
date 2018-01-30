//Bodie Malik
//this program simply prints all arguments.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	printf("Program name is %s\n", argv[0]);

	for(int i = 1; i < argc; i++)
	{
		printf("arg %d: %s\n",i,argv[i]);
	}
}
