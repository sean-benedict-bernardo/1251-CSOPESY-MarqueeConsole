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
        speed = 100; // Default animation speed in milliseconds (used for both marquee and GIF)
        marqueeText = "Welcome to CSOPESY Marquee Console!";
        
        // Initialize GIF animation variables
        currentGifFrame = 0;
        isGifAnimating = true;
        
        // Initialize handlers with shared state pointers
        commandHandler = new CommandHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        displayHandler = new DisplayHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        keyboardHandler = new KeyboardHandler(&isRunning, &isAnimating, &speed, &marqueeText);
        marqueeLogicHandler = new MarqueeLogicHandler(80, 6); // 80 width, 6 height
        
        // Connect the handlers through callbacks
        connectHandlers();
        
        // Load GIF frames from data folder
        loadGifFrames();
    }
    
    void loadGifFrames() {
        const std::string framesPath = "utils/data/ascii_frames/";
        
        // Load frames frame_01.txt through frame_93.txt
        for (int i = 1; i <= 93; i++) {
            std::string filename = framesPath + "frame_" + std::string(2 - std::to_string(i).length(), '0') + std::to_string(i) + ".txt";
            std::ifstream file(filename);
            
            if (file.is_open()) {
                std::vector<std::string> frame;
                std::string line;
                
                while (std::getline(file, line)) {
                    frame.push_back(line);
                }
                
                if (!frame.empty()) {
                    gifFrames.push_back(frame);
                }
                
                file.close();
            }
        }
        
        // If frames loaded successfully, start the GIF
        if (!gifFrames.empty() && displayHandler) {
            displayHandler->updateGifFrame(gifFrames[0]);
        }
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

        // Connect CommandHandler to MarqueeLogicHandler for text changes
        commandHandler->connectMarqueeTextChange([this](const std::string& newText) {
            marqueeLogicHandler->setText(newText);
        });
        
        // Set up the marquee logic handler with initial text and speed
        marqueeLogicHandler->setText(marqueeText);
        marqueeLogicHandler->setAnimationSpeed(speed);
        marqueeLogicHandler->initialize();
        marqueeLogicHandler->startScrolling();
        
        // Get initial marquee display and send to DisplayHandler
        std::vector<std::string> initialMarqueeDisplay = marqueeLogicHandler->getCurrentDisplay();
        displayHandler->updateMarqueeDisplay(initialMarqueeDisplay);
    }
    
    void start() {
        // Initialize the display by clearing screen and drawing initial layout
        displayHandler->displayWelcome();
        
        bool needsDisplayUpdate = false;
        auto lastMarqueeUpdate = std::chrono::steady_clock::now();
        auto lastGifUpdate = std::chrono::steady_clock::now();
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
                    // Update MarqueeLogicHandler's animation speed and process
                    marqueeLogicHandler->setAnimationSpeed(speed);
                    marqueeLogicHandler->process();
                    
                    // Get the current display from MarqueeLogicHandler and send to DisplayHandler
                    std::vector<std::string> marqueeDisplay = marqueeLogicHandler->getCurrentDisplay();
                    displayHandler->updateMarqueeDisplay(marqueeDisplay);
                    
                    lastMarqueeUpdate = now;
                    needsDisplayUpdate = true;
                }
            }
            
            // Update GIF animation if enabled and frames are loaded (throttled)
            if (isGifAnimating && !gifFrames.empty()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastGifUpdate);
                
                if (elapsed.count() >= speed) {
                    currentGifFrame = (currentGifFrame + 1) % gifFrames.size();
                    displayHandler->updateGifFrame(gifFrames[currentGifFrame]);
                    lastGifUpdate = now;
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