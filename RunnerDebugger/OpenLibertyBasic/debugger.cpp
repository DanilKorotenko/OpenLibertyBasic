//
//  debugger.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/20/24.
//

#include "debugger.hpp"

#include <fstream>
#include <sstream>
#include <thread>

#include "StringUtils.hpp"

Debugger::Debugger()
    : _mutex()
    , _line(0)
    , _currentSource(nullptr)
    , _breakpoints()
    , _threads()
    , _paused(false)
    , _allowStep(false)
{

}

void Debugger::run()
{
    start(false);
}

void Debugger::pause()
{
    _paused.store(true);
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
    _allowStep = true;
    _allowStepCondVar.notify_all();
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
    if (aStopOnNetry)
    {
        pause();
    }

    std::thread thr (&Debugger::execute, this);
    thr.detach();
}

void Debugger::execute()
{
    std::istringstream istr(_currentSource->content());
    std::string line;
    while (std::getline(istr, line))
    {
        StringUtils::trim(line);
        if (!line.empty())
        {
            if (_paused.load())
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _allowStepCondVar.wait(lock,[this](){return _allowStep;});
            }
        }

        _allowStep = false;
        _line++;
        if (auto delegate = _delegate.lock())
        {
            delegate->onStepped();
        }
    }
    if (auto delegate = _delegate.lock())
    {
        delegate->onTerminated();
    }
    if (auto delegate = _delegate.lock())
    {
        delegate->onExited();
    }
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
        delegate->onThreadStarted(thread.id);
    }

    _threads.push_back(thread);
}

dap::Thread Debugger::getCurrentThread()
{
    return _threads.back();
}
