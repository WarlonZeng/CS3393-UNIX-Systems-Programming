/*	msh - Max's Shell
 *	Created by: Max Lebedev
 *	This is a simple shell designed to read input,
 *	tokenize it, and exec it.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 1024
#define MAX_TOK 64

int main (int argc, char* argv[]){
	puts(getenv("PWD"));
	chdir(getenv("HOME"));

	char* cwd[MAX_LEN];
	getcwd(cwd, MAX_LEN);
	setenv("PWD", cwd, 1);
	puts(getenv("PWD"));
}