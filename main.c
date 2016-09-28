#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

#define MSIZ 81

void redirect(char *fname, int inred, int outred);
int chkred(char *token, int *inred, int *outred);

int main (int argc, char **argv)
{
	int pid, child_pid, i, inred, outred, save_in, save_out;
	char **arguments;
	char *token, *line, *fname;
	char *separator = " \t\n";
	
	if(argc != 1)                 //Checks if there is a directory from which to start the shell.
	{
		if(chdir(argv[1]) == -1)
		{
			printf("There is no such directory as %s, you will be redirected to home!\n", argv[1]);
			chdir(getenv("HOME"));
		}
	}
	else if(chdir(getenv("HOME")) == -1)
	{
		printf("ERROR setting the default path\n");
		exit(103);
	}
	
	
	
	arguments = (char **) malloc (MSIZ * sizeof(char *));
	if(arguments == NULL)
	{
		printf("ERROR 101 while allocating memory!\n");
		exit(101);
	}
	
	
	line = (char *) malloc(MSIZ * sizeof(char));
	if(line == NULL)
	{
		printf("ERROR 102 while allocating memory!\n");
		exit(102);
	}
	
	
	//Duplicates of the standard input and output file descriptors. They are used to restore the shell to the initial after a redirection was made.
	save_in = dup(STDIN_FILENO);
	save_out = dup(STDOUT_FILENO);
	
	fprintf(stdout,"[%s]",getenv("USER"));
	while(fgets(line, MSIZ-1, stdin) != NULL)
	{
		//The input is tokenized and progressively processed in order to execute a command.
		token = strtok (line, separator);
		i = inred = outred = 0;
		arguments[i++] = token;
		while (token != NULL)
		{
			token = strtok (NULL, separator);
			//Controls if there is a IO redirection request in command. If one "<" or ">" is found in the command then one of the two corresponding flags are consequently set.
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
		
		//checking for builtin commands "cd" and "exit".
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
			//checking if the IO redirecting flags are set in order to perform the necessary operations.
			if (inred || outred)
				redirect(fname, inred, outred);
			//Creation of the child process.
			switch (pid = fork())
			{
				case 0:
					
					execvp(arguments[0], arguments);
					fprintf(stderr, "ERROR %s no such program!\n", line);
					exit(1);
				case -1:
					
					fprintf(stderr,"ERROR can't create child process!\n");   
		               		break;
		               		
		               	default:
		               		
		               		while(child_pid = wait(NULL) != -1);
		               		if(errno != ECHILD)
		               		{
		               			printf("There has been a problem waiting for the children!\n");
		               			exit(107);
		               		}
			}
		}
		
		fprintf(stdout,"[%s]",getenv("USER"));
		//Restoring the standard input and output file descriptors.
		dup2(save_in, STDIN_FILENO);
		dup2(save_out, STDOUT_FILENO);
	}
	
	
	
	
	
	
	
	return 0;
}


/* This function is responsible of redirecting the input or output of the commands. In the 
case one of the two flags are set it just opens a new file descriptor and it duplicates the
ones of standard input and output*/
void redirect(char *fname, int inred, int outred)
{
	int newdescriptor;
	
	if(inred == 1)
	{
		newdescriptor = open (fname, O_RDONLY, 0600);
		if(dup2(newdescriptor, STDIN_FILENO) == -1)
		{
			printf("ERROR while redirecting the input");
			exit(110);
		}
		close (newdescriptor);
	}
	
	if(outred == 1)
	{
		newdescriptor = open(fname,O_WRONLY|O_CREAT|O_TRUNC, 0600);
		if(dup2(newdescriptor, STDOUT_FILENO) == -1)
		{
			printf("ERROR while redirecting the input");
			exit(110);
		}
		close(newdescriptor);
	}
	
	
}

//It just compares the tokens with the corresponding characters and sets the values of the redirection flags.
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