#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char** environ;

void print_environment (char** environment);
int find_command_index(int argc, char **argv, bool if_i_flag);
int find_environment_size(char** environment);