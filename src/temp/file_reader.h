#ifndef FILE_READER_H
#define FILE_READER_H

#define TABLE_SIZE 26
#define MAX_HEIGHT 6

// Entry structure
typedef struct Entry {
    char key;             // single letter (A-Z)
    char **art;           // 2D array (6 lines of ASCII art)
    struct Entry *next;
} Entry;

// API functions
void initFileReader(void);
void freeFileReader(void);

void insertArt(char key, char **art);
char **lookupArt(char key);
char **readAsciiArt(const char *filename);

// Optional: load all files A-Z into hashmap
void loadAllFiles(void);

#endif