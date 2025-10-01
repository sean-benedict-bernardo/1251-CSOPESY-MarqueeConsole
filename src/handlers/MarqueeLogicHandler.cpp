#include "FileReader.cpp"
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;

/**
 * MarqueeLogicHandler - Handles marquee text animation and ASCII art rendering
 * Converted from marquee_logic.c and enhanced with Handler architecture
 */
class MarqueeLogicHandler
{
private:
    // Text and animation state
    string currentText;
    int scrollPosition;
    int animationSpeed;  // milliseconds between updates
    
    // ASCII art management
    FileReader* fileReader;
    bool useAsciiArt;
    
    // Display properties
    int displayWidth;
    int displayHeight;
    vector<vector<char>> displayBuffer;  // 2D character buffer
    
    // Animation timing
    chrono::steady_clock::time_point lastUpdate;
    
    // Thread safety
    mutable mutex textMutex;
    
    // State flags
    bool needsUpdate;
    bool isScrolling;
    
public:
    /**
     * Constructor for MarqueeLogicHandler
     * @param width Display width for the marquee
     * @param height Display height for the marquee
     */
    MarqueeLogicHandler(int width = 80, int height = 6)
    {
        currentText = "";
        scrollPosition = 0;
        animationSpeed = 100;  // 100ms default
        displayWidth = width;
        displayHeight = height;
        useAsciiArt = false;
        needsUpdate = false;
        isScrolling = false;
        
        // Initialize file reader for ASCII art
        fileReader = new FileReader();
        
        // Initialize display buffer
        initializeDisplayBuffer();
        
        lastUpdate = chrono::steady_clock::now();
    }
    
    /**
     * Destructor
     */
    ~MarqueeLogicHandler()
    {
        if (fileReader)
        {
            delete fileReader;
        }
    }

protected:
    /**
     * Initialize the marquee handler
     */
    void initialize()
    {
        // Load ASCII art characters
        try
        {
            fileReader->loadAllFiles();
            useAsciiArt = true;
            std::cout << "MarqueeLogicHandler: ASCII art loaded successfully" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "MarqueeLogicHandler: Could not load ASCII art: " << e.what() << std::endl;
            useAsciiArt = false;
        }
        
        needsUpdate = true;
    }
    
    /**
     * Main processing loop - handles marquee animation
     */
    void process()
    {
        auto now = chrono::steady_clock::now();
        
        // Check if it's time to update animation
        if (isScrolling && 
            chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= animationSpeed)
        {
            {
                std::lock_guard<std::mutex> lock(textMutex);
                
                if (!currentText.empty())
                {
                    // Update scroll position
                    scrollPosition = (scrollPosition + 1) % (currentText.length() + displayWidth);
                    needsUpdate = true;
                }
            }
            
            lastUpdate = now;
        }
        
        // Update display buffer if needed
        if (needsUpdate)
        {
            updateDisplayBuffer();
            needsUpdate = false;
        }
    }
    
    /**
     * Cleanup resources
     */
    void cleanup()
    {
        std::lock_guard<std::mutex> lock(textMutex);
        currentText = "";
        clearDisplayBuffer();
    }

public:
    /**
     * Handle input method required by base Handler class
     */
    vector<string> handleInput(const string& input)
    {
        vector<string> response;
        response.push_back("MarqueeLogicHandler processes marquee text and animations");
        return response;
    }
    
    /**
     * Get handler type
     */
    string getHandlerType() const
    {
        return "MarqueeLogicHandler";
    }
    
    /**
     * Set the marquee text
     * @param text New text to display
     */
    void setText(const string& text)
    {
        std::lock_guard<std::mutex> lock(textMutex);
        currentText = text;
        scrollPosition = 0;  // Reset scroll position
        needsUpdate = true;
    }
    
    /**
     * Get the current marquee text
     * @return Current text being displayed
     */
    string getText() const
    {
        std::lock_guard<std::mutex> lock(textMutex);
        return currentText;
    }
    
    /**
     * Set animation speed
     * @param speed Milliseconds between animation updates
     */
    void setAnimationSpeed(int speed)
    {
        std::lock_guard<std::mutex> lock(textMutex);
        animationSpeed = max(10, speed);  // Minimum 10ms
    }
    
    /**
     * Start scrolling animation
     */
    void startScrolling()
    {
        std::lock_guard<std::mutex> lock(textMutex);
        isScrolling = true;
        needsUpdate = true;
    }
    
    /**
     * Stop scrolling animation
     */
    void stopScrolling()
    {
        std::lock_guard<std::mutex> lock(textMutex);
        isScrolling = false;
    }
    
    /**
     * Check if currently scrolling
     * @return True if scrolling is active
     */
    bool getIsScrolling() const
    {
        std::lock_guard<std::mutex> lock(textMutex);
        return isScrolling;
    }
    
    /**
     * Enable or disable ASCII art mode
     * @param enabled True to use ASCII art, false for plain text
     */
    void setAsciiArtMode(bool enabled)
    {
        std::lock_guard<std::mutex> lock(textMutex);
        useAsciiArt = enabled && fileReader != nullptr;
        needsUpdate = true;
    }
    
    /**
     * Get the current display as vector of strings
     * @return Vector of strings representing the current marquee display
     */
    vector<string> getCurrentDisplay()
    {
        std::lock_guard<std::mutex> lock(textMutex);
        
        vector<string> display;
        for (int row = 0; row < displayHeight; row++)
        {
            string line;
            for (int col = 0; col < displayWidth; col++)
            {
                line += displayBuffer[row][col];
            }
            display.push_back(line);
        }
        
        return display;
    }
    
    /**
     * Get a single line of the current display (for simple marquee)
     * @return Single string with the current marquee line
     */
    string getCurrentLine()
    {
        vector<string> display = getCurrentDisplay();
        if (!display.empty())
        {
            // Return the middle line for single-line marquee
            int middleLine = displayHeight / 2;
            if (middleLine < display.size())
            {
                return display[middleLine];
            }
        }
        return "";
    }

private:
    /**
     * Initialize the display buffer
     */
    void initializeDisplayBuffer()
    {
        displayBuffer.clear();
        displayBuffer.resize(displayHeight);
        
        for (int row = 0; row < displayHeight; row++)
        {
            displayBuffer[row].resize(displayWidth, ' ');
        }
    }
    
    /**
     * Clear the display buffer
     */
    void clearDisplayBuffer()
    {
        for (int row = 0; row < displayHeight; row++)
        {
            for (int col = 0; col < displayWidth; col++)
            {
                displayBuffer[row][col] = ' ';
            }
        }
    }
    
    /**
     * Update the display buffer with current text and scroll position
     */
    void updateDisplayBuffer()
    {
        clearDisplayBuffer();
        
        if (currentText.empty())
        {
            return;
        }
        
        if (useAsciiArt && fileReader)
        {
            renderAsciiArt();
        }
        else
        {
            renderPlainText();
        }
    }
    
    /**
     * Render text using ASCII art characters
     */
    void renderAsciiArt()
    {
        // Calculate visible characters based on scroll position
        int textLength = currentText.length();
        int extendedLength = textLength + displayWidth;
        
        // Determine which characters are visible
        vector<char> visibleChars;
        for (int i = 0; i < displayWidth; i++)
        {
            int textIndex = (scrollPosition + i) % extendedLength;
            if (textIndex < textLength)
            {
                visibleChars.push_back(currentText[textIndex]);
            }
            else
            {
                visibleChars.push_back(' ');  // Padding space
            }
        }
        
        // Render each character using ASCII art
        int charWidth = 8;  // Assume each ASCII character is 8 columns wide
        int charsPerLine = displayWidth / charWidth;
        
        for (int charIndex = 0; charIndex < charsPerLine && charIndex < visibleChars.size(); charIndex++)
        {
            char c = visibleChars[charIndex];
            if (c != ' ' && fileReader->hasArt(c))
            {
                vector<string> art = fileReader->lookupArt(c);
                
                // Place ASCII art in display buffer
                for (int row = 0; row < min((int)art.size(), displayHeight); row++)
                {
                    int startCol = charIndex * charWidth;
                    for (int col = 0; col < min((int)art[row].length(), charWidth); col++)
                    {
                        if (startCol + col < displayWidth)
                        {
                            displayBuffer[row][startCol + col] = art[row][col];
                        }
                    }
                }
            }
        }
    }
    
    /**
     * Render text as plain characters (fallback mode)
     */
    void renderPlainText()
    {
        // Use middle row for single-line text
        int textRow = displayHeight / 2;
        
        // Calculate visible portion of text
        int textLength = currentText.length();
        int extendedLength = textLength + displayWidth;
        
        for (int col = 0; col < displayWidth; col++)
        {
            int textIndex = (scrollPosition + col) % extendedLength;
            if (textIndex < textLength)
            {
                displayBuffer[textRow][col] = currentText[textIndex];
            }
            else
            {
                displayBuffer[textRow][col] = ' ';  // Padding space
            }
        }
    }
    
    /**
     * Rotate/shift logic (converted from original C function)
     * @param row Pointer to row array
     * @param cols Number of columns
     */
    void rotateLeft(vector<char>& row)
    {
        if (row.size() <= 1) return;
        
        char first = row[0];
        for (size_t j = 0; j < row.size() - 1; j++)
        {
            row[j] = row[j + 1];
        }
        row[row.size() - 1] = first;
    }
    
    /**
     * Apply marquee logic to entire display buffer
     * (converted from original marqueeLogic function)
     */
    void applyMarqueeLogic()
    {
        for (int row = 0; row < displayHeight; row++)
        {
            rotateLeft(displayBuffer[row]);
        }
    }
    
    /**
     * Get status information for debugging
     */
    string getStatusInfo() const
    {
        std::lock_guard<std::mutex> lock(textMutex);
        
        string info = "MarqueeLogicHandler Status:\n";
        info += " - Text: \"" + currentText + "\"\n";
        info += " - Scroll Position: " + to_string(scrollPosition) + "\n";
        info += " - Animation Speed: " + to_string(animationSpeed) + "ms\n";
        info += " - Is Scrolling: " + string(isScrolling ? "Yes" : "No") + "\n";
        info += " - ASCII Art Mode: " + string(useAsciiArt ? "Enabled" : "Disabled") + "\n";
        info += " - Display Size: " + to_string(displayWidth) + "x" + to_string(displayHeight) + "\n";
        
        if (fileReader)
        {
            vector<char> loadedKeys = fileReader->getLoadedKeys();
            info += " - Loaded ASCII Characters: " + to_string(loadedKeys.size()) + "\n";
        }
        
        return info;
    }
};