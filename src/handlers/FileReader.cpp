#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cctype>
#include <sstream>

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
    FileReader()
    {
        // Constructor - nothing specific needed for STL containers
    }

    /**
     * Destructor - cleanup handled automatically by STL containers
     */
    ~FileReader()
    {
        // Destructor - STL containers handle cleanup automatically
    }

    /**
     * Insert ASCII art for a specific character
     * @param key Character key (A-Z)
     * @param art Vector of strings representing the ASCII art (6 lines)
     */
    void insertArt(char key, const std::vector<std::string> &art)
    {
        key = std::toupper(key); // Ensure uppercase

        // Ensure we have exactly MAX_HEIGHT lines
        std::vector<std::string> normalizedArt;
        for (int i = 0; i < MAX_HEIGHT; i++)
        {
            if (i < art.size())
            {
                normalizedArt.push_back(art[i]);
            }
            else
            {
                normalizedArt.push_back(""); // Pad with empty strings if needed
            }
        }

        artMap[key] = normalizedArt;
    }

    /**
     * Lookup ASCII art by character key
     * @param key Character to lookup
     * @return Vector of strings containing the ASCII art, empty if not found
     */
    std::vector<std::string> lookupArt(char key)
    {
        key = std::toupper(key);

        auto it = artMap.find(key);
        if (it != artMap.end())
        {
            return it->second;
        }

        // Return empty vector if not found
        return std::vector<std::string>();
    }

    /**
     * Read ASCII art from file
     * @param filename Name of file to read
     * @return Vector of strings containing the ASCII art
     */
    std::vector<std::string> readAsciiArt(const std::string &filename)
    {
        std::vector<std::string> art;
        std::ifstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Could not open file: " << filename << std::endl;
            return art;
        }

        std::string line;
        int lineCount = 0;

        while (std::getline(file, line) && lineCount < MAX_HEIGHT)
        {
            art.push_back(line);
            lineCount++;
        }

        // Pad with empty strings if we have fewer than MAX_HEIGHT lines
        while (art.size() < MAX_HEIGHT)
        {
            art.push_back("");
        }

        file.close();
        return art;
    }

    /**
     * Load all character files (A-Z) into the hash map
     * Assumes files are named "A.txt", "B.txt", etc.
     */
    void loadAllFiles()
    {
        loadAllFiles("utils/data/characters/"); // Default directory
    }

    /**
     * Load all character files from a specific directory
     * @param directory Path to directory containing character files
     */
    void loadAllFiles(const std::string &directory)
    {
        std::string basePath = directory;

        // Ensure directory path ends with slash
        if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\')
        {
            basePath += "/";
        }

        // Load files for A-Z and 0-9
        for (char c = 'A'; c <= 'Z'; c++)
        {
            std::string filename = basePath + c + ".txt";
            std::vector<std::string> art = readAsciiArt(filename);

            if (!art.empty() && !art[0].empty()) // Check if we actually read something
            {
                insertArt(c, art);
                std::cout << "Loaded ASCII art for character: " << c << std::endl;
            }
            else
            {
                std::cerr << "Could not load ASCII art for character: " << c
                          << " from file: " << filename << std::endl;
            }
        }

        // Also load numbers 0-9
        for (char c = '0'; c <= '9'; c++)
        {
            std::string filename = basePath + c + ".txt";
            std::vector<std::string> art = readAsciiArt(filename);

            if (!art.empty() && !art[0].empty())
            {
                insertArt(c, art);
                std::cout << "Loaded ASCII art for digit: " << c << std::endl;
            }
            else
            {
                std::cerr << "Could not load ASCII art for digit: " << c
                          << " from file: " << filename << std::endl;
            }
        }

        // load . and !
        std::vector<char> specialChars = {'.', '!'};
        for (char c : specialChars)
        {
            std::string filename = basePath + c + ".txt";
            std::vector<std::string> art = readAsciiArt(filename);

            if (!art.empty() && !art[0].empty())
            {
                insertArt(c, art);
                std::cout << "Loaded ASCII art for character: " << c << std::endl;
            }
            else
            {
                std::cerr << "Could not load ASCII art for character: " << c
                          << " from file: " << filename << std::endl;
            }
        }
    }

    /**
     * Check if a character has ASCII art loaded
     * @param key Character to check
     * @return True if character has ASCII art loaded
     */
    bool hasArt(char key) const
    {
        key = std::toupper(key);
        return artMap.find(key) != artMap.end();
    }

    /**
     * Get all loaded character keys
     * @return Vector of characters that have ASCII art loaded
     */
    std::vector<char> getLoadedKeys() const
    {
        std::vector<char> keys;
        for (const auto &pair : artMap)
        {
            keys.push_back(pair.first);
        }
        return keys;
    }

    /**
     * Clear all loaded ASCII art
     */
    void clear()
    {
        artMap.clear();
    }
};