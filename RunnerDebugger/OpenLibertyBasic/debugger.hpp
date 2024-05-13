//
//  debugger.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/20/24.
//

#ifndef debugger_hpp
#define debugger_hpp

#include <stdio.h>

#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <unordered_set>

// Debugger holds the dummy debugger state and fires events to the EventHandler
// passed to the constructor.
class Debugger
{
public:
    enum class EventType
    {
        BreakpointHit,
        Stepped,
        Paused
    };

    using EventHandler = std::function<void(EventType)>;

    Debugger();

    // run() instructs the debugger to continue execution.
    void run();

    // pause() instructs the debugger to pause execution.
    void pause();

    // currentLine() returns the currently executing line number.
    int64_t currentLine();

    // stepForward() instructs the debugger to step forward one line.
    void stepForward();

    // clearBreakpoints() clears all set breakpoints.
    void clearBreakpoints();

    // addBreakpoint() sets a new breakpoint on the given line.
    void addBreakpoint(int64_t line);

    void setBreakpointHit(const EventHandler &aBreakpointHit);
    void setStepped(const EventHandler &aStepped);
    void setPaused(const EventHandler &aPaused);

private:
    EventHandler                _onBreakpointHit;
    EventHandler                _onStepped;
    EventHandler                _onPaused;

    std::mutex                  _mutex;
    int64_t                     _line = 1;
    std::unordered_set<int64_t> _breakpoints;
};


#endif /* debugger_hpp */
