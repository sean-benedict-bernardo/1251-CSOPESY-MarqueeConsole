#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>

#define NUM_FRAMES 93
#define NUM_ROWS 20
#define FRAME_DIR "ascii_frames/"

using namespace std;
class Frame
{
private:
    string fileName;
    int frameNumber;

    vector<string> frameData;

    void encodeFrame()
    {
        // open file
        string fullDirectory = FRAME_DIR + fileName;
        ifstream inFile(fullDirectory);
        if (!inFile)
        {
            throw runtime_error("File not found: " + fullDirectory);
        }

        // Read frame data
        string line;
        int row = 0;
        while (getline(inFile, line) && row < NUM_ROWS)
        {
            this->frameData.push_back(line);
            ++row;
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

    vector<string> getFrameRows()
    {
        return this->frameData;
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
    Frames() {}

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

    vector<string> getFrame(int index)
    {
        try
        {
            return frames[index].getFrameRows();
        }
        catch (const std::exception &e)
        {
            if (index < 0 || index >= numFrames)
                throw runtime_error("Frame index out of bounds: " + to_string(index));

            return {e.what()};
        }
    }
};