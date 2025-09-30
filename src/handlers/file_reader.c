#include "file_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static Entry *hashTable[TABLE_SIZE];

// Hash function (A=0, ..., Z=25)
static unsigned int hash(char key) {
    key = toupper((unsigned char)key);
    return (key - 'A') % TABLE_SIZE;
}

// Initialize hash table
void initFileReader(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
}

// Free all memory in hash map
void freeFileReader(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = hashTable[i];
        while (entry) {
            Entry *next = entry->next;
            for (int j = 0; j < MAX_HEIGHT; j++) {
                free(entry->art[j]);
            }
            free(entry->art);
            free(entry);
            entry = next;
        }
        hashTable[i] = NULL;
    }
}

// Insert ASCII art into hash map
void insertArt(char key, char **art) {
    unsigned int index = hash(key);

    Entry *newEntry = malloc(sizeof(Entry));
    newEntry->key = key;

    // Deep copy of art
    newEntry->art = malloc(MAX_HEIGHT * sizeof(char *));
    for (int i = 0; i < MAX_HEIGHT; i++) {
        newEntry->art[i] = strdup(art[i]);
    }

    newEntry->next = hashTable[index];
    hashTable[index] = newEntry;
}

// Lookup by key
char **lookupArt(char key) {
    unsigned int index = hash(key);
    Entry *entry = hashTable[index];
    while (entry) {
        if (entry->key == key) {
            return entry->art;
        }
        entry = entry->next;
    }
    return NULL;
}

// Read ASCII art (6 lines) from file into char**
char **readAsciiArt(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    char **art = malloc(MAX_HEIGHT * sizeof(char *));
    char buffer[512];  // assume max width < 512

    for (int i = 0; i < MAX_HEIGHT; i++) {
        if (fgets(buffer, sizeof(buffer), file)) {
            buffer[strcspn(buffer, "\n")] = 0; // remove newline
            art[i] = strdup(buffer);
        } else {
            art[i] = strdup(""); // pad with empty if fewer lines
        }
    }

    fclose(file);
    return art;
}

// Load all files A-Z into hash map automatically
void loadAllFiles(void) {
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < 26; i++) {
        char filename[2] = { letters[i], '\0' };
        char **art = readAsciiArt(filename);
        if (art) {
            insertArt(letters[i], art);
            // free temp copy (deep-copied inside insertArt)
            for (int j = 0; j < MAX_HEIGHT; j++) free(art[j]);
            free(art);
        } else {
            printf("Could not read file %s\n", filename);
        }
    }
}
