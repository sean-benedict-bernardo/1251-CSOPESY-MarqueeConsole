#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <iomanip>
#include <conio.h>
#include <fstream>
#include <map>

using namespace std;

#define MAX_ASCII_ART_HEIGHT 6

/**
 * Simple ASCII Art Manager for DisplayHandler
 */
class SimpleASCIIArt
{
private:
    map<char, vector<string>> charMap;
    int artHeight;
    
public:
    SimpleASCIIArt()
    {
        artHeight = MAX_ASCII_ART_HEIGHT;
        loadBasicCharacters();
    }
    
    void loadBasicCharacters()
    {
        // Load some basic characters from files
        string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        for (char c : chars)
        {
            loadCharacterFromFile(c);
        }
        
        // Add space character
        vector<string> space(artHeight, "     ");
        charMap[' '] = space;
    }
    
    void loadCharacterFromFile(char c)
    {
        string filename = "src/utils/data/characters/" + string(1, c) + ".txt";
        ifstream file(filename);
        vector<string> lines;
        
        if (file.is_open())
        {
            string line;
            while (getline(file, line) && lines.size() < artHeight)
            {
                // Ensure consistent width
                if (line.length() < 10) {
                    line += string(10 - line.length(), ' ');
                }
                lines.push_back(line);
            }
            file.close();
            
            // Fill to required height
            while (lines.size() < artHeight)
            {
                lines.push_back(string(10, ' '));
            }
        }
        else
        {
            // Create simple fallback
            vector<string> fallback(artHeight, string(5, ' ') + c + string(4, ' '));
            lines = fallback;
        }
        
        charMap[c] = lines;
    }
    
    vector<string> textToASCII(const string& text, int scrollPos = 0)
    {
        vector<string> result(artHeight);
        
        // Calculate visible portion based on scroll
        string displayText = text + "   " + text; // Add padding for smooth scroll
        int startPos = scrollPos % displayText.length();
        
        // Build ASCII art line by line
        for (int row = 0; row < artHeight; row++)
        {
            string line = "";
            for (int i = 0; i < min(8, (int)text.length()); i++) // Limit to ~8 chars for width
            {
                int charIndex = (startPos + i) % displayText.length();
                if (charIndex < displayText.length())
                {
                    char c = toupper(displayText[charIndex]);
                    if (charMap.find(c) != charMap.end())
                    {
                        line += charMap[c][row];
                    }
                    else
                    {
                        line += string(10, ' ');
                    }
                }
            }
            result[row] = line;
        }
        
        return result;
    }
    
    int getHeight() const { return artHeight; }
};

class DisplayHandler
{
private:
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // Screen dimensions and layout settings
    int consoleWidth;
    int consoleHeight;
    int marqueeHeight;
    int textConsoleWidth;
    int gifSectionWidth;
    int textConsoleHeight;
    
    // Current display state
    vector<string> currentGifFrame;
    vector<string> textConsoleLines;
    string currentMarqueeText;
    int marqueePosition;
    
    // Marquee ASCII art handler
    SimpleASCIIArt* asciiArt;
    bool useASCIIArt;
    
    // Input area management
    int inputAreaY;
    string currentInputLine;
    bool isInInputMode;
    
    // OS emulator state pointers
    bool *isRunning;
    bool *isAnimating;
    int *speed;
    string *marqueeText;
    
public:
    /**
     * Constructor for DisplayHandler
     * @param isRunning Pointer to the running state of the OS emulator
     * @param isAnimating Pointer to the animation state
     * @param speed Pointer to the marquee speed
     * @param marqueeText Pointer to the marquee text
     */
    DisplayHandler(bool *isRunning, bool *isAnimating, int *speed, string *marqueeText)
    {
        this->isRunning = isRunning;
        this->isAnimating = isAnimating;
        this->speed = speed;
        this->marqueeText = marqueeText;
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        // Initialize layout dimensions
        consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        
        marqueeHeight = 8;  // Increased height for ASCII art marquee
        textConsoleWidth = consoleWidth * 0.6;  // Left 60% for text console
        gifSectionWidth = consoleWidth * 0.4;   // Right 40% for gif section
        textConsoleHeight = consoleHeight - marqueeHeight;
        
        marqueePosition = 0;
        currentMarqueeText = *marqueeText;
        isInInputMode = false;
        currentInputLine = "";
        
        // Calculate input area position (last line of text console)
        inputAreaY = marqueeHeight + textConsoleHeight - 1;
        
        // Initialize ASCII art support
        try {
            asciiArt = new SimpleASCIIArt();
            useASCIIArt = true;
        } catch (const exception& e) {
            asciiArt = nullptr;
            useASCIIArt = false;
        }
        
        // Initialize console lines storage
        textConsoleLines.reserve(textConsoleHeight - 2); // Reserve space minus input area
    }
    
    /**
     * Destructor - Clean up resources
     */
    ~DisplayHandler()
    {
        if (asciiArt) {
            delete asciiArt;
        }
    }
    
    /**
     * Main display update function - orchestrates all display sections
     */
    void updateDisplay()
    {
        // Hide cursor during updates to prevent flickering
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        bool wasVisible = cursorInfo.bVisible;
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // Only update specific sections instead of clearing entire screen
        drawMarqueeSection();
        drawTextConsole();
        drawGifSection();
        drawInputArea();
        
        // Restore cursor visibility if it was visible
        if (wasVisible)
        {
            cursorInfo.bVisible = true;
            SetConsoleCursorInfo(hConsole, &cursorInfo);
        }
    }
    
    /**
     * Updates the gif section with new frame data
     * @param frameData Array of strings representing the current gif frame
     */
    void updateGifFrame(const vector<string>& frameData)
    {
        currentGifFrame = frameData;
    }
    
    /**
     * Adds a new line to the text console
     * @param line The line to add to the console
     */
    void addConsoleOutput(const string& line)
    {
        textConsoleLines.push_back(line);
        
        // Keep only the lines that fit in the console height (minus input area)
        int maxLines = textConsoleHeight - 2; // -2 for input area
        if (textConsoleLines.size() > maxLines)
        {
            textConsoleLines.erase(textConsoleLines.begin());
        }
    }
    
    /**
     * Adds multiple lines to the text console
     * @param lines Vector of lines to add to the console
     */
    void addConsoleOutput(const vector<string>& lines)
    {
        for (const string& line : lines)
        {
            addConsoleOutput(line);
        }
    }
    
    /**
     * Updates the marquee position and text for animation
     * @param position New marquee position
     */
    void updateMarqueePosition(int position)
    {
        marqueePosition = position;
        // Update current marquee text cache
        currentMarqueeText = *marqueeText;
    }
    
    /**
     * Displays the current prompt in the text console
     */
    void displayPrompt()
    {
        isInInputMode = true;
        currentInputLine = "";
        
        // Position cursor at the input area
        setCursorPosition(0, inputAreaY);
        cout << "CSOPESY> ";
        
        // Show cursor for input
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
    
    /**
     * Updates the current input line for display
     * @param input Current input string
     */
    void updateInputLine(const string& input)
    {
        currentInputLine = input;
        drawInputArea();
    }
    
    /**
     * Displays command response in the text console
     * @param response Vector of response lines from command handler
     */
    void displayCommandResponse(const vector<string>& response)
    {
        // Check for special commands
        if (!response.empty() && response[0] == "CLEAR_CONSOLE")
        {
            clearConsole();
            return;
        }
        
        addConsoleOutput(response);
        updateDisplay();
    }

private:
    /**
     * Clears the entire screen
     */
    void clearScreen()
    {
        // Use Windows API for faster clearing
        COORD coordScreen = {0, 0};
        DWORD cCharsWritten;
        DWORD dwConSize;
        
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        
        FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
        SetConsoleCursorPosition(hConsole, coordScreen);
    }
    
    /**
     * Sets cursor position on the console
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setCursorPosition(int x, int y)
    {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);
    }
    
    /**
     * Draws the marquee section at the top of the screen using ASCII art
     */
    void drawMarqueeSection()
    {
        // Draw top border
        setCursorPosition(0, 0);
        cout << "+" << string(consoleWidth - 2, '=') << "+";
        
        // Draw marquee content using ASCII art
        if (useASCIIArt && asciiArt) {
            vector<string> marqueeDisplay = asciiArt->textToASCII(*marqueeText, 
                                                                 *isAnimating ? marqueePosition : 0);
            
            for (int i = 0; i < marqueeDisplay.size() && i < marqueeHeight - 2; i++) {
                setCursorPosition(0, i + 1);
                cout << "|";
                
                string line = marqueeDisplay[i];
                if (line.length() > consoleWidth - 2) {
                    line = line.substr(0, consoleWidth - 2);
                } else if (line.length() < consoleWidth - 2) {
                    line += string(consoleWidth - 2 - line.length(), ' ');
                }
                
                cout << line << "|";
            }
        } else {
            // Fallback to simple text if ASCII art failed
            setCursorPosition(0, 1);
            cout << "|";
            drawMarqueeText();
            cout << "|";
            
            // Fill remaining height
            for (int i = 2; i < marqueeHeight - 1; i++) {
                setCursorPosition(0, i);
                cout << "|" << string(consoleWidth - 2, ' ') << "|";
            }
        }
        
        // Draw bottom border
        setCursorPosition(0, marqueeHeight - 1);
        cout << "+" << string(consoleWidth - 2, '=') << "+";
    }
    
    /**
     * Draws the marquee text
     */
    void drawMarqueeText()
    {
        string displayText = *marqueeText;
        int textLength = displayText.length();
        int availableWidth = consoleWidth - 2; // Account for side borders
        string output;
        
        if (*isAnimating && textLength > 0)
        {
            // Create scrolling effect
            string scrollText = displayText + "    " + displayText; // Add padding
            int startPos = marqueePosition % (textLength + 4);
            
            string visibleText = scrollText.substr(startPos, availableWidth);
            if (visibleText.length() < availableWidth)
            {
                visibleText += scrollText.substr(0, availableWidth - visibleText.length());
            }
            
            output = visibleText;
        }
        else
        {
            // Static display - center the text
            int padding = (availableWidth - textLength) / 2;
            if (padding > 0)
            {
                output += string(padding, ' ');
            }
            output += displayText;
            if (padding > 0)
            {
                output += string(availableWidth - textLength - padding, ' ');
            }
        }
        
        // Ensure output is exactly the right width
        if (output.length() > availableWidth)
        {
            output = output.substr(0, availableWidth);
        }
        else if (output.length() < availableWidth)
        {
            output += string(availableWidth - output.length(), ' ');
        }
        
        // Output entire line at once to reduce flickering
        cout << output;
    }
    
    /**
     * Draws the text console section on the left side
     */
    void drawTextConsole()
    {
        // Draw vertical separator
        for (int i = marqueeHeight; i < consoleHeight; i++)
        {
            setCursorPosition(textConsoleWidth, i);
            cout << "|";
        }
        
        // Draw text console content (excluding the input area)
        int startY = marqueeHeight;
        int maxLines = textConsoleHeight - 2; // Leave space for input area
        
        for (int i = 0; i < textConsoleLines.size() && i < maxLines; i++)
        {
            setCursorPosition(0, startY + i);
            string line = textConsoleLines[i];
            
            // Truncate line if it's too long for the text console width
            if (line.length() > textConsoleWidth - 1)
            {
                line = line.substr(0, textConsoleWidth - 1);
            }
            
            cout << line;
            
            // Pad with spaces to clear any remaining characters
            if (line.length() < textConsoleWidth - 1)
            {
                cout << string(textConsoleWidth - 1 - line.length(), ' ');
            }
        }
        
        // Clear any remaining lines before input area
        for (int i = textConsoleLines.size(); i < maxLines; i++)
        {
            setCursorPosition(0, startY + i);
            cout << string(textConsoleWidth - 1, ' ');
        }
    }
    
    /**
     * Draws the input area at the bottom of the text console
     */
    void drawInputArea()
    {
        setCursorPosition(0, inputAreaY);
        
        // Clear the input line
        cout << string(textConsoleWidth - 1, ' ');
        
        // Draw the prompt and current input
        setCursorPosition(0, inputAreaY);
        string promptLine = "CSOPESY> " + currentInputLine;
        
        if (promptLine.length() > textConsoleWidth - 1) {
            promptLine = promptLine.substr(0, textConsoleWidth - 1);
        }
        
        cout << promptLine;
        
        // Position cursor at end of input for typing
        if (isInInputMode) {
            setCursorPosition(promptLine.length(), inputAreaY);
        }
    }
    
    /**
     * Draws the gif section on the right side
     */
    void drawGifSection()
    {
        if (currentGifFrame.empty())
        {
            // Display placeholder when no gif is loaded
            drawGifPlaceholder();
            return;
        }
        
        int startX = textConsoleWidth + 1;
        int startY = marqueeHeight;
        
        // Draw gif frame
        for (int i = 0; i < currentGifFrame.size() && i < textConsoleHeight; i++)
        {
            setCursorPosition(startX, startY + i);
            string frameLine = currentGifFrame[i];
            
            // Adjust frame line to fit in gif section width
            if (frameLine.length() > gifSectionWidth - 1)
            {
                frameLine = frameLine.substr(0, gifSectionWidth - 1);
            }
            else if (frameLine.length() < gifSectionWidth - 1)
            {
                frameLine += string(gifSectionWidth - 1 - frameLine.length(), ' ');
            }
            
            cout << frameLine;
        }
        
        // Fill remaining space if gif frame has fewer lines than available space
        for (int i = currentGifFrame.size(); i < textConsoleHeight; i++)
        {
            setCursorPosition(startX, startY + i);
            cout << string(gifSectionWidth - 1, ' ');
        }
    }
    
    /**
     * Draws a placeholder when no gif is loaded
     */
    void drawGifPlaceholder()
    {
        int startX = textConsoleWidth + 1;
        int startY = marqueeHeight;
        int centerY = startY + (textConsoleHeight / 2);
        
        // Clear the gif section
        for (int i = 0; i < textConsoleHeight; i++)
        {
            setCursorPosition(startX, startY + i);
            cout << string(gifSectionWidth - 1, ' ');
        }
        
        // Display placeholder text
        string placeholderText = "GIF SECTION";
        int centerX = startX + (gifSectionWidth - placeholderText.length()) / 2;
        
        setCursorPosition(centerX, centerY);
        cout << placeholderText;
        
        placeholderText = "No animation loaded";
        centerX = startX + (gifSectionWidth - placeholderText.length()) / 2;
        setCursorPosition(centerX, centerY + 1);
        cout << placeholderText;
    }
    
    /**
     * Gets the current console dimensions and updates layout
     */
    void updateConsoleInfo()
    {
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        
        // Recalculate layout dimensions
        textConsoleWidth = consoleWidth * 0.6;
        gifSectionWidth = consoleWidth * 0.4;
        textConsoleHeight = consoleHeight - marqueeHeight;
    }
    
public:
    /**
     * Displays welcome screen with layout information
     */
    void displayWelcome()
    {
        clearScreen();
        
        vector<string> welcomeLines = {
            "+==============================================================================+",
            "|                        CSOPESY Marquee Console v1.0                         |",
            "|                                                                              |",
            "|  Layout:                                                                     |",
            "|  * Marquee Display (Top)                                                     |",
            "|  * Text Console (Left) | GIF Animation (Right)                              |",
            "|                                                                              |",
            "|  Commands:                                                                   |",
            "|  * set_text <text>    - Set marquee text                                    |",
            "|  * start_marquee      - Start marquee animation                             |",
            "|  * stop_marquee       - Stop marquee animation                              |",
            "|  * set_speed <ms>     - Set animation speed                                 |",
            "|  * clear              - Clear console                                       |",
            "|  * exit               - Exit application                                    |",
            "|                                                                              |",
            "|  Press any key to continue...                                               |",
            "+==============================================================================+"
        };
        
        addConsoleOutput(welcomeLines);
        updateDisplay();
        
        // Wait for key press
        _getch();
        
        // Clear and show initial state
        textConsoleLines.clear();
        addConsoleOutput("Welcome to CSOPESY Marquee Console");
        addConsoleOutput("Type 'help' for available commands");
        updateDisplay();
        displayPrompt();
    }
    
    /**
     * Displays exit message
     */
    void displayExit()
    {
        clearScreen();
        setCursorPosition(0, consoleHeight / 2);
        
        string exitMsg = "Thank you for using CSOPESY Marquee Console!";
        int centerX = (consoleWidth - exitMsg.length()) / 2;
        setCursorPosition(centerX, consoleHeight / 2);
        cout << exitMsg;
        
        setCursorPosition(0, consoleHeight / 2 + 2);
        exitMsg = "Press any key to exit...";
        centerX = (consoleWidth - exitMsg.length()) / 2;
        setCursorPosition(centerX, consoleHeight / 2 + 2);
        cout << exitMsg;
        
        _getch();
    }
    
    /**
     * Clears the text console section
     */
    void clearConsole()
    {
        textConsoleLines.clear();
        updateDisplay();
    }
    
    /**
     * Forces a complete redraw of the display
     */
    void forceRedraw()
    {
        updateConsoleInfo();
        updateDisplay();
    }
};