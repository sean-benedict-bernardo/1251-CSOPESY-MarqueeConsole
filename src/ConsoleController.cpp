#include "handlers/CommandHandler.cpp"
#include "handlers/DisplayHandler.cpp"
#include "handlers/KeyboardHandler.cpp"
#include "handlers/MarqueeLogicHandler.cpp"
#include <thread>
#include <chrono>

class ConsoleController {
private:
    // Shared state variables that all handlers will reference
    bool isRunning;
    bool isAnimating;
    int speed;
    std::string marqueeText;
    
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
        
        // Set up the marquee logic handler with initial text
        marqueeLogicHandler->setText(marqueeText);
        marqueeLogicHandler->initialize();
        marqueeLogicHandler->startScrolling();
    }
    
    void start() {
        // Initialize the display by clearing screen and drawing initial layout
        displayHandler->updateDisplay();
        
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
            }
            
            // Update marquee animation if enabled
            if (isAnimating) {
                marqueeLogicHandler->setText(marqueeText);
                marqueeLogicHandler->process();
                
                // Get the current marquee display and update display handler
                std::vector<std::string> marqueeDisplay = marqueeLogicHandler->getCurrentDisplay();
                // Note: You may need to implement a method in DisplayHandler to update marquee
                // For now, we'll let the display handler manage its own marquee updates
            }
            
            // Update the display
            displayHandler->updateDisplay();
            
            // Control the main loop speed
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // Clean up and exit
        marqueeLogicHandler->cleanup();
    }
};