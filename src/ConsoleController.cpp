#include "handlers/CommandHandler.cpp"
#include "handlers/DisplayHandler.cpp"
#include "handlers/KeyboardHandler.cpp"
#include "handlers/MarqueeLogicHandler.cpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

class ConsoleController {
private:
    // Shared state variables that all handlers will reference
    bool isRunning;
    bool isAnimating;
    int speed;
    std::string marqueeText;
    
    // GIF animation variables
    std::vector<std::vector<std::string>> gifFrames;
    int currentGifFrame;
    int gifSpeed; // milliseconds between frames
    bool isGifAnimating;
    
    // Handler instances
    CommandHandler* commandHandler;
    DisplayHandler* displayHandler;
    KeyboardHandler* keyboardHandler;
    MarqueeLogicHandler* marqueeLogicHandler;
    
public:
    ConsoleController() {
        // Initialize shared state
        isRunning = true;
        isAnimating = true;
        speed = 100; // Default marquee speed in milliseconds
        marqueeText = "Welcome to CSOPESY Marquee Console!";
        
        // Initialize handlers with shared state pointers
        commandHandler = new CommandHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        displayHandler = new DisplayHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        keyboardHandler = new KeyboardHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        marqueeLogicHandler = new MarqueeLogicHandler(80, 6); // 80 width, 6 height
        
        // Connect the handlers through callbacks
        connectHandlers();
    }
    
    ~ConsoleController() {
        // Clean up handler instances
        delete commandHandler;
        delete displayHandler;
        delete keyboardHandler;
        delete marqueeLogicHandler;
    }
    
    void connectHandlers() {
        // Connect KeyboardHandler to CommandHandler
        // KeyboardHandler will call CommandHandler's enqueueCommand when user presses Enter
        keyboardHandler->connectHandler([this](const std::string& command) {
            commandHandler->enqueueCommand(command);
        });
        
        // Connect KeyboardHandler to DisplayHandler for real-time input display
        keyboardHandler->connectInputDisplay([this](const std::string& currentInput) {
            displayHandler->updateInputLine(currentInput);
        });
        
        // Set up the marquee logic handler with initial text
        marqueeLogicHandler->setText(marqueeText);
        marqueeLogicHandler->initialize();
        marqueeLogicHandler->startScrolling();
    }
    
    void start() {
        // Initialize the display by clearing screen and drawing initial layout
        displayHandler->displayWelcome();
        
        bool needsDisplayUpdate = false;
        auto lastMarqueeUpdate = std::chrono::steady_clock::now();
        int marqueePosition = 0;
        
        // Main application loop
        while (isRunning) {
            // Process keyboard input (non-blocking)
            keyboardHandler->pollKeyboard();
            keyboardHandler->processBuffer();
            
            // Process any queued commands
            std::vector<std::string> commandResponses = commandHandler->processNextCommand();
            if (!commandResponses.empty()) {
                // Display command responses in the console area
                displayHandler->displayCommandResponse(commandResponses);
                needsDisplayUpdate = true;
            }
            
            // Update marquee animation if enabled (throttled)
            if (isAnimating) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMarqueeUpdate);
                
                if (elapsed.count() >= speed) {
                    marqueePosition++;
                    displayHandler->updateMarqueePosition(marqueePosition);
                    lastMarqueeUpdate = now;
                    needsDisplayUpdate = true;
                }
            }
            
            // Only update display when necessary
            if (needsDisplayUpdate) {
                displayHandler->updateDisplay();
                needsDisplayUpdate = false;
            }
            
            // Control the main loop speed - longer sleep since we're not updating constantly
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Show exit message
        displayHandler->displayExit();
    }
};