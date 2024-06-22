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

class DebuggerDelegate
{
public:
    using WPtrT = std::weak_ptr<DebuggerDelegate>;

    virtual void onBreakpointHit() = 0;
    virtual void onStepped() = 0;
    virtual void onPaused() = 0;
    virtual void threadStarted(int64_t aThreadId) = 0;

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
    int getSourceReferenceId();
    std::string getSourceContent();

    // stepForward() instructs the debugger to step forward one line.
    void stepForward();

    // clearBreakpoints() clears all set breakpoints.
    void clearBreakpoints();

    // addBreakpoint() sets a new breakpoint on the given line.
    void addBreakpoint(int64_t line);

    void setDelegate(const DebuggerDelegate::WPtrT &aDelegate);

    void launch(const std::string &aSourcePath, bool aStopOnNetry);
    void start(bool aStopOnNetry);

    bool loadSourceAtPath(const std::string &aSourcePath);

    std::vector<dap::Thread> getThreads();
    void createMainThread();
    dap::Thread getCurrentThread();

private:
    DebuggerDelegate::WPtrT     _delegate;

    std::mutex                  _mutex;

    int64_t                     _line = 1;
    std::filesystem::path       _sourcePath;
    std::string                 _sourceContent;

    std::unordered_set<int64_t> _breakpoints;

    std::vector<dap::Thread>    _threads;

};


#endif /* debugger_hpp */
