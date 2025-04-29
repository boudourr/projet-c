#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <dirent.h>

#include <sys/stat.h>

#include "chap1.c"

typedef struct {

	char* name;

	char* hash;

	int mode;

} WorkFile;



typedef struct {

	WorkFile* tab;

	int size;

	int n;

} WorkTree;



#define WORK_TREE_SIZE 1000



// Function to get file permissions

int getChmod(const char *path) {

	struct stat ret;

	if (stat(path, &ret) == -1) {

    	return -1;

	}

	return

	(ret.st_mode & S_IRUSR)|(ret.st_mode & S_IWUSR)|(ret.st_mode & S_IXUSR)|/*owner*/

	(ret.st_mode & S_IRGRP)|(ret.st_mode & S_IWGRP)|(ret.st_mode & S_IXGRP)|/*group*/

	(ret.st_mode & S_IROTH)|(ret.st_mode & S_IWOTH)|(ret.st_mode & S_IXOTH);/*other*/

}



// Function to set file permissions

void setMode(int mode, char* path) {

	char buff[100];

	sprintf(buff, "chmod %d %s", mode, path);

	system(buff);

}



// Helper functions from list_implementation.c

char* hashToPath(char* hash);

void blobFile(char* file);

void cp(char* to, char* from);

int file_exists(char* file);


List* initList();

Cell* buildCell(char* ch);

void insertFirst(List* L, Cell* C);

char* ltos(List* L);

List* listdir(char* root_dir);



// Q-4.1: Create and initialize a WorkFile

WorkFile* createWorkFile(char* name) {

	WorkFile* wf = (WorkFile*)malloc(sizeof(WorkFile));

	if (wf == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	wf->name = strdup(name);

	if (wf->name == NULL) {

    	free(wf);

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	wf->hash = NULL;

	wf->mode = 0;

    

	return wf;

}



// Q-4.2: Convert WorkFile to string

char* wfts(WorkFile* wf) {

	if (wf == NULL) return NULL;

    

	// Calculate required size

	size_t size = strlen(wf->name) + 1; // +1 for tab

	if (wf->hash != NULL) {

    	size += strlen(wf->hash);

	}

	size += 12; // Space for the mode (max 4 digits) and extra tabs and null terminator

    

	char* result = (char*)malloc(size);

	if (result == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	if (wf->hash != NULL) {

    	sprintf(result, "%s\t%s\t%d", wf->name, wf->hash, wf->mode);

	} else {

    	sprintf(result, "%s\t\t%d", wf->name, wf->mode);

	}

    

	return result;

}



// Q-4.3: Convert string to WorkFile

WorkFile* stwf(char* ch) {

	if (ch == NULL) return NULL;

    

	char* copy = strdup(ch);

	if (copy == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	WorkFile* wf = (WorkFile*)malloc(sizeof(WorkFile));

	if (wf == NULL) {

    	free(copy);

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	// Extract name

	char* token = strtok(copy, "\t");

	if (token == NULL) {

    	free(copy);

    	free(wf);

    	return NULL;

	}

	wf->name = strdup(token);

    

	// Extract hash

	token = strtok(NULL, "\t");

	if (token == NULL || strlen(token) == 0) {

    	wf->hash = NULL;

	} else {

    	wf->hash = strdup(token);

	}

    

	// Extract mode

	token = strtok(NULL, "\t");

	if (token == NULL) {

    	wf->mode = 0;

	} else {

    	wf->mode = atoi(token);

	}

    

	free(copy);

	return wf;

}



// Q-4.4: Initialize a WorkTree

WorkTree* initWorkTree() {

	WorkTree* wt = (WorkTree*)malloc(sizeof(WorkTree));

	if (wt == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	wt->tab = (WorkFile*)malloc(sizeof(WorkFile) * WORK_TREE_SIZE);

	if (wt->tab == NULL) {

    	free(wt);

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	wt->size = WORK_TREE_SIZE;

	wt->n = 0;

    

	return wt;

}



// Q-4.5: Check if file/directory exists in WorkTree

int inWorkTree(WorkTree* wt, char* name) {

	if (wt == NULL || name == NULL) return -1;

    

	for (int i = 0; i < wt->n; i++) {

    	if (strcmp(wt->tab[i].name, name) == 0) {

        	return i;

    	}

	}

    

	return -1;

}



// Q-4.6: Add file/directory to WorkTree

int appendWorkTree(WorkTree* wt, char* name, char* hash, int mode) {

	if (wt == NULL || name == NULL) return -1;

    

	// Check if file already exists

	if (inWorkTree(wt, name) != -1) {

    	return 0;

	}

    

	// Check if there's enough space

	if (wt->n >= wt->size) {

    	return -1;

	}

    

	// Add the file

	wt->tab[wt->n].name = strdup(name);

	if (hash != NULL) {

    	wt->tab[wt->n].hash = strdup(hash);

	} else {

    	wt->tab[wt->n].hash = NULL;

	}

	wt->tab[wt->n].mode = mode;

	wt->n++;

    

	return 1;

}



// Q-4.7: Convert WorkTree to string

char* wtts(WorkTree* wt) {

	if (wt == NULL || wt->n == 0) {

    	return strdup("");

	}

    

	// Calculate required size

	size_t size = 0;

	for (int i = 0; i < wt->n; i++) {

    	char* temp = wfts(&(wt->tab[i]));

    	size += strlen(temp) + 1; // +1 for newline

    	free(temp);

	}

    

	char* result = (char*)malloc(size + 1); // +1 for null terminator

	if (result == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

	result[0] = '\0';

    

	// Concatenate WorkFile strings

	for (int i = 0; i < wt->n; i++) {

    	char* temp = wfts(&(wt->tab[i]));

    	strcat(result, temp);

    	if (i < wt->n - 1) {

        	strcat(result, "\n");

    	}

    	free(temp);

	}

    

	return result;

}



// Q-4.8: Convert string to WorkTree

WorkTree* stwt(char* s) {

	if (s == NULL || strlen(s) == 0) {

    	return initWorkTree();

	}

    

	WorkTree* wt = initWorkTree();

    

	char* copy = strdup(s);

	if (copy == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	exit(EXIT_FAILURE);

	}

    

	char* token = strtok(copy, "\n");

	while (token != NULL) {

    	WorkFile* wf = stwf(token);

    	if (wf != NULL) {

        	appendWorkTree(wt, wf->name, wf->hash, wf->mode);

        	free(wf->name);

        	if (wf->hash != NULL) free(wf->hash);

        	free(wf);

    	}

    	token = strtok(NULL, "\n");

	}

    

	free(copy);

	return wt;

}



// Q-4.9: Write WorkTree to file

int wttf(WorkTree* wt, char* file) {

	if (wt == NULL || file == NULL) return -1;

    

	FILE* f = fopen(file, "w");

	if (f == NULL) {

    	fprintf(stderr, "Could not open file: %s\n", file);

    	return -1;

	}

    

	char* wtString = wtts(wt);

	fprintf(f, "%s", wtString);

	free(wtString);

    

	fclose(f);

	return 0;

}



// Q-4.10: Read WorkTree from file

WorkTree* ftwt(char* file) {

	if (file == NULL || !file_exists(file)) return NULL;

    

	FILE* f = fopen(file, "r");

	if (f == NULL) {

    	fprintf(stderr, "Could not open file: %s\n", file);

    	return NULL;

	}

    

	// Get file size

	fseek(f, 0, SEEK_END);

	long fileSize = ftell(f);

	rewind(f);

    

	// Read file content

	char* buffer = (char*)malloc(fileSize + 1);

	if (buffer == NULL) {

    	fprintf(stderr, "Memory allocation failed\n");

    	fclose(f);

    	exit(EXIT_FAILURE);

	}

    

	size_t readSize = fread(buffer, 1, fileSize, f);

	buffer[readSize] = '\0';

	fclose(f);

    

	// Convert to WorkTree

	WorkTree* wt = stwt(buffer);

	free(buffer);

    

	return wt;

}



// Q-5.1: Create a snapshot of a WorkTree

char* blobWorkTree(WorkTree* wt) {
	if (wt == NULL) return NULL;

	// 1. Créer un fichier temporaire
	char tempFile[] = "temp_worktree_XXXXXX";
	int fd = mkstemp(tempFile);
	if (fd == -1) {
    	fprintf(stderr, "Could not create temporary file\n");
    	return NULL;
	}
    

	// 2. Écrire le WorkTree dans le fichier temporaire
	wttf(wt, tempFile);

	// 3. Hash du fichier temporaire avec sha256file
	char* hash = sha256file(tempFile);
	if (!hash) {
    	fprintf(stderr, "Erreur de hachage du fichier temporaire\n");
    	remove(tempFile);
    	return NULL;
	}

	// 4. Créer le chemin à partir du hash
	char* hashPath = hashToPath(hash);
	char folder[3] = {hashPath[0], hashPath[1], '\0'};

	// 5. Créer le répertoire si besoin
	char command[256];
	sprintf(command, "mkdir -p %s", folder);
	system(command);

	// 6. Construire le chemin final (ajouter l'extension .t)
	char finalPath[512];
	sprintf(finalPath, "%s.t", hashPath);

	// 7. Copier le fichier temporaire vers le chemin final
	cp(finalPath, tempFile);

	// 8. Supprimer le fichier temporaire
	remove(tempFile);

	free(hashPath);

	// 9. Retourner une copie du hash
	return hash;
}






// Q-5.2: Save a WorkTree and its content recursively

char* saveWorkTree(WorkTree* wt, char* path) {

	if (wt == NULL || path == NULL) return NULL;

    

	char fullPath[1024];

	struct stat st;

    

	// Process all files in WorkTree

	for (int i = 0; i < wt->n; i++) {

    	sprintf(fullPath, "%s/%s", path, wt->tab[i].name);

   	 

    	// Check if it's a file or directory

    	if (stat(fullPath, &st) == 0) {

        	if (S_ISREG(st.st_mode)) {

            	// It's a regular file

            	blobFile(fullPath);

           	 

            	// Get the hash (simple implementation)

           	char* fileHash = sha256file(fullPath);
    	if (fileHash == NULL) continue;

    	// Update WorkFile
    	wt->tab[i].hash = fileHash;

            	wt->tab[i].mode = getChmod(fullPath);

        	} else if (S_ISDIR(st.st_mode)) {

            	// It's a directory

            	// Create a new WorkTree for this directory

            	WorkTree* newWT = initWorkTree();

           	 

            	// Fill the new WorkTree with directory contents

            	List* dirContents = listdir(fullPath);

            	if (dirContents == NULL || *dirContents == NULL) {

                	free(newWT->tab);

                	free(newWT);

                	continue;

            	}

           	 

            	Cell* current = *dirContents;

            	while (current != NULL) {

                	// Skip . and ..

                	if (strcmp(current->data, ".") != 0 && strcmp(current->data, "..") != 0) {

                    	appendWorkTree(newWT, current->data, NULL, 0);

                	}

                	current = current->next;

            	}

           	 

            	// Recursive call on the new WorkTree

            	char* dirHash = saveWorkTree(newWT, fullPath);

           	 

            	// Update WorkFile

            	wt->tab[i].hash = dirHash;

            	wt->tab[i].mode = getChmod(fullPath);

           	 

            	// Clean up

            	free(newWT->tab);

            	free(newWT);

        	}

    	}

	}

    

	// Create snapshot of the WorkTree

	return blobWorkTree(wt);

}



// Q-5.3: Restore a WorkTree and its content recursively

void restoreWorkTree(WorkTree* wt, char* path) {

	if (wt == NULL || path == NULL) return;

    

	// Create the destination directory if it doesn't exist

	char command[1024];

	sprintf(command, "mkdir -p %s", path);

	system(command);

    

	// Process all files in WorkTree

	for (int i = 0; i < wt->n; i++) {

    	if (wt->tab[i].hash == NULL) continue;

   	 

    	// Get the snapshot file path

    	char* hashPath = hashToPath(wt->tab[i].hash);

    	char* destPath = (char*)malloc(strlen(path) + strlen(wt->tab[i].name) + 2);

    	sprintf(destPath, "%s/%s", path, wt->tab[i].name);

   	 

    	// Check if it's a file or directory snapshot

    	char snapshotPath[1024];

    	sprintf(snapshotPath, "%s.t", hashPath);

   	 

    	if (file_exists(snapshotPath)) {

        	// It's a directory snapshot

        	// Create the directory

        	sprintf(command, "mkdir -p %s", destPath);

        	system(command);

       	 

        	// Read the WorkTree from the snapshot

        	WorkTree* subWT = ftwt(snapshotPath);

        	if (subWT != NULL) {

            	// Recursive call to restore the directory content

            	restoreWorkTree(subWT, destPath);

           	 

            	// Clean up

            	free(subWT->tab);

            	free(subWT);

        	}

    	} else {

        	// It's a file snapshot

        	// Copy the snapshot to the destination

        	cp(destPath, hashPath);

       	 

        	// Set the file permissions

        	setMode(wt->tab[i].mode, destPath);

    	}

   	 

    	free(hashPath);

    	free(destPath);

	}

}





int main() {

	printf("Testing WorkTree functions\n");

    

	// Create and populate a WorkTree

	WorkTree* wt = initWorkTree();

	appendWorkTree(wt, "file1.txt", NULL, 0);

	appendWorkTree(wt, "file2.txt", NULL, 0);

	appendWorkTree(wt, "dir1", NULL, 0);

    

	// Save to file

	printf("Saving WorkTree to file 'worktree.txt'\n");

	wttf(wt, "worktree.txt");

    

	// Read from file

	printf("Reading WorkTree from file 'worktree.txt'\n");

	WorkTree* wt2 = ftwt("worktree.txt");

    

	// Print WorkTree

	char* wtString = wtts(wt2);

	printf("WorkTree content:\n%s\n", wtString);

	free(wtString);

    

	// Test saveWorkTree

	printf("\nTesting saveWorkTree with current directory\n");

	char* hash = saveWorkTree(wt, ".");

	if (hash != NULL) {

    	printf("WorkTree saved with hash: %s\n", hash);

    	free(hash);

	}

    

	// Test restoreWorkTree

	printf("\nTesting restoreWorkTree to 'restored' directory\n");

	restoreWorkTree(wt, "restored");

    

	// Clean up

	free(wt->tab);

	free(wt);

	free(wt2->tab);

	free(wt2);

    

	return 0;

}

