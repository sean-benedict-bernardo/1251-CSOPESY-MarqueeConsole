#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <conio.h> // for _kbhit()

#define NUM_FRAMES 93
#define NUM_ROWS 20
#define FRAME_DIR "ascii_frames/"

void clearScreen()
{
    system("cls");
}

using namespace std;
class Frame
{
private:
    string fileName;
    int frameNumber;

    string frameData = "\n";

    void encodeFrame()
    {
        // open file
        ifstream inFile(FRAME_DIR + fileName);
        if (!inFile)
        {
            cerr << "Error opening file: " << FRAME_DIR + fileName << endl;
            return;
        }

        // Read frame data
        string line;
        int row = 0;
        while (getline(inFile, line) && row < NUM_ROWS)
        {
            this->frameData += line + "\n";
        }

        inFile.close();
    }

public:
    Frame() {}

    Frame(string fileName, int frameNumber)
    {
        this->fileName = fileName;
        this->frameNumber = frameNumber;

        this->encodeFrame();
    }

    void printFrame()
    {
        cout << this->frameData << endl;
    }
};

class Frames
{
private:
    Frame frames[NUM_FRAMES];
    int numFrames;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

public:
    Frames(int numFrames = NUM_FRAMES)
    {
        this->numFrames = numFrames;
        for (int i = 0; i < numFrames; i++)
        {
            char filenameC[100];
            sprintf(filenameC, "frame_%02d.txt", i + 1);
            string filename = filenameC;
            frames[i] = Frame(filename, i + 1);
        }
    }

    void animate(std::atomic<bool> &running, std::atomic<bool> &pause, std::atomic<int> &speed)
    {
        GetConsoleScreenBufferInfo(this->hConsole, &this->csbi);
        COORD savedPos = {0, 0};

        int i = 0;
        // int count_mode = 0;

        while (running)
        {
            if (pause)
                continue;

            SetConsoleCursorPosition(this->hConsole, savedPos);
            frames[i].printFrame();

            Sleep(speed);

            /**
             *
             if (count_mode == 0)
             {
                i++;
                if (i == this->numFrames - 1)
                count_mode = 1;
            }
            else
            {
                i--;
                if (i == 0)
                count_mode = 0;
            }
            */
            i = (i + 1) % this->numFrames;
        }
    }
};

int main()
{
    Frames frames;
    std::atomic<bool> running(true);
    std::atomic<bool> pause(false);
    std::atomic<int> speed(100);
    clearScreen();
    cout << "Press 'q' to quit.\n";
    // Start animation in a separate thread
    std::thread animThread(&Frames::animate, &frames, std::ref(running), std::ref(pause), std::ref(speed));

    // Handle user input while animation runs
    short int line_char_count = 0;
    short int row_count = 0;
    while (running)
    {
        if (_kbhit())
        {
            char ch = _getch();

            switch (ch)
            {
            case 'q':
                running = false; // Stop animation
                break;
            case 'p':
                pause = !pause; // Toggle pause
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                speed = 100 - (ch - '0') * 10; // Adjust speed (0-9)
                break;
            case '\r':
                line_char_count = 0; // Reset character count on new line
                row_count++;
            case '\b':
            {
                // write whitespace to erase character
                COORD erasePos = {(short int)(line_char_count - 1), (short int)(NUM_ROWS + row_count + 2)};
                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), erasePos);
                cout << ' ';
                if (line_char_count > 0)
                    line_char_count--;
            }
            break;
            default:
                line_char_count++;
                break;
            }

            // Move cursor below animation before printing
            COORD inputPos = {line_char_count, (short int)(NUM_ROWS + row_count + 2)};
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), inputPos);
            cout << ch;
        }
        Sleep(50);
    }

    animThread.join();
    // clear screen
    clearScreen();
    return 0;
}
