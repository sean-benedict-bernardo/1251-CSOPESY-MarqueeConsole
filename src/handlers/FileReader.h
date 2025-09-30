#ifndef FILEREADER_H
#define FILEREADER_H

#include <string>
#include <vector>
#include <unordered_map>

#define MAX_HEIGHT 6

/**
 * FileReader class for loading and managing ASCII art characters
 * Converted from C to C++ with STL containers
 */
class FileReader
{
private:
    // Hash map to store ASCII art by character key
    std::unordered_map<char, std::vector<std::string>> artMap;
    
public:
    /**
     * Constructor - initializes the file reader
     */
    FileReader();
    
    /**
     * Destructor - cleanup handled automatically by STL containers
     */
    ~FileReader();
    
    /**
     * Insert ASCII art for a specific character
     * @param key Character key (A-Z)
     * @param art Vector of strings representing the ASCII art (6 lines)
     */
    void insertArt(char key, const std::vector<std::string>& art);
    
    /**
     * Lookup ASCII art by character key
     * @param key Character to lookup
     * @return Vector of strings containing the ASCII art, empty if not found
     */
    std::vector<std::string> lookupArt(char key);
    
    /**
     * Read ASCII art from file
     * @param filename Name of file to read
     * @return Vector of strings containing the ASCII art
     */
    std::vector<std::string> readAsciiArt(const std::string& filename);
    
    /**
     * Load all character files (A-Z) into the hash map
     * Assumes files are named "A.txt", "B.txt", etc.
     */
    void loadAllFiles();
    
    /**
     * Load all character files from a specific directory
     * @param directory Path to directory containing character files
     */
    void loadAllFiles(const std::string& directory);
    
    /**
     * Check if a character has ASCII art loaded
     * @param key Character to check
     * @return True if character has ASCII art loaded
     */
    bool hasArt(char key) const;
    
    /**
     * Get all loaded character keys
     * @return Vector of characters that have ASCII art loaded
     */
    std::vector<char> getLoadedKeys() const;
    
    /**
     * Clear all loaded ASCII art
     */
    void clear();
};

#endif // FILEREADER_H