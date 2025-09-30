#include "FileReader.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <sstream>

FileReader::FileReader()
{
    // Constructor - nothing specific needed for STL containers
}

FileReader::~FileReader()
{
    // Destructor - STL containers handle cleanup automatically
}

void FileReader::insertArt(char key, const std::vector<std::string>& art)
{
    key = std::toupper(key);  // Ensure uppercase
    
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
            normalizedArt.push_back("");  // Pad with empty strings if needed
        }
    }
    
    artMap[key] = normalizedArt;
}

std::vector<std::string> FileReader::lookupArt(char key)
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

std::vector<std::string> FileReader::readAsciiArt(const std::string& filename)
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

void FileReader::loadAllFiles()
{
    loadAllFiles("utils/data/characters/");  // Default directory
}

void FileReader::loadAllFiles(const std::string& directory)
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
        
        if (!art.empty() && !art[0].empty())  // Check if we actually read something
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
}

bool FileReader::hasArt(char key) const
{
    key = std::toupper(key);
    return artMap.find(key) != artMap.end();
}

std::vector<char> FileReader::getLoadedKeys() const
{
    std::vector<char> keys;
    for (const auto& pair : artMap)
    {
        keys.push_back(pair.first);
    }
    return keys;
}

void FileReader::clear()
{
    artMap.clear();
}