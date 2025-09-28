#include <string>
#include <vector>
#include <iostream>

#define DEBUG true

using namespace std;

typedef struct CommandStruct
{
    string command;
    vector<string> arguments;
} Command;

class CommandHandler
{
public:
    bool *isRunning;
    bool *isAnimating;
    int *speed;

    CommandHandler(bool *isRunning, bool *isAnimating, int *speed)
    {
        this->isRunning = isRunning;
        this->isAnimating = isAnimating;
        this->speed = speed;
    }

    vector<string> parseInput(string input)
    {
        Command cmd = parseCommand(input);
        vector<string> display = commandController(cmd.command, cmd.arguments);

        return display;
    }

private:
    /**
     * Splits the input string into a vector of arguments based on spaces.
     * @param input The input string to split.
     * @return A vector of arguments.
     */
    Command parseCommand(string input)
    {
        string command = "";
        Command cmd = {
            .command = "",
            .arguments = {},
        };

        for (int i = 0; i < input.length(); i++)
        {
            if (input[i] == ' ')
            {
                if (!command.empty())
                {
                    if (cmd.command.empty())
                    {
                        cmd.command = command;
                    }
                    else
                    {
                        cmd.arguments.push_back(command);
                    }
                    command = "";
                }
            }
            else
            {
                command += input[i];
            }
        }
        // Handle last token
        if (!command.empty())
        {
            if (cmd.command.empty())
            {
                cmd.command = command;
            }
            else
            {
                cmd.arguments.push_back(command);
            }
        }

        if (!command.empty())
            cmd.arguments.push_back(command);

        return cmd;
    }

    /**
     * Handles command execution based on the input command and its arguments.
     * @param command The command to execute.
     * @param arguments The arguments associated with the command.
     * @return vector of response messages. see indivudal functions for actual return types.
     */
    vector<string> commandController(string command, const vector<string> &arguments = {})
    {
        if (command == "")
            return {""};
        else if (command == "help")
            return this->getHelp();
        else if (command == "start_marquee")
        {
            return {this->startMarquee()};
        }
        else if (command == "stop_marquee")
            return {this->stopMarquee()};
        else if (command == "set_speed" || command == "speed")
        {
            if (arguments.size() < 2)
                return {"Error: set_speed requires an argument."};
            else
                try
                {
                    int speed = stoi(arguments[1]);
                    return {this->setSpeed(speed)};
                }
                catch (const std::exception &e)
                {
                    return {"Error: Invalid speed value."};
                }
        }
        else if (command == "clear" || command == "cls")
        {
            this->clearScreen();
            return {""};
        }
        else if (command == "status" && DEBUG)
            return {this->status()};
        else if (command == "exit")
            return {this->exitProgram()};

        return {"Error: Unknown command '" + command + "'."};
    }

    /**
     * Returns a list of available commands and their descriptions.
     * @return vector of help messages.
     */
    vector<string> getHelp()
    {
        vector<string> helpMessages;
        helpMessages.push_back("Available commands:");
        helpMessages.push_back(" - help               Show this help message");
        helpMessages.push_back(" - start_marquee      Start the marquee animation");
        helpMessages.push_back(" - stop_marquee       Stop the marquee animation");
        helpMessages.push_back(" - set_speed <value>  Set the speed of the marquee animation");
        helpMessages.push_back(" - clear              Clear the console screen");
        helpMessages.push_back(" - exit               Exit the program");

        return helpMessages;
    }

    /**
     * Starts the marquee animation by setting isAnimating to true.
     * @return message indicating the marquee has been started or was already running.
     */
    string startMarquee()
    {
        if (!*this->isAnimating)
        {
            *this->isAnimating = true;
            return "Marquee started.";
        }
        else
        {
            return "Marquee is already running.";
        }
    }

    /**
     * Stops the marquee animation by setting isAnimating to false.
     * @return message indicating the marquee has been stopped or was not running.
     */
    string stopMarquee()
    {
        if (*this->isAnimating)
        {
            *this->isAnimating = false;
            return "Marquee stopped.";
        }
        else
        {
            return "Marquee is not running.";
        }
    }

    /**
     * Sets the speed of the marquee animation.
     * @param speed The new speed value.
     * @return A message indicating the new speed.
     */
    string setSpeed(int speed)
    {
        // Implementation to set speed
        if (speed == *this->speed)
            return "Marquee speed is already set to " + to_string(speed) + ".";

        *this->speed = speed;
        return "Marquee speed set to " + to_string(speed) + ".";
    }

    string status()
    {
        string statusMessage;
        statusMessage += "isRunning: " + string(*this->isRunning ? "true" : "false") + ", ";
        statusMessage += "isAnimating: " + string(*this->isAnimating ? "true" : "false") + ", ";
        statusMessage += "speed: " + to_string(*this->speed);
        return statusMessage;
    }

    /**
     * Exits the program by setting isRunning to false.
     * @return A message indicating the program is exiting.
     */
    string exitProgram()
    {
        *this->isRunning = false;
        return "Exiting program.";
    }

    /**
     * Clears the console screen.
     */
    void clearScreen()
    {
        system("cls");
    }
};

int main()
{
    bool isRunning = true;
    bool isAnimating = true;
    int speed = 1;

    CommandHandler parser(&isRunning, &isAnimating, &speed);

    string input;
    while (isRunning)
    {
        cout << "> ";
        getline(cin, input);
        vector<string> response = parser.parseInput(input);

        for (int i = 0; i < response.size(); i++)
        {
            cout << response[i] << endl;
        }
    }

    if (DEBUG)
        printf("%s, %s, %d\n", (isRunning ? "true" : "false"), (isAnimating ? "true" : "false"), speed);

    return 0;
}