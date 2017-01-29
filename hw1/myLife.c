#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Warlon Zeng N11183332

// CONSTANTS
const int DEAD = 0;
const int ALIVE = 1;

// print the current generation
void printGen(int** lifeArr, int rows, int columns) {
	for (int i = 1; i < rows - 1; i++) {
		for (int j = 1; j < columns - 1; j++)
			if (lifeArr[i][j] == ALIVE)
				printf("%s", "*");
			else
				printf("%s", "-");
		printf("\n");
	}
}

// initalize generation 0
// we fill up the (rows + 2, columns + 2) 2D allocated array with file contents and print the generation
void initGen(int** lifeArr, int rows, int columns, char* filename) { // 10 rows, 10 columns

	// we are going to use this b/c size constraints of file unknown)
	char ch; // parse char by char
	int x = 1; // x coordinate
	int y = 1; // y coordinate
	FILE* file; // file pointer
	file = fopen(filename, "r"); // read mode

	if (file == NULL) {
		perror("Error opening file"); // file checking
	}
	else {
		while ((ch = fgetc(file)) != EOF) { // read until EOF
			if (ch == '\n') { // detect new line
				y++; // go next row
				x = 0; // reset starting to left
			}
			if (ch == '*')
				lifeArr[y][x] = ALIVE;
			x++; // go next column - next cell
		}
	}

	fclose(file); // free memory stream

	printf("Generation 0:\n");
	printGen(lifeArr, rows, columns);
}

// get neighbors sum
int getPeers(int** lifeArr, int i, int j) {

	int NORTH = lifeArr[i][j + 1];
	int SOUTH = lifeArr[i][j - 1];
	int WEST = lifeArr[i - 1][j];
	int EAST = lifeArr[i + 1][j];

	int NORTH_EAST = lifeArr[i + 1][j + 1];
	int SOUTH_EAST = lifeArr[i + 1][j - 1];
	int NORTH_WEST = lifeArr[i - 1][j + 1];
	int SOUTH_WEST = lifeArr[i - 1][j - 1];

	int sum = NORTH + SOUTH + WEST + EAST + NORTH_EAST + SOUTH_EAST + NORTH_WEST + SOUTH_WEST;
	return sum;
}

// create new generations in specified times
// initalize new 2D array outside of loop 
void newGen(int** lifeArr, int rows, int columns, int gens) {

	int adj = 0; // initialize adjs sum

	// // ========================
	// // START 2D array generation
	// // ========================
	// int** nextlifeArr = (int**) calloc(rows, sizeof(int**)); // pointer calloc to pointer calloc cast int - multidimensional array
	// for (int i = 0; i < rows; i++) {
	// 	nextlifeArr[i] = (int*) calloc(rows, sizeof(int*)); // fill it in with 1D array 
	// }
	// for (int i = 0; i < rows; i++)
	// 	for (int j = 0; j < columns; j++)
	// 		nextlifeArr[i][j] = DEAD;
	// // ========================
	// // END 2D array generation
	// // ========================


	for (int thisGen = 0; thisGen < gens; thisGen++) { // we allocate memory for new 2D array, because we don't want to change the original


		// ========================
		// START 2D array generation
		// ========================
		int** nextlifeArr = (int**)calloc(rows, sizeof(int**)); // pointer calloc to pointer calloc cast int - multidimensional array
		for (int i = 0; i < rows; i++) {
			nextlifeArr[i] = (int*)calloc(rows, sizeof(int*)); // fill it in with 1D array 
		}
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < columns; j++)
				nextlifeArr[i][j] = DEAD;
		// ========================
		// END 2D array generation
		// ========================


		// ========================
		// START processing
		// ========================
		for (int i = 1; i < rows - 1; i++) { // i did outer bounds padding so we have to start 1 cell deeper
			for (int j = 1; j < columns - 1; j++) {
				adj = getPeers(lifeArr, i, j); // for each cell, we calculate adjs sum

				if ((lifeArr[i][j] == ALIVE) && (adj == 2)) { // insert updated cell to new array based on rule
					nextlifeArr[i][j] = ALIVE; 
					continue;
				}

				if (adj == 3) {
					nextlifeArr[i][j] = ALIVE;
					continue;
				}


				if (adj < 2) {
					nextlifeArr[i][j] = DEAD;
					continue;
				}

				if (adj > 3) {
					nextlifeArr[i][j] = DEAD;
					continue;
				}

			}
		}
		// ========================
		// END processing
		// ========================


		// ========================
		// START updates
		// ========================
		free(lifeArr); // free up memory
		lifeArr = nextlifeArr; // update contents 
		printf("Generation %i:\n", thisGen+1);
		printGen(nextlifeArr, rows, columns);
		// ========================
		// END updates
		// ========================
	}
	// we free up the original lifeArr in main memory space
}

int main(int argc, char** argv) {
	// ========================
	// DEFAULTS
	// ========================
	int rows = 10;
	int columns = 10;
	int gens = 10;
	char* filename = "life";
	// ========================
	// DEFAULTS
	// ========================


	printf("%i", argc);
	printf("\n");
	printf("%s", argv[1]);
	printf("\n");


	// ========================
	// START COMMAND LINE parse
	// ========================
	if (argc == 2){
		filename = argv[1];
	}

	if (argc == 4){
		filename = argv[1];
		rows = atoi(argv[2]);
		columns = atoi(argv[3]);
	}

	if (argc == 5){
		filename = argv[1];
		rows = atoi(argv[2]);
		columns = atoi(argv[3]);
		gens = atoi(argv[4]);
	}
	// ========================
	// END COMMAND LINE parse
	// ========================


	// ========================
	// START padding
	// ========================
	rows = rows + 2;
	columns = columns + 2;
	// ========================
	// END padding
	// ========================


	// ========================
	// 2D array generation
	// ========================
	int** lifeArr = (int**) calloc(rows, sizeof(int**)); // pointer calloc to pointer calloc cast int - multidimensional array
	for (int i = 0; i < rows; i++) {
		lifeArr[i] = (int*) calloc(rows, sizeof(int*)); // fill it in with 1D array 
	}
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < columns; j++)
			lifeArr[i][j] = DEAD;
	// ========================
	// END 2D array generation
	// ========================

	initGen(lifeArr, rows, columns, filename);
	newGen(lifeArr, rows, columns, gens);

	free(lifeArr); // we are not using this anymore
	return 0;
}