#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <map>

// Include all necessary handler files
#include "src/handlers/DisplayHandler.cpp"
#include "src/handlers/CommandHandler.cpp"
#include "src/handlers/KeyboardHandler.cpp"
#include "src/utils/Frames.cpp"

using namespace std;

/**
 * ASCII Art Manager - Loads and manages ASCII character art
 */
class ASCIIArtManager
{
private:
    map<char, vector<string>> characterMap;
    int artHeight;
    
public:
    ASCIIArtManager()
    {
        artHeight = 6; // Standard height for ASCII art characters
        loadCharacterArt();
    }
    
    /**
     * Loads ASCII art for all characters from files
     */
    void loadCharacterArt()
    {
        // Load alphabetic characters
        for (char c = 'A'; c <= 'Z'; c++)
        {
            loadCharacter(c, "src/utils/data/characters/" + string(1, c) + ".txt");
        }
        
        // Load numeric characters
        for (char c = '0'; c <= '9'; c++)
        {
            loadCharacter(c, "src/utils/data/characters/" + string(1, c) + ".txt");
        }
        
        // Add space character
        vector<string> spaceArt(artHeight, "   ");
        characterMap[' '] = spaceArt;
    }
    
    /**
     * Loads a single character's ASCII art from file
     */
    void loadCharacter(char c, const string& filename)
    {
        ifstream file(filename);
        vector<string> lines;
        string line;
        
        if (file.is_open())
        {
            while (getline(file, line) && lines.size() < artHeight)
            {
                // Ensure each line is at least 10 characters wide for proper spacing
                if (line.length() < 10)
                {
                    line += string(10 - line.length(), ' ');
                }
                lines.push_back(line);
            }
            file.close();
            
            // Fill remaining lines if needed
            while (lines.size() < artHeight)
            {
                lines.push_back(string(10, ' '));
            }
            
            characterMap[c] = lines;
        }
        else
        {
            // Fallback for missing files
            vector<string> fallback(artHeight, "   " + string(1, c) + "   ");
            characterMap[c] = fallback;
        }
    }
    
    /**
     * Converts text to ASCII art lines
     */
    vector<string> textToASCIIArt(const string& text)
    {
        vector<string> result(artHeight);
        
        for (char c : text)
        {
            char upperC = toupper(c);
            if (characterMap.find(upperC) != characterMap.end())
            {
                vector<string> charArt = characterMap[upperC];
                for (int i = 0; i < artHeight; i++)
                {
                    result[i] += charArt[i];
                }
            }
            else
            {
                // Fallback for unsupported characters
                for (int i = 0; i < artHeight; i++)
                {
                    result[i] += "   ?   ";
                }
            }
        }
        
        return result;
    }
    
    int getArtHeight() const { return artHeight; }
};

/**
 * Main Marquee Console Application
 * Integrates DisplayHandler, CommandHandler, KeyboardHandler, Frames, and ASCII Art
 * to create a complete console application with marquee text, command interface, and GIF animation
 */
class MarqueeConsole
{
private:
    // Core application state
    bool isRunning;
    bool isAnimating;
    int speed;  // milliseconds between marquee updates
    string marqueeText;
    
    // Animation state
    int marqueePosition;
    int gifFrameIndex;
    chrono::steady_clock::time_point lastMarqueeUpdate;
    chrono::steady_clock::time_point lastGifUpdate;
    
    // Component handlers
    DisplayHandler* displayHandler;
    CommandHandler* commandHandler;
    KeyboardHandler* keyboardHandler;
    Frames* frames;
    ASCIIArtManager* asciiArt;
    
    // Input management
    string currentInput;
    bool waitingForInput;
    HANDLE hConsole;
    
public:
    /**
     * Constructor - Initializes all components and sets default values
     */
    MarqueeConsole()
    {
        // Initialize application state
        isRunning = true;
        isAnimating = false;
        speed = 100;  // Default speed: 100ms between updates
        marqueeText = "CSOPESY MARQUEE CONSOLE";
        marqueePosition = 0;
        gifFrameIndex = 0;
        currentInput = "";
        waitingForInput = false;
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Initialize timing
        lastMarqueeUpdate = chrono::steady_clock::now();
        lastGifUpdate = chrono::steady_clock::now();
        
        // Initialize component handlers
        displayHandler = new DisplayHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        commandHandler = new CommandHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        keyboardHandler = new KeyboardHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        asciiArt = new ASCIIArtManager();
        
        // Initialize frames for GIF animation with correct path
        try {
            frames = new Frames(NUM_FRAMES);
        } catch (const exception& e) {
            frames = nullptr;
            displayHandler->addConsoleOutput("Warning: Could not load GIF frames - " + string(e.what()));
        }
    }
    
    /**
     * Destructor - Cleanup allocated resources
     */
    ~MarqueeConsole()
    {
        delete displayHandler;
        delete commandHandler;
        delete keyboardHandler;
        delete asciiArt;
        if (frames) delete frames;
    }
    
    /**
     * Main application entry point
     */
    void run()
    {
        // Display welcome screen
        displayHandler->displayWelcome();
        
        // Main application loop
        while (isRunning)
        {
            // Handle keyboard input
            handleInput();
            
            // Update animations if enabled
            if (isAnimating)
            {
                updateAnimations();
            }
            
            // Update display
            displayHandler->updateDisplay();
            
            // Small delay to prevent excessive CPU usage
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        
        // Display exit message
        displayHandler->displayExit();
    }

private:
    /**
     * Handles keyboard input and command processing with proper display management
     */
    void handleInput()
    {
        if (_kbhit())
        {
            char key = _getch();
            
            if (!waitingForInput)
            {
                // Start input mode
                waitingForInput = true;
                currentInput = "";
                displayHandler->displayPrompt();
            }
            
            // Handle special keys
            if (key == '\r' || key == '\n')  // Enter key
            {
                if (!currentInput.empty())
                {
                    // Process the command
                    processCommand(currentInput);
                    currentInput = "";
                }
                waitingForInput = false;
                displayHandler->displayPrompt();
            }
            else if (key == '\b')  // Backspace
            {
                if (!currentInput.empty())
                {
                    currentInput.pop_back();
                    displayHandler->updateInputLine(currentInput);
                }
            }
            else if (key == 27)  // Escape key
            {
                currentInput = "";
                waitingForInput = false;
                displayHandler->addConsoleOutput("Input cancelled");
                displayHandler->displayPrompt();
            }
            else if (key >= 32 && key <= 126)  // Printable characters
            {
                currentInput += key;
                displayHandler->updateInputLine(currentInput);
            }
        }
    }
    
    /**
     * Processes user commands through the command handler
     * @param input The command input string to process
     */
    void processCommand(const string& input)
    {
        vector<string> response = commandHandler->parseInput(input);
        
        // Handle special responses
        bool shouldClear = false;
        for (const string& line : response)
        {
            if (line == "CLEAR_CONSOLE")
            {
                shouldClear = true;
                break;
            }
        }
        
        if (shouldClear)
        {
            displayHandler->clearConsole();
        }
        else if (!response.empty())
        {
            displayHandler->displayCommandResponse(response);
        }
        
        // Add command to console output
        displayHandler->addConsoleOutput("CSOPESY> " + input);
        if (!shouldClear && !response.empty())
        {
            displayHandler->addConsoleOutput(response);
        }
    }
    
    /**
     * Updates marquee and GIF animations with ASCII art support
     */
    void updateAnimations()
    {
        auto currentTime = chrono::steady_clock::now();
        
        // Update marquee animation with ASCII art
        auto marqueeElapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - lastMarqueeUpdate);
        if (marqueeElapsed.count() >= speed)
        {
            // Convert marquee text to ASCII art
            vector<string> asciiLines = asciiArt->textToASCIIArt(marqueeText);
            
            // For now, we'll use the regular marquee functionality
            // but we could enhance DisplayHandler to show ASCII art in the marquee section
            marqueePosition++;
            if (marqueePosition >= marqueeText.length() + 100)
            {
                marqueePosition = 0;
            }
            displayHandler->updateMarqueePosition(marqueePosition);
            lastMarqueeUpdate = currentTime;
        }
        
        // Update GIF animation (frames from utils/data/ascii_frames)
        if (frames)
        {
            auto gifElapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - lastGifUpdate);
            if (gifElapsed.count() >= 100)  // Update GIF every 100ms
            {
                try
                {
                    vector<string> currentFrame = frames->getFrame(gifFrameIndex);
                    displayHandler->updateGifFrame(currentFrame);
                    
                    gifFrameIndex++;
                    if (gifFrameIndex >= NUM_FRAMES)
                    {
                        gifFrameIndex = 0;  // Loop back to first frame
                    }
                    lastGifUpdate = currentTime;
                }
                catch (const exception& e)
                {
                    // If frame loading fails, just continue with next frame
                    gifFrameIndex++;
                    if (gifFrameIndex >= NUM_FRAMES)
                    {
                        gifFrameIndex = 0;
                    }
                }
            }
        }
    }
};

/**
 * Main function - Entry point of the application
 */
int main()
{
    try
    {
        // Set console window properties for better display
        SetConsoleTitleA("CSOPESY Marquee Console");
        
        // Create and run the marquee console application
        MarqueeConsole app;
        app.run();
    }
    catch (const exception& e)
    {
        cout << "Error: " << e.what() << endl;
        cout << "Press any key to exit..." << endl;
        _getch();
        return 1;
    }
    
    return 0;
}