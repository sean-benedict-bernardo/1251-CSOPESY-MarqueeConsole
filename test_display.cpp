#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <conio.h>

// Simple test to verify ASCII art loading
#include "src/handlers/DisplayHandler.cpp"

using namespace std;

int main() {
    cout << "Testing DisplayHandler with ASCII art..." << endl;
    
    try {
        bool running = true;
        bool animating = false;
        int speed = 100;
        string text = "HELLO";
        
        DisplayHandler handler(&running, &animating, &speed, &text);
        
        cout << "DisplayHandler created successfully!" << endl;
        cout << "Press any key to continue..." << endl;
        _getch();
        
        // Test the welcome screen
        handler.displayWelcome();
        
        cout << "Test completed!" << endl;
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    
    return 0;
}