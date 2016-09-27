#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

#define MSIZ 81

char *prompt="myshell> ";
void redirect(char *fname, int inred, int outred);
int chkred(char *token, int *inred, int *outred);

int main (int argc, char **argv)
{
	int pid, i, inred, outred, save_in, save_out;
	char **arguments;
	char *token, *line, *fname;
	char *separator = " \t\n";
	
	if(argc != 1)
	{
		if(chdir(argv[1]) == -1)
		{
			printf("There is no such directory as %s, you will be redirected to home!\n", argv[1]);
			chdir(getenv("HOME"));
		}
	}
	else if(chdir(getenv("HOME")) == -1)
	{
		printf("ERROR!");
		exit(103);
	}
	
	
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
	
	save_in = dup(STDIN_FILENO);
	save_out = dup(STDOUT_FILENO);
	
	
	while(fgets(line, MSIZ-1, stdin) != NULL)
	{
		//fgets(line, MSIZ-1, stdin)
		token = strtok (line, separator);
		i = inred = outred = 0;
		arguments[i++] = token;
		while (token != NULL)
		{
			token = strtok (NULL, separator);
			if(chkred(token, &inred, &outred))
			{
				fname = strtok(NULL, separator);
				if(fname == NULL)
				{
					printf("ERROR while saving the path for redirecting!\n");
					exit(106);
				}
				
			}
			else
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
			if (inred || outred)
				redirect(fname, inred, outred);
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
		dup2(save_in, STDIN_FILENO);
		dup2(save_out, STDOUT_FILENO);
	}
	
	
	
	
	
	
	
	return 0;
}

void redirect(char *fname, int inred, int outred)
{
	int newdescriptor;
	
	if(inred == 1)
	{
		newdescriptor = open (fname, O_RDONLY, 0600);
		dup2(newdescriptor, STDIN_FILENO);
		close (newdescriptor);
	}
	
	if(outred == 1)
	{
		newdescriptor = open(fname,O_WRONLY|O_CREAT|O_TRUNC, 0600);
		dup2(newdescriptor, STDOUT_FILENO);
		close(newdescriptor);
	}
	
	
}

int chkred(char *token, int *inred, int *outred)
{
	
	if(token == NULL)
		return 0;
	
	else if(strcmp(token, "<") == 0)
		*inred = 1;
	else if(strcmp (token, ">") == 0)
		*outred = 1;
	
	return (*inred || *outred);
}