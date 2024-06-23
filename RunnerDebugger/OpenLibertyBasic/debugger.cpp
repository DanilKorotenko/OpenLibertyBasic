//
//  debugger.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/20/24.
//

#include "debugger.hpp"

#include <fstream>

namespace
{

    // Total number of newlines in source.
    constexpr int64_t numSourceLines = 7;
}

Debugger::Debugger()
    : _mutex()
    , _line(0)
    , _breakpoints()
    , _threads()
{

}

void Debugger::run()
{
    std::unique_lock<std::mutex> lock(_mutex);

    for (int64_t i = 0; i < numSourceLines; i++)
    {
        int64_t l = ((_line + i) % numSourceLines) + 1;
        if (_breakpoints.count(l))
        {
            _line = l;
            lock.unlock();
            if (auto delegate = _delegate.lock())
            {
                delegate->onBreakpointHit();
            }
            return;
        }
    }
}

void Debugger::pause()
{
    if (auto delegate = _delegate.lock())
    {
        delegate->onPaused();
    }
}

int64_t Debugger::currentLine()
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _line;
}

void Debugger::stepForward()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _line = (_line % numSourceLines) + 1;
    lock.unlock();
    if (auto delegate = _delegate.lock())
    {
        delegate->onStepped();
    }
}

void Debugger::clearBreakpoints()
{
    std::unique_lock<std::mutex> lock(_mutex);
    this->_breakpoints.clear();
}

void Debugger::addBreakpoint(int64_t l)
{
    std::unique_lock<std::mutex> lock(_mutex);
    this->_breakpoints.emplace(l);
}

void Debugger::setDelegate(const DebuggerDelegate::WPtrT &aDelegate)
{
    _delegate = aDelegate;
}

void Debugger::launch(const std::string &aSourcePath, bool aStopOnNetry)
{
    _currentSource = std::make_shared<Source>(aSourcePath);
    start(aStopOnNetry);
}

void Debugger::start(bool aStopOnNetry)
{
    createMainThread();
    _line = 1;

}

std::vector<dap::Thread> Debugger::getThreads()
{
    return _threads;
}

void Debugger::createMainThread()
{
    dap::Thread thread;
    thread.id = 1;
    thread.name = "main";
    if (auto delegate = _delegate.lock())
    {
        delegate->threadStarted(thread.id);
    }

    _threads.push_back(thread);
}

dap::Thread Debugger::getCurrentThread()
{
    return _threads.back();
}
