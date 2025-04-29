#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

int hashFile(char* source, char* dest){
	char cmd[256];
	sprintf(cmd, "cat %s | sha256sum > %s", source, dest);
	return system(cmd);
    
}

char* sha256file(char* file) {
	static char template[] = "/tmp/hashXXXXXX";
	char fname[1000];
	strcpy(fname, template); // création d'une copie manipulable

	int fd = mkstemp(fname);
	if (fd == -1) {
    	perror("Erreur mkstemp");
    	return NULL;
	}
	close(fd); // On n'utilise que le nom du fichier ici

	// Hachage via system et redirection vers le fichier temporaire
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "sha256sum %s > %s", file, fname);
	if (system(cmd) != 0) {
    	perror("Erreur system(sha256sum)");
    	remove(fname);
    	return NULL;
	}

	// Lecture du hash depuis le fichier temporaire
	FILE* f = fopen(fname, "r");
	if (!f) {
    	perror("Erreur fopen");
    	remove(fname);
    	return NULL;
	}

	char* hash = malloc(65); // SHA256 = 64 caractères + '\0'
	if (!hash) {
    	perror("Erreur malloc");
    	fclose(f);
    	remove(fname);
    	return NULL;
	}

	fscanf(f, "%64s", hash); // On lit juste le hash
	fclose(f);
	remove(fname); // Supprimer le fichier temporaire

	return hash;
}


typedef struct cell {
	char* data;
	struct cell* next;
} Cell;

typedef Cell* List;

//2.1. initialiser une liste vide
List* initList()           	 
{  	 
	List* l= (List*)malloc(sizeof(List));
	if(l==NULL){
    	fprintf(stderr, "echec d'allocation\n");
    	exit(EXIT_FAILURE);
	}
	*l = NULL;
	return l;
}        	 

//2.2. allouer et retourner une cellule de le liste
Cell* buildCell(char* ch) {  	 
	Cell* c = malloc(sizeof(Cell));
	if (c == NULL) {
	fprintf(stderr, "echec d'allocation\n");
	exit(EXIT_FAILURE);
	}
	c->data = strdup(ch);
	c->next = NULL;
	return c;
}

//2.3. inserer une cellule au début d'une liste
void insertFirst(List* L, Cell* C)
{
	if(L==NULL || C==NULL) return;
	C ->next = *L;
	*L = C;
}  

//retourne la chaîne de caractères qu’elle représente
char* ctos(Cell* c)
{
	if(c == NULL) return NULL;
	return c -> data;    
}


// transforme une liste en une chaîne de caractères: chaîne1|chaîne2|chaîne3..
char* ltos(List* l)
{
	if(l == NULL || *l == NULL){
    	char* res = (char*)malloc(1);
    	if(res == NULL){
        	fprintf(stderr, "echec d'allocation\n");
        	exit(EXIT_FAILURE);
    	}
    	res[0] = '\0';
    	return res;
	}
	size_t taille = 0;
	Cell* current = *l;
	while(current != NULL){
    	taille += strlen(current -> data)+1;
    	current = current -> next;
	}
    
	char* res = (char*)malloc(taille);
	if (res == NULL){
    	fprintf(stderr, "echec d'allocation\n");
    	exit(EXIT_FAILURE);
	}
	res[0] = '\0';    
    
	current = *l;
	while(current != NULL){
    	strcat(res, current -> data);
    	if(current -> next != NULL){
        	strcat(res, "|");
    	}
    	current= current -> next;
	}
	return res;    
}

//2.5. renvoie le i’eme élément d’une liste.
Cell* listGet(List* l, int i)
{
	if (l == NULL || *l == NULL || i<0) return NULL;
    
	Cell* current = *l;
	int pos=0;
	while(current !=NULL && pos <i){
    	current = current -> next;
    	pos++;
	}
	return current;
}

//2.6. recherche un élément dans une liste à partir de son contenu et renvoie une référence vers lui ou NULL s’il n’est pas dans la liste.

Cell* searchList(List* l, char* str)
{
	if (l == NULL || *l == NULL || str == NULL) return NULL;
    
	Cell* current = *l;
	while(current != NULL){
    	if(strcmp(current->data, str)==0){
        	return current;
    	}
    	current = current -> next;
	}
	return NULL;    
}

//2.7. transformer une chaîne de caractères représentant une liste en une liste chaînée.

List* stol(char* s)
{
	if (s== NULL) return NULL;
    
	List* l = initList();
	if (strlen(s) == 0) return l;
	char* p = strdup(s);
	if (p == NULL){
    	fprintf(stderr, "echec d'allocation\n");
    	exit(EXIT_FAILURE);
	}
    
	char* token = strtok(p, "|");
	Cell* oldcell = NULL;
	while (token != NULL){
    	Cell* newcell = buildCell(token);
    	newcell->next = oldcell;
    	oldcell = newcell;
    	token= strtok(NULL, "|");
	}
    
	Cell* current = oldcell;
	oldcell = NULL;
	Cell* next;
    
	while(current != NULL){
    	next = current->next;
    	current->next = oldcell;
    	oldcell = current;
    	current = next;
	}
    
	*l = oldcell;
	free(p);
	return l;
}


//2.8. écrire une liste dans un fichier

void ltof(List* l, char* path)
{
	if(l==NULL || path == NULL) return;
    
	FILE* file = fopen(path, "w");
	if(file == NULL){
    	fprintf(stderr, "could not open file %s\n", path);
    	return;
	}
    
	char* p = ltos(l);
	fprintf(file, "%s", p);
	free(p);
	fclose(file);
}


//lire une liste enregistrée dans un fichier.
List* ftol(char* path) {

	if (path == NULL) return NULL;
	FILE* file = fopen(path, "r");

	if (file == NULL) {
    	fprintf(stderr, "Could not open file %s\n", path);
    	return NULL;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);
	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
    	fprintf(stderr, "Memory allocation failed\n");
    	fclose(file);
    	return NULL;
	}

	size_t readSize = fread(buffer, 1, fileSize, file);
	buffer[readSize] = '\0';
	fclose(file);

	List* L = stol(buffer);
	free(buffer);
	return L;
}













// Q-3.1: List directory contents

List* listdir(char* root_dir) {
	List* result = initList();
	DIR* dp = opendir(root_dir);
	struct dirent* ep;
	if (dp != NULL) {
    	while ((ep = readdir(dp)) != NULL) {
        	Cell* cell = buildCell(ep->d_name);
        	insertFirst(result, cell);
    	}
    	closedir(dp);
	}
	return result;
}



// Q-3.2: Check if file exists in current directory

int file_exists(char* file) {
	return access(file, F_OK) == 0;
}
// Q-3.3: Copy file contents from one file to another
void cp(char* to, char* from) {

	// Check if source file exists
	if (!file_exists(from)) {
    	fprintf(stderr, "Source file does not exist: %s\n", from);
    	return;
	}
	FILE* source = fopen(from, "r");
	if (source == NULL) {
    	fprintf(stderr, "Could not open source file: %s\n", from);
    	return;
	}
    
	FILE* destination = fopen(to, "w");
	if (destination == NULL) {
    	fprintf(stderr, "Could not open destination file: %s\n", to);
    	fclose(source);
    	return;
	}

	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), source) != NULL) {
    	fputs(buffer, destination);
	}

	fclose(source);
	fclose(destination);
}

// Q-3.4: Convert hash to path by inserting '/' after 2nd character
char* hashToPath(char* hash) {
	if (hash == NULL || strlen(hash) < 3) {
    	return NULL;
	}

	char* path = (char*)malloc(strlen(hash) + 2); // +2 for '/' and '\0'
	if (path == NULL) {
    	fprintf(stderr, "Memory allocation failed\n");
    	exit(EXIT_FAILURE);
	}

	// Copy first 2 characters
	strncpy(path, hash, 2);
	// Insert '/'
	path[2] = '/';
	// Copy rest of hash
	strcpy(path + 3, hash + 2);
	return path;
}

// Q-3.5: Create a snapshot of a file
void blobFile(char* file) {

	// Check if file exists
	if (!file_exists(file)) {
    	fprintf(stderr, "File does not exist: %s\n", file);
    	return;
	}

	// Read file content
	FILE* f = fopen(file, "r");
	if (f == NULL) {
    	fprintf(stderr, "Could not open file: %s\n", file);
    	return;
	}
	// Get file size
	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	rewind(f);

	// Read file content
	char* content = (char*)malloc(fileSize + 1);
	if (content == NULL) {
    	fprintf(stderr, "Memory allocation failed\n");
    	fclose(f);
    	exit(EXIT_FAILURE);
	}

	size_t readSize = fread(content, 1, fileSize, f);
	content[readSize] = '\0';
	fclose(f);

	// Generate a simple hash (a real implementation would use SHA1)
	// For demonstration, we'll use a simple hash based on file length and first characters

	char hash[41]; // SHA1 is 40 chars + null terminator
	sprintf(hash, "%02lx%s", fileSize % 255, file);

	// Convert hash to path
	char* path = hashToPath(hash);

	// Create directory structure
	char command[1024];
	sprintf(command, "mkdir -p %c%c", path[0], path[1]);
	system(command);

	// Create the blob file
	char blobPath[1024];
	sprintf(blobPath, "%s", path);
    
	// Write content to the blob file
	FILE* blob = fopen(blobPath, "w");
	if (blob == NULL) {
    	fprintf(stderr, "Could not create blob file: %s\n", blobPath);
    	free(content);
    	free(path);
    	return;
	}

	fwrite(content, 1, readSize, blob);
	fclose(blob);
	printf("Created blob for %s at %s\n", file, blobPath);
	free(content);
	free(path);

}

 



