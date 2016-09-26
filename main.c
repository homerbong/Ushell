#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MSIZ 81

char *prompt="myshell> ";

int main (int argc, char **argv)
{
	int pid, id, i;
	char **arguments;
	char *token, *line;
	char *separator = " \t\n";
	
	
	
	arguments = (char **) malloc (MSIZ * sizeof(char *));
	fprintf(stderr,"%s",prompt);
	line = (char *) malloc(MSIZ * sizeof(char));
	
	
	
	while(fgets(line, MSIZ-1, stdin) != NULL);
	{
		token = strtok (line, separator);
		i = 0;
		arguments[i++] = token;
		while (token != NULL)
		{
			token = strtok (NULL, separator);
			arguments[i++] = token;
		} 
		arguments [i] = NULL;
		
		switch (pid = fork())
		{
			case 0:
				execvp(arguments[0], arguments);
				fprintf(stderr, "ERROR %s no such program!\n", line);
				exit(1);
			case -1:
				fprintf(stderr,"ERROR can't create child process!\n");   /* unlikely but possible if hit a limit */
	               		break;
	               		
	               	default:
	               		wait();
		}
	}
	
	
	
	
	
	
	
	return 0;
}