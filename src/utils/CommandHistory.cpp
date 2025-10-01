#include <queue>
#include <string>
#include <iostream>

class CommandHistory
{
private:
    std::vector<std::string> history;
    int rowHeight;

public:
    CommandHistory(int rowHeight = 50) : rowHeight(rowHeight) {}

    void addCommand(const std::string &command)
    {
        if (history.size() >= rowHeight)
            history.erase(history.begin());
        history.push_back(command);
    }

    std::vector<std::string> getHistory() const
    {
        return history;
    }
};
