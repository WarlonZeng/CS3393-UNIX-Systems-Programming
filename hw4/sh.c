#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LEN 1024
#define MAX_ARGS 64

/*
	Prompt environment string from PS1, else custom made.
	Handles I/O redirection by searching for ">", "<". ">>" and "<<" are not required.
	PID and dup2.
	Built-in cd and exit support.
	Parse for tokens with strtok.
	P.S. "> output cmd" DOES NOT WORK ON OFFICIAL BASH so not support for it.
*/

void io_redirect(char** argv);
bool builtin(char** argv);
void parse(char* buff, char** argv);
void updatePWDs();
void updateArgv(int index, char** argv);
void execute(char** argv);

int main() {

	char line[MAX_LEN];
	char* argv[MAX_ARGS];

	char* prompt = getenv("PS1"); // if provided
	if (!prompt) { 
		prompt = "$ "; // else use this
	}

	while (1) {

		int saved_stdout = dup(1); // duplicate 1:stdin into 3

		printf ("%s ", prompt);

		if (fgets (line, MAX_LEN, stdin) == NULL) { 
	    	printf ("input error\n");
	    	exit(1);
	    }

	    parse(line, argv); // get tokens into array. \n then NULL terminated
	    //builtin(argv); // check builtin functions. not updating PWD because it is not required.
	    if (builtin(argv)) {
	    	dup2(saved_stdout, 1); // duplicate 1 into 3
	    	close(saved_stdout);
	    	continue;
	    }
	    io_redirect(argv); // dup(1) becomes some weird ass fd
	    execute(argv);

	    dup2(saved_stdout, 1); // duplicate 1 into 3
	    close(saved_stdout);

    }

    return 0;
}

void parse(char* line, char** argv) {
	line[strlen(line) -1 ] = '\0'; // remove \n before token processing.. write off NULL
	line[strlen(line) ] = '\0'; // remove \n before token processing.. write off NULL

	char* delim = " ";
	char* arg;

	arg = strtok(line, delim);

	int index = 0;
	argv[index] = arg;
	index++;

	while (arg != NULL) {
		arg = strtok(NULL, delim);
		argv[index] = arg;
		index++;
	}
}

bool builtin(char** argv) {

	if (!strcmp(argv[0], "exit")) // if there is exit
		exit(0);

	if (!strcmp(argv[0], "cd")) {
		if (argv[1] == NULL) { // cd by itself
			if (!chdir(getenv("HOME"))) {
				perror("chdir");
			}
			updatePWDs();
		}
		else {
			if (!chdir(argv[1])) {
				perror("chdir");
			}
			updatePWDs();
		}
		return true;
	}
	return false;
}

void updatePWDs() {
	char* oldPWD;
	oldPWD = getenv("PWD");
	setenv("OLDPWD", oldPWD, 1);

	char* newPWD[MAX_LEN];
	getcwd(newPWD, MAX_LEN); // getcwd requires a buffer and a size of buffer
	setenv("PWD", newPWD, 1);

	puts(getenv("OLDPWD")); // proof that PWD changed
	puts(getenv("PWD"));
}

void io_redirect(char** argv) {
	int fdin;
	int fdout;
	int i = 0;

	for (; argv[i] != NULL; i++) { // this assumes ONE redirection per line. 
		
		if (!strcmp(argv[i], "<")) { // read in
			fdin = open(argv[i + 1], O_RDONLY);
			if (fdin < 0) {
				perror("file error");
			}
			if (dup2(fdin, 0) < 0) {
				perror("dup2 failed");
			}
			updateArgv(i, argv);
			close(fdin);
			break;
		}

		if (!strcmp(argv[i], ">")) { // write out

			fdout = open(argv[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666); // 4
			if (fdout < 0) {
				perror("file error");
			}

			if (dup2(fdout, 1) < 0) { // 1 becomes 4;
				perror("dup2 failed");
			}

			updateArgv(i, argv);
			close(fdout); // drop 4;
			break;
		}
	
	}

}

void updateArgv(int index, char** argv) { 
	for (; argv[index + 1] != NULL; index++) { // support for things like ls > out.txt out.txt
		argv[index] = argv[index + 2]; // covers 3+ args
	}
	argv[index] = NULL;
}

void execute(char** argv) {
	pid_t pid;
    int status;

    if ((pid = fork()) < 0) {     
    	printf("child fork failed\n");
    	exit(1);
    }
    
    if (pid == 0) {          
    	if (execvp(*argv, argv) < 0) {
    		printf("exec failed\n");
    		exit(1);
        }
    }

    while (wait(&status) != pid);
}