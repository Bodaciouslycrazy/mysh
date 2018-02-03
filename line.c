//Bodie Malik
//reads from stdin
//gives each char its own line
//ends at EOF or \n

#include <stdio.h>

int main()
{
	char c;
	while((c = getchar()) != EOF && c != '\n')
	{
		printf("***%c***\n",c);
	}
}
