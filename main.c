#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSIZ 81
#define LSZ 10

void redirect(char **fname, int inred, int outred, int append);
int chkred(char *token, int *inred, int *outred, int *append, int *pipe_flag);
int chkbuiltin(char **arguments);
void tokenize(char ** arguments, char **fname,char *line, int *inred, int *outred, int *append, int *pipe_flag);
void memoryalloc(char ***arguments, char **line, char ***fname);
void launchjob(char **arguments, char *line, char **fname, int inred, int outred, int append, int pipe_flag);
void log_update(char *line, char **log, int *n);
char **logalloc();
void create_pipe(char **arguments, int pipe_flag);

int main (int argc, char **argv)
{
	int inred, outred, save_in, save_out, append, n=0, i, pipe_flag;
	char **arguments;
	char *line, **fname, **log;

	if(argc != 1) {               //Checks if there is a directory from which to start the shell.
		if(chdir(argv[1]) == -1) {
			printf("There is no such directory as %s, you will be redirected to home!\n", argv[1]);
			chdir(getenv("HOME"));
		}
	} else if(chdir(getenv("HOME")) == -1) {
		printf("ERROR setting the default path\n");
		exit(103);
	}

	memoryalloc(&arguments, &line, &fname);
	log = logalloc();

	//Duplicates of the standard input and output file descriptors. They are used to restore the shell to the initial after a redirection was made.
	save_in = dup(STDIN_FILENO);
	save_out = dup(STDOUT_FILENO);

	fprintf(stdout,"[%s]",getenv("USER"));
	while(fgets(line, MSIZ-1, stdin) != NULL) {
		log_update(line, log, &n);
		tokenize(arguments, fname, line, &inred, &outred, &append, &pipe_flag);

		if(!chkbuiltin(arguments)) {
			launchjob(arguments, line, fname, inred, outred, append, pipe_flag);
		}


		//Restoring the standard input and output file descriptors.
		dup2(save_in, STDIN_FILENO);
		dup2(save_out, STDOUT_FILENO);
		fprintf(stdout,"[%s]",getenv("USER"));
	}







	return 0;
}


/* This function is responsible of redirecting the input or output of the commands. In the
case one of the two flags are set it just opens a new file descriptor and it duplicates the
ones of standard input and output*/
void redirect(char **fname, int inred, int outred, int append)
{
	int newdescriptor;

	if(inred == 1) {
		newdescriptor = open (fname[0], O_RDONLY, 0600);
		if(dup2(newdescriptor, STDIN_FILENO) == -1) {
			printf("ERROR while redirecting the input\n");
			exit(110);
		}
		close (newdescriptor);
	}

	if(outred == 1) {
		newdescriptor = open(fname[1],O_WRONLY|O_CREAT|O_TRUNC, 0600);
		if(dup2(newdescriptor, STDOUT_FILENO) == -1) {
			printf("ERROR while redirecting the output\n");
			exit(111);
		}
		close(newdescriptor);
	}

	if(append == 1) {
		newdescriptor = open(fname[2],O_WRONLY|O_APPEND);
		if(dup2(newdescriptor, STDOUT_FILENO) == -1) {
			printf("ERROR while redirecting the output(append)\n");
			exit(112);
		}
		close(newdescriptor);
	}

}

//It just compares the tokens with the corresponding characters and sets the values of the redirection flags.
int chkred(char *token, int *inred, int *outred, int *append, int *pipe_flag)
{

	if(token == NULL)
		return 0;

	else if(strcmp(token, "<") == 0) {
		*inred = 1;
		return 1;
	} else if(strcmp (token, ">") == 0) {
		*outred = 1;
		return 2;
	} else if(strcmp(token, ">>") == 0) {
		*append = 1;
		return 3;
	} else if(strcmp(token, "|") == 0) {
		*pipe_flag = 1;
		return 4;
	}

	return 0;
}


int chkbuiltin(char **arguments)
{
	//checking for builtin commands "cd" and "exit".
	if(strcmp(arguments[0], "cd") == 0) {
		if(arguments[1] == NULL) {
			chdir(getenv("HOME"));
		} else if(chdir(arguments[1]) == -1) {
			printf("%s: no such directory\n", arguments[1]);
		}
		return 1;
	}

	else if(strcmp (arguments[0], "exit") == 0) {
		exit(0);
	}

	return 0;
}


void tokenize(char **arguments, char **fname, char *line, int *inred, int *outred, int *append, int *pipe_flag)
{
	//The input is tokenized and progressively processed in order to execute a command.
	int i;
	char *token;
	char *separator = " \t\n";
	token = strtok (line, separator);
	i = *inred = *outred = *append =  0;
	*pipe_flag = -1;
	arguments[i++] = token;
	while (token != NULL) {
		token = strtok (NULL, separator);

		//Controls if there is a IO redirection request in command. If one "<", ">" or ">>" is found in the command then one of the three corresponding flags are consequently set.
		switch(chkred(token, inred, outred, append, pipe_flag)) {
		case 1: {
			fname[0] = strtok (NULL, separator);
			if(fname[0] == NULL) {
				printf("ERROR while saving the path for redirection\n");
				exit(141);
			}
			break;
		}
		case 2: {
			fname[1] = strtok (NULL, separator);
			if(fname[1] == NULL) {
				printf("ERROR while saving the path for redirection\n");
				exit(141);
			}
			break;
		}
		case 3: {
			fname[2] = strtok (NULL, separator);
			if(fname[2] == NULL) {
				printf("ERROR while saving the path for redirection\n");
				exit(141);
			}
			break;
		}
		case 4: {
			arguments[i++] = NULL;
			*pipe_flag = i;
			break;
		}
		default:
			arguments[i++] = token;
		}
	}
	arguments [i] = NULL;
	return;
}

void memoryalloc(char ***arguments, char **line, char ***fname)
{
	*arguments = (char **) malloc (MSIZ * sizeof(char *));
	if(*arguments == NULL) {
		printf("ERROR 101 while allocating memory!\n");
		exit(101);
	}


	*line = (char *) malloc(MSIZ * sizeof(char));
	if(*line == NULL) {
		printf("ERROR 102 while allocating memory!\n");
		exit(102);
	}

	*fname = (char **) malloc(3 * sizeof(char*));
	if (*fname == NULL) {
		printf("ERROR 103 while allocating memory!\n");
		exit(103);
	}

	return;
}

void launchjob(char **arguments, char *line, char **fname, int inred, int outred, int append, int pipe_flag)
{
	int pid, child_pid;

	if(pipe_flag != -1) {

		switch(pid = fork()) {
		case 0:
			create_pipe(arguments, pipe_flag);
			break;

		case -1:
			fprintf(stderr,"ERROR can't create child process!\n");
			break;

		default:
			while(child_pid = wait(NULL) != -1);
			if(errno != ECHILD) {
				printf("There has been a problem waiting for the children!\n");
				exit(107);
			}
		}
	}

	else {
		//checking if the IO redirecting flags are set in order to perform the necessary operations.
		if (inred || outred || append)
			redirect(fname, inred, outred, append);
		//Creation of the child process.
		switch (pid = fork()) {
		case 0:

			execvp(arguments[0], arguments);
			fprintf(stderr, "ERROR %s no such program!\n", line);
			exit(1);
		case -1:


			fprintf(stderr,"ERROR can't create child process!\n");
			break;

		default:
			//Waits for all the children to terminate.
			while(child_pid = wait(NULL) != -1);
			if(errno != ECHILD) {
				printf("There has been a problem waiting for the children!\n");
				exit(107);
			}

		}
	}
}

void log_update(char *line, char **log, int *n)
{
	FILE *fp;
	int i = 0;
	if (strcpy(log[*n%LSZ], line) == NULL) {
		printf("ERROR while saving the log!\n");
		exit(1984);
	}
	fp = fopen ("/tmp/audit.log", "w");
	if(fp == NULL) {
		printf("ERROR while opening the log file!\n");
		exit(180);
	}
	if(*n < LSZ) {
		for(i=0; i <= *n; i++) {
			fprintf(fp,"%s", log[i]);
		}
	} else {
		for(i=((*n+1)%LSZ); i != (*n%LSZ) ; i=(i+1)%LSZ) {
			fprintf(fp,"%s", log[i]);
		}
		fprintf(fp,"%s", log[*n%LSZ]);
	}
	fclose(fp);
	(*n)++;
	return;
}

char **logalloc()
{
	int i;
	char **tmp;
	tmp = (char **) malloc(LSZ * sizeof(char*));
	if(tmp == NULL) {
		printf("ERROR 104 while allocating memory!\n");
		exit(104);
	}

	for(i=0; i < LSZ; i++) {
		tmp[i] = (char*) malloc (MSIZ * sizeof(char));
		if(tmp[i] == NULL) {
			printf("ERROR 104.%d while allocating memory!\n", i+1);
			exit(105);
		}
	}
	return tmp;
}

void create_pipe(char **arguments, int pipe_flag)
{
	char **arguments_2;
	int fd[2], i, pid, child_pid;

	arguments_2 = &arguments[pipe_flag];

	if(pipe(fd) == -1) {
		printf("ERROR while opening the pipe!\n");
		exit(204);
	}

	switch(pid = fork()) {
	case 0:
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		execvp(arguments[0], arguments);
		fprintf(stderr, "ERROR %s no such program!\n", arguments[0]);
		exit(1);

	case -1:
		fprintf(stderr,"ERROR can't create child process!\n");
		break;

	default:
		close(fd[1]);
		dup2(fd[0], 0);
		close(fd[0]);
		execvp(arguments_2[0], arguments_2);
		fprintf(stderr, "ERROR %s no such program!\n", arguments_2[0]);
		exit(1);
	}



}
