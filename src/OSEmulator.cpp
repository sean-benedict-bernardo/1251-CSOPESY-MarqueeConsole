#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>
#include "handlers/CommandHandler.cpp"
#include "handlers/KeyboardHandler.cpp"

using namespace std;

class OSEmulator
{
private:
    // Core OS emulator state
    bool isRunning;
    bool isAnimating;
    int speed;
    string marqueeText;
    
    // Components
    CommandHandler* commandHandler;
    KeyboardHandler* keyboardHandler;
    
    // Console management
    HANDLE hConsole;
    
public:
    OSEmulator()
    {
        isRunning = true;
        isAnimating = false;
        speed = 100; // milliseconds
        marqueeText = "Welcome to CSOPESY Marquee Console";
        
        // Initialize components
        commandHandler = new CommandHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        keyboardHandler = new KeyboardHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    
    ~OSEmulator()
    {
        delete commandHandler;
        delete keyboardHandler;
    }
    
    /**
     * Main OS emulator loop
     * Handles keyboard polling, command processing, and marquee display
     */
    void run()
    {
        displayWelcome();
        
        auto lastMarqueeUpdate = chrono::steady_clock::now();
        int marqueePosition = 0;
        
        while (isRunning)
        {
            // Component 1: Keyboard Handler - handles keyboard buffering and polling
            keyboardHandler->pollKeyboard();
            string command = keyboardHandler->processBuffer();
            
            // Component 2: Command Interpreter - accepts command and controls marquee logic
            if (!command.empty())
            {
                vector<string> response = commandHandler->parseInput(command);
                
                // Component 3: Display Handler - handles display for command interpreter
                displayCommandResponse(response);
                displayPrompt();
            }
            
            // Component 4: Marquee Logic - handles animation logic for marquee text
            if (isAnimating)
            {
                auto now = chrono::steady_clock::now();
                auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastMarqueeUpdate);
                
                if (elapsed.count() >= speed)
                {
                    displayMarquee(marqueePosition);
                    marqueePosition = (marqueePosition + 1) % (marqueeText.length() + 80); // 80 is console width
                    lastMarqueeUpdate = now;
                }
            }
            
            // Small delay to prevent excessive CPU usage
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        
        displayExit();
    }

private:
    /**
     * Display Handler - Welcome screen
     */
    void displayWelcome()
    {
        clearScreen();
        cout << "============================================" << endl;
        cout << "    CSOPESY Marquee Console OS Emulator    " << endl;
        cout << "============================================" << endl;
        cout << endl;
        cout << "Type 'help' to see available commands." << endl;
        cout << "Type 'start_marquee' to begin animation." << endl;
        cout << endl;
        displayPrompt();
    }
    
    /**
     * Display Handler - Command response output
     */
    void displayCommandResponse(const vector<string>& response)
    {
        for (const string& line : response)
        {
            if (!line.empty())
            {
                cout << line << endl;
            }
        }
    }
    
    /**
     * Display Handler - Command prompt
     */
    void displayPrompt()
    {
        cout << "> ";
    }
    
    /**
     * Marquee Logic - Display marquee animation
     */
    void displayMarquee(int position)
    {
        // Save current cursor position
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        // Move to top of screen for marquee display
        COORD marqueePos = {0, 0};
        SetConsoleCursorPosition(hConsole, marqueePos);
        
        // Create marquee display
        string display = createMarqueeFrame(position);
        cout << display;
        
        // Restore cursor position
        SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
    }
    
    /**
     * Marquee Logic - Create marquee frame
     */
    string createMarqueeFrame(int position)
    {
        const int screenWidth = 80;
        string frame(screenWidth, ' ');
        
        // Calculate text position
        int textStart = screenWidth - position;
        int textEnd = textStart + marqueeText.length();
        
        // Fill the frame with text
        for (int i = 0; i < marqueeText.length(); i++)
        {
            int screenPos = textStart + i;
            if (screenPos >= 0 && screenPos < screenWidth)
            {
                frame[screenPos] = marqueeText[i];
            }
        }
        
        return "+" + string(screenWidth, '-') + "+\n|" + frame + "|\n+" + string(screenWidth, '-') + "+\n";
    }
    
    /**
     * Display Handler - Clear screen
     */
    void clearScreen()
    {
        system("cls");
    }
    
    /**
     * Display Handler - Exit message
     */
    void displayExit()
    {
        cout << endl << "Thank you for using CSOPESY Marquee Console!" << endl;
        cout << "Goodbye!" << endl;
    }
};

/**
 * Main entry point for the OS emulator
 */
int main()
{
    OSEmulator emulator;
    emulator.run();
    return 0;
}