#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include <vector>

using namespace std;

/**
 * Base Handler class interface
 * Provides common interface for all handler classes
 */
class Handler
{
public:
    /**
     * Virtual destructor for proper cleanup
     */
    virtual ~Handler() {}
    
    /**
     * Process any pending operations (called in main loop)
     */
    virtual void process() {}
    
    /**
     * Handle input and return response
     * @param input Input string to process
     * @return Vector of response strings
     */
    virtual vector<string> handleInput(const string& input) { return {}; }
    
    /**
     * Get the handler type name
     * @return String identifier for this handler type
     */
    virtual string getHandlerType() const { return "Handler"; }
    
    /**
     * Cleanup handler resources
     */
    virtual void cleanup() {}
};

#endif // HANDLER_H