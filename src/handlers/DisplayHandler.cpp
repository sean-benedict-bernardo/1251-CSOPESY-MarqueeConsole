#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <iomanip>
#include <conio.h>

using namespace std;

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
        
        marqueeHeight = 3;  // Top section for marquee
        textConsoleWidth = consoleWidth * 0.6;  // Left 60% for text console
        gifSectionWidth = consoleWidth * 0.4;   // Right 40% for gif section
        textConsoleHeight = consoleHeight - marqueeHeight;
        
        marqueePosition = 0;
        currentMarqueeText = *marqueeText;
        
        // Initialize console lines storage
        textConsoleLines.reserve(textConsoleHeight);
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
        
        // Keep only the lines that fit in the console height
        if (textConsoleLines.size() > textConsoleHeight - 2) // -2 for borders/prompt
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
     * Updates the marquee position for animation
     * @param position New marquee position
     */
    void updateMarqueePosition(int position)
    {
        marqueePosition = position;
    }
    
    /**
     * Displays the current prompt in the text console
     */
    void displayPrompt()
    {
        setCursorPosition(0, marqueeHeight + textConsoleHeight - 1);
        cout << "CSOPESY> ";
        
        // Show cursor for input
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
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
     * Draws the marquee section at the top of the screen
     */
    void drawMarqueeSection()
    {
        // Draw top border
        setCursorPosition(0, 0);
        cout << "+" << string(consoleWidth - 2, '=') << "+";
        
        // Draw marquee text
        setCursorPosition(0, 1);
        cout << "|";
        drawMarqueeText();
        cout << "|";
        
        // Draw bottom border
        setCursorPosition(0, 2);
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
        
        // Draw text console content
        int startY = marqueeHeight;
        for (int i = 0; i < textConsoleLines.size() && i < textConsoleHeight - 2; i++)
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
            "|  * marquee <text>     - Set marquee text                                    |",
            "|  * marquee-start      - Start marquee animation                             |",
            "|  * marquee-stop       - Stop marquee animation                              |",
            "|  * marquee-speed <ms> - Set animation speed                                 |",
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