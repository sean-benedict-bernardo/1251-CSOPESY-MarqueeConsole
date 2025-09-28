#include <queue>
#include <string>
#include <iostream>

using namespace std;

class CommandHistory
{
private:
    vector<string> history;
    int rowHeight;

public:
    CommandHistory(int rowHeight = 50) : rowHeight(rowHeight) {}

    void addCommand(const string &command)
    {
        if (history.size() >= rowHeight)
            history.erase(history.begin());
        history.push_back(command);
    }

    vector<string> getHistory() const
    {
        return history;
    }
};
