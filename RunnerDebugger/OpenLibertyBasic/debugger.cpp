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
    , _sourcePath()
    , _sourceContent()
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

std::string Debugger::getSourceContent()
{
    return _sourceContent;
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
    loadSourceAtPath(aSourcePath);
    start(aStopOnNetry);
}

bool Debugger::loadSourceAtPath(const std::string &aSourcePath)
{
    _sourcePath = aSourcePath;

    std::ifstream sourceFile(aSourcePath, std::ios::in);
    if (!sourceFile.is_open())
    {
        return false;
    }
    _sourceContent = std::string(std::istreambuf_iterator<char>(sourceFile), std::istreambuf_iterator<char>());
    sourceFile.close();

    return true;
}

void Debugger::start(bool aStopOnNetry)
{
    _line = 1;

}
