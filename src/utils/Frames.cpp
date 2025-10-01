#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>

#define NUM_FRAMES 93
#define NUM_ROWS 20
#define FRAME_DIR "src/utils/data/ascii_frames/"

class Frame
{
private:
    std::string fileName;
    int frameNumber;

    std::vector<std::string> frameData;

    void encodeFrame()
    {
        // open file
        std::string fullDirectory = FRAME_DIR + fileName;
        std::ifstream inFile(fullDirectory);
        if (!inFile)
        {
            throw std::runtime_error("File not found: " + fullDirectory);
        }

        // Read frame data
        std::string line;
        int row = 0;
        while (std::getline(inFile, line) && row < NUM_ROWS)
        {
            this->frameData.push_back(line);
            ++row;
        }

        inFile.close();
    }

public:
    Frame() {}

    Frame(std::string fileName, int frameNumber)
    {
        this->fileName = fileName;
        this->frameNumber = frameNumber;

        this->encodeFrame();
    }

    std::vector<std::string> getFrameRows()
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
            std::string filename = filenameC;
            frames[i] = Frame(filename, i + 1);
        }
    }

    std::vector<std::string> getFrame(int index)
    {
        try
        {
            return frames[index].getFrameRows();
        }
        catch (const std::exception &e)
        {
            if (index < 0 || index >= numFrames)
                throw std::runtime_error("Frame index out of bounds: " + std::to_string(index));

            return {e.what()};
        }
    }
};