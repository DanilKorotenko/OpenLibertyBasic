//
//  debugger.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/20/24.
//

#include "debugger.hpp"

namespace
{

    // sourceContent holds the synthetic file source.
//    constexpr char sourceContent[] = R"(// Hello Debugger!
//
//This is a synthetic source file provided by the DAP debugger.
//
//You can set breakpoints, and single line step.
//
//You may also notice that the locals contains a single variable for the currently executing line number.)";

    // Total number of newlines in source.
    constexpr int64_t numSourceLines = 7;
}

Debugger::Debugger(const EventHandler& onEvent)
    : _onEvent(onEvent)
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
            _onEvent(Event::BreakpointHit);
            return;
        }
    }
}

void Debugger::pause()
{
    _onEvent(Event::Paused);
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
    _onEvent(Event::Stepped);
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
