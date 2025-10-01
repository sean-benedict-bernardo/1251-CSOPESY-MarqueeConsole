#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <conio.h>
#include <windows.h>
#include <functional>

using namespace std;

class KeyboardHandler
{
private:
    // Keyboard buffer for input management
    queue<char> keyBuffer;
    string currentInput;
    vector<string> commandHistory;
    int historyIndex;
    int cursorPos;
    bool capsLock;
    
    // Console handle for cursor manipulation
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // OS emulator state pointers
    bool *isRunning;
    bool *isAnimating;
    int *speed;
    string *marqueeText;
    
    // Producer-Consumer callback function
    function<void(const string&)> commandCallback;
    
public:
    /**
     * Constructor for KeyboardHandler
     * @param isRunning Pointer to the running state of the OS emulator
     * @param isAnimating Pointer to the animation state
     * @param speed Pointer to the marquee speed
     * @param marqueeText Pointer to the marquee text
     * @param callback Callback function to enqueue commands (from CommandHandler)
     */
    KeyboardHandler(bool *isRunning, bool *isAnimating, int *speed, string *marqueeText, function<void(const string&)> callback)
    {
        this->isRunning = isRunning;
        this->isAnimating = isAnimating;
        this->speed = speed;
        this->marqueeText = marqueeText;
        this->commandCallback = callback;
        this->currentInput = "";
        this->historyIndex = -1;
        this->cursorPos = 0;
        this->capsLock = false;
        this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    
    /**
     * Polls keyboard for input and buffers keystrokes
     * This is the main polling function called in the OS emulator loop
     */
    void pollKeyboard()
    {
        while (_kbhit())
        {
            int key = _getch();
            
            // Handle special keys that return two values
            if (key == 0 || key == 224)
            {
                int specialKey = _getch();
                handleSpecialKey(specialKey);
            }
            else
            {
                // Buffer regular keys
                keyBuffer.push(static_cast<char>(key));
            }
        }
    }
    
    /**
     * Processes buffered keystrokes and sends commands via callback
     * Commands are now enqueued in CommandHandler instead of returned
     */
    void processBuffer()
    {
        while (!keyBuffer.empty())
        {
            char key = keyBuffer.front();
            keyBuffer.pop();
            
            handleKey(key); // No longer returns commands
        }
    }
    
    /**
     * Gets the current input line for display purposes
     * @return Current input string with cursor indicator
     */
    string getCurrentInputLine()
    {
        string line = "> " + currentInput;
        
        // Add cursor indicator for display
        if (cursorPos <= currentInput.length())
        {
            string displayLine = "> ";
            displayLine += currentInput.substr(0, cursorPos);
            displayLine += "_";  // Cursor indicator
            displayLine += currentInput.substr(cursorPos);
            return displayLine;
        }
        
        return line + "_";
    }
    
    /**
     * Clears the current input buffer
     */
    void clearInput()
    {
        currentInput = "";
        cursorPos = 0;
        historyIndex = -1;
    }
    
    /**
     * Gets command history for debugging/display purposes
     * @return Vector of previous commands
     */
    vector<string> getCommandHistory() const
    {
        return commandHistory;
    }
    
    /**
     * Updates console cursor position for proper display
     */
    void updateCursorPosition()
    {
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        COORD pos = {static_cast<SHORT>(2 + cursorPos), csbi.dwCursorPosition.Y};
        SetConsoleCursorPosition(hConsole, pos);
    }
    
    /**
     * Refreshes the input line display
     */
    void refreshInputDisplay()
    {
        // Get current cursor position
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        // Move to beginning of line
        COORD lineStart = {0, csbi.dwCursorPosition.Y};
        SetConsoleCursorPosition(hConsole, lineStart);
        
        // Clear the line
        string clearLine(csbi.dwSize.X, ' ');
        cout << clearLine;
        
        // Reset to line start and print current input
        SetConsoleCursorPosition(hConsole, lineStart);
        cout << "> " << currentInput;
        
        // Position cursor correctly
        COORD cursorPosition = {static_cast<SHORT>(2 + cursorPos), csbi.dwCursorPosition.Y};
        SetConsoleCursorPosition(hConsole, cursorPosition);
    }

private:
    /**
     * Handles regular key input
     * @param key The character key pressed
     * Commands are now sent via callback instead of returned
     */
    void handleKey(char key)
    {
        switch (key)
        {
        case 13: // Enter (Carriage Return)
            handleEnter();
            break;
            
        case 8: // Backspace
            handleBackspace();
            break;
            
        case 27: // Escape
            handleEscape();
            break;
            
        case 9: // Tab
            handleTab();
            break;
            
        case 3: // Ctrl+C
            handleCtrlC();
            break;
            
        case 26: // Ctrl+Z
            handleCtrlZ();
            break;
            
        default:
            // Handle printable characters
            if (key >= 32 && key <= 126)
            {
                insertCharacter(key);
            }
            break;
        }
    }
    
    /**
     * Handles special keys (arrow keys, function keys, etc.)
     * @param key The special key code
     */
    void handleSpecialKey(int key)
    {
        switch (key)
        {
        case 72: // Up arrow - Command history
            navigateHistory(-1);
            break;
            
        case 80: // Down arrow - Command history
            navigateHistory(1);
            break;
            
        case 75: // Left arrow - Cursor movement
            moveCursor(-1);
            break;
            
        case 77: // Right arrow - Cursor movement
            moveCursor(1);
            break;
            
        case 83: // Delete key
            handleDelete();
            break;
            
        case 71: // Home key
            cursorPos = 0;
            break;
            
        case 79: // End key
            cursorPos = currentInput.length();
            break;
            
        case 73: // Page Up
            // Could be used for scrolling command history
            break;
            
        case 81: // Page Down
            // Could be used for scrolling command history
            break;
            
        default:
            // Unhandled special key
            break;
        }
        
        refreshInputDisplay();
    }
    
    /**
     * Handles Enter key press - completes command input and sends via callback
     * Commands are now enqueued in CommandHandler via callback
     */
    void handleEnter()
    {
        string command = currentInput;
        
        // Add to command history if not empty and different from last command
        if (!command.empty())
        {
            if (commandHistory.empty() || commandHistory.back() != command)
            {
                commandHistory.push_back(command);
                
                // Limit history size to prevent memory issues
                if (commandHistory.size() > 100)
                {
                    commandHistory.erase(commandHistory.begin());
                }
            }
            
            // Send command to CommandHandler via callback (Producer)
            if (commandCallback)
            {
                commandCallback(command);
            }
        }
        
        // Reset input state
        clearInput();
        
        // Move to next line
        cout << endl;
    }
    
    /**
     * Handles backspace key
     */
    void handleBackspace()
    {
        if (cursorPos > 0)
        {
            currentInput.erase(cursorPos - 1, 1);
            cursorPos--;
            refreshInputDisplay();
        }
    }
    
    /**
     * Handles escape key - clears current input
     */
    void handleEscape()
    {
        clearInput();
        refreshInputDisplay();
    }
    
    /**
     * Handles tab key - provides basic auto-completion
     */
    void handleTab()
    {
        vector<string> commands = {
            "help", "start_marquee", "stop_marquee", 
            "set_text", "set_speed", "clear", "cls", "exit"
        };
        
        vector<string> matches;
        for (const string& cmd : commands)
        {
            if (cmd.length() >= currentInput.length() && 
                cmd.substr(0, currentInput.length()) == currentInput)
            {
                matches.push_back(cmd);
            }
        }
        
        if (matches.size() == 1)
        {
            currentInput = matches[0];
            cursorPos = currentInput.length();
            refreshInputDisplay();
        }
        else if (matches.size() > 1)
        {
            // Show available matches
            cout << endl << "Available completions: ";
            for (size_t i = 0; i < matches.size(); i++)
            {
                cout << matches[i];
                if (i < matches.size() - 1) cout << ", ";
            }
            cout << endl;
            refreshInputDisplay();
        }
    }
    
    /**
     * Handles Ctrl+C - interrupt current input
     */
    void handleCtrlC()
    {
        cout << "^C" << endl;
        clearInput();
        cout << "> ";
    }
    
    /**
     * Handles Ctrl+Z - suspend (for compatibility)
     */
    void handleCtrlZ()
    {
        cout << "^Z" << endl;
        clearInput();
        cout << "> ";
    }
    
    /**
     * Handles delete key
     */
    void handleDelete()
    {
        if (cursorPos < currentInput.length())
        {
            currentInput.erase(cursorPos, 1);
            refreshInputDisplay();
        }
    }
    
    /**
     * Inserts a character at the current cursor position
     * @param ch The character to insert
     */
    void insertCharacter(char ch)
    {
        currentInput.insert(cursorPos, 1, ch);
        cursorPos++;
        refreshInputDisplay();
    }
    
    /**
     * Navigates through command history
     * @param direction -1 for previous, 1 for next
     */
    void navigateHistory(int direction)
    {
        if (commandHistory.empty()) return;
        
        if (direction == -1) // Previous command
        {
            if (historyIndex == -1)
            {
                historyIndex = commandHistory.size() - 1;
            }
            else if (historyIndex > 0)
            {
                historyIndex--;
            }
        }
        else if (direction == 1) // Next command
        {
            if (historyIndex == -1) return;
            
            historyIndex++;
            if (historyIndex >= commandHistory.size())
            {
                historyIndex = -1;
                currentInput = "";
                cursorPos = 0;
                refreshInputDisplay();
                return;
            }
        }
        
        if (historyIndex != -1)
        {
            currentInput = commandHistory[historyIndex];
            cursorPos = currentInput.length();
        }
        
        refreshInputDisplay();
    }
    
    /**
     * Moves cursor left or right
     * @param direction -1 for left, 1 for right
     */
    void moveCursor(int direction)
    {
        if (direction == -1 && cursorPos > 0)
        {
            cursorPos--;
        }
        else if (direction == 1 && cursorPos < currentInput.length())
        {
            cursorPos++;
        }
    }
    
    /**
     * Gets system information for debugging
     * @return String with current keyboard handler state
     */
    string getSystemInfo()
    {
        string info = "Keyboard Handler Status:\n";
        info += " - Buffer size: " + to_string(keyBuffer.size()) + "\n";
        info += " - Current input: '" + currentInput + "'\n";
        info += " - Cursor position: " + to_string(cursorPos) + "\n";
        info += " - History size: " + to_string(commandHistory.size()) + "\n";
        info += " - History index: " + to_string(historyIndex) + "\n";
        return info;
    }
};