//
//  debugger.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/20/24.
//

#ifndef debugger_hpp
#define debugger_hpp

#include <stdio.h>

#include "dap/protocol.h"

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <unordered_set>
#include <filesystem>

#include "types/Source.hpp"

class DebuggerDelegate
{
public:
    using WPtrT = std::weak_ptr<DebuggerDelegate>;

    virtual void onBreakpointHit() = 0;
    virtual void onStepped() = 0;
    virtual void onPaused() = 0;
    virtual void onThreadStarted(int64_t aThreadId) = 0;
    virtual void onTerminated() = 0;
    virtual void onExited() = 0;
};

// Debugger holds the dummy debugger state and fires events to the EventHandler
// passed to the constructor.
class Debugger
{
public:
    using PtrT = std::shared_ptr<Debugger>;

    Debugger();

    // run() instructs the debugger to continue execution.
    void run();

    // pause() instructs the debugger to pause execution.
    void pause();

    // currentLine() returns the currently executing line number.
    int64_t currentLine();

    Source::PtrT getCurrentSource() { return _currentSource; }

    // stepForward() instructs the debugger to step forward one line.
    void stepForward();

    // clearBreakpoints() clears all set breakpoints.
    void clearBreakpoints();

    // addBreakpoint() sets a new breakpoint on the given line.
    void addBreakpoint(int64_t line);

    void setDelegate(const DebuggerDelegate::WPtrT &aDelegate);

    void launch(const std::string &aSourcePath, bool aStopOnNetry);
    void start(bool aStopOnNetry);

    std::vector<dap::Thread> getThreads();
    void createMainThread();
    dap::Thread getCurrentThread();

private:
    void execute();
private:
    DebuggerDelegate::WPtrT     _delegate;

    std::mutex                  _mutex;

    int64_t                     _line = 1;
    Source::PtrT                _currentSource;

    std::unordered_set<int64_t> _breakpoints;

    std::vector<dap::Thread>    _threads;
    std::atomic<bool>           _paused;

    std::condition_variable     _allowStepCondVar;
    bool                        _allowStep;
};

#endif /* debugger_hpp */
