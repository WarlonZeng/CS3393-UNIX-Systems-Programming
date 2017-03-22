#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <unistd.h>
#include <inttypes.h>

#define INODES_CAPACITY 20
#define DIRSIZE 4

/*
    Create dynamic array of inodes.
    Each index holds an UNIQUE inode number.
    Dynamic array needs a size and capacity to 
    expand when size closes in on capacity.
*/
typedef struct {
	size_t* array;
	int size;
	int capacity;
} inodes;


int traverseDir(char* dirname, inodes* arrInodes);
int fileStats(char* dir, inodes* arrInodes);
void resize(inodes* arrInodes);
void buildPath(char* dest, char* path, char* entry);

int main (int argc, char* argv[]){
	inodes arrInodes;
	arrInodes.capacity = INODES_CAPACITY;
	arrInodes.size = 0;
	arrInodes.array = malloc(sizeof(size_t) * arrInodes.capacity);

	char path[PATH_MAX + 1];

	if (argc == 1) {
		int blockSize = traverseDir(".", &arrInodes);
		printf("%d %s\n", blockSize, ".");
	}

	else if (argc == 2) {
		if (!realpath(argv[1], path)){
			perror("Path could not be resolved");
		}
		int blockSize = traverseDir(path, &arrInodes);
		printf("%d %s\n", blockSize, path);
	}
	
	return 0;
}

/*
	Create the directory name. 
	dirent -> d_name does not give full path name.
	Need to store recursively.
*/
void buildPath(char* dest, char* path, char* entry){
	strncat(dest, path, strlen(path));
	strncat(dest, "/" , 1);
	strncat(dest, entry, strlen(entry));
}

/*
	Traverse directory tree by directory name.
	Includes ALL files.
*/
int traverseDir(char* dirname, inodes* arrInodes){ 
	DIR *dirp = opendir(dirname);
	struct dirent *direntp;

	int dirLen = 0;
	int totalBlockSize = 0;
	int size = 0;
	char *temp = NULL;

	while((direntp = readdir(dirp)) != NULL){

		if (direntp -> d_name[0] != '.' || (direntp -> d_name[1] >= 'a' && direntp -> d_name[1] <= 'z')) {
			dirLen = strlen(dirname) + strlen(direntp -> d_name) + 1;
			temp = (char*) calloc(dirLen,  sizeof(char*));
			buildPath(temp, dirname, direntp -> d_name);

			if(direntp -> d_type == DT_DIR){
				size = traverseDir(temp, arrInodes);
				totalBlockSize += size;
				printf("%d \t %s\n", size, temp);
			}

			else{ 
				totalBlockSize += fileSize(temp, arrInodes);
			}
		}
		else if (direntp -> d_name[1] == '\0'){
			totalBlockSize += DIRSIZE;
		}
	}

	free(temp);
	return totalBlockSize;
}


/*
	Take in unique hardlinks. 
	Return 0 otherwise.
*/
int fileSize(char* file, inodes* arrInodes) {
	struct stat buf;
	
	if (!lstat(file, &buf)) {
		perror("lstat command failed");
	}

	if (buf.st_nlink > 1) {

		for(int i = 0; i < arrInodes -> size; i++) {
			if((size_t) buf.st_ino == arrInodes -> array[i]) {
				return 0;
			}
		}
		
		arrInodes -> array[arrInodes -> size] = buf.st_ino;
		arrInodes -> size++;

	}

	if(arrInodes -> size == arrInodes -> capacity){ 
		resize(arrInodes);
	}

	return buf.st_blocks;
}


/*
	Double the capacity.
	Resize the dynamic array of inodes.
*/
void resize(inodes* arrInodes) {
	size_t* newArrayInodes;
	arrInodes -> capacity = arrInodes -> capacity * 2;
	newArrayInodes = malloc(sizeof(size_t) * arrInodes -> capacity);

	free(arrInodes -> array);
	arrInodes -> array = newArrayInodes;
}