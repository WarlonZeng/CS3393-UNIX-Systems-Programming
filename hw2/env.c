#include "env.h"

// Warlon Zeng N11183332

void print_environment (char** environment) {
	for (int i = 0; environment[i] != NULL; i++) { 
		puts(environment[i]);
	}
	exit(0);
}

int find_command_index(int argc, char **argv, bool if_i_flag) { // we need to find new environment size only for NO -i option
	if (if_i_flag) {
		for (int i = 2; i < argc; i++) { // starts from 2 because -i
			if (strchr(argv[i], '=') == NULL) { // we found command index
				return i;
			}
		}
	}
	else {
		for (int i = 1; i < argc; i++) { // starts from 1 because ./env
			if (strchr(argv[i], '=') == NULL) { // we found command index
				return i;
			}
		}
	}
	
	return argc; // what if we don't have an command? command_index == argc
}

int find_environment_size(char** environment) { // needed to modification of environment
	int size = 0;
	for (int i = 0; environment[i] != NULL; i++) {
		size++;
	}
	return size;
}

int main(int argc, char* argv[]) {
	// test cases
	// env
	// env -i
	// env -i ls -a
	// env -i FOO=BAR
	// env -i FOO=BAR FOO2=BAR2
	// env -i FOO=BAR FOO2=BAR2 ls -a
	// ..............

	// env ls -a
	// env FOO=BAR
	// env FOO=BAR ls -a

	bool if_i_flag = false;
	int command_index;

	if (argc == 1) { // issued without arguements -> displays the environment
		print_environment(environ);
    }

    if (!strcmp(argv[1], "-i")) { // if -i
    	if_i_flag = true;
    	command_index = find_command_index(argc, argv, if_i_flag); // either returns index of command or argc

    	char **new_environment = (char**) malloc((command_index - 2) * sizeof(char*));  // env -i FOO=BAR FOO2=BAR2 ls -a; 4 - 2
			
		for (int i = 0; i < command_index - 2; i++) { // dynamically allocate memory
			new_environment[i] = (char*) malloc(sizeof(char*));
		}

		for (int i = 2, j = 0; j < command_index - 2; i++, j++) { // fill it in with key value pairs
			new_environment[j] = argv[i];
		}
		new_environment[command_index] = NULL;

		environ = new_environment;

     	if (command_index != argc) { // execute if there is a command
			execvp(argv[command_index], &argv[command_index]);
			exit(0);

    	}
    	else { // do not execute if there is no command (just print)
    		print_environment(environ);
    		exit(0);
    	}
    }

    else { // create the new environement with additional key value pairs
    	command_index = find_command_index(argc, argv, if_i_flag);
    	int environment_size = find_environment_size(environ);

    	char **new_environment = (char**) malloc((environment_size + command_index - 1) * sizeof(char*));
			
		for (int i = 0; i < (environment_size + command_index - 1); i++) { // dynmaically allocate memory for modified environment
			new_environment[i] = (char*) malloc(sizeof(char*));
		}
		int i = 0;
		for (; i < environment_size; i++) { // fill up new environment with original environment
			new_environment[i] = environ[i];
		}
		for (int j = 1; i < environment_size + command_index - 1; i++, j++) { // fill up new environment with additional key value pairs
			new_environment[i] = argv[j];
		}
		new_environment[environment_size + command_index - 1] = NULL; // terminate with NULL

		environ = new_environment;

		if (command_index != argc) { // execute if there is a command
			execvp(argv[command_index], &argv[command_index]);
			exit(0);

    	}
    	else { // do not execute if there is no command (just print)
    		print_environment(environ);
    		exit(0);
    	}
	}

	return 0;
}