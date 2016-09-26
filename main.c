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
	
	if(argc != 1)
	{
		if(chdir(argv[1]) == -1)
		{
			printf("There is no such directory as %s, you will be redirected to home!\n", argv[1]);
			chdir(getenv("HOME"));
		}
	}
	else
		chdir(getenv("HOME"));
		
	
	arguments = (char **) malloc (MSIZ * sizeof(char *));
	if(arguments == NULL)
	{
		printf("ERROR 101 while allocating memory!");
		exit(101);
	}
	fprintf(stderr,"%s",prompt);
	line = (char *) malloc(MSIZ * sizeof(char));
	if(line == NULL)
	{
		printf("ERROR 102 while allocating memory!");
		exit(102);
	}
	
	
	
	while(fgets(line, MSIZ-1, stdin) != NULL)
	{
		//fgets(line, MSIZ-1, stdin)
		token = strtok (line, separator);
		i = 0;
		arguments[i++] = token;
		while (token != NULL)
		{
			token = strtok (NULL, separator);
			arguments[i++] = token;
		} 
		arguments [i] = NULL;
		
		if(strcmp(arguments[0], "cd") == 0)
		{
			if(arguments[1] == NULL)
			{
				chdir(getenv("HOME"));
			}
			else
				if(chdir(arguments[1]) == -1)
				{
					printf("%s: no such directory\n", arguments[1]);
				} 
		}
		
		else
			if(strcmp (arguments[0], "exit") == 0)
			{
				exit(0);
			}
		
		
		
		else
		{
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
		fprintf(stderr,"%s",prompt);
	}
	
	
	
	
	
	
	
	return 0;
}