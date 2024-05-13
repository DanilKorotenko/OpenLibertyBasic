//
//  controller.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 5/13/24.
//

#include "controller.hpp"

Controller::PtrT Controller::create()
{
    Controller::PtrT result = std::make_shared<Controller>();
    result->_debugger->setDelegate(result);
    return result;
}


Controller::Controller()
    : _debugger(std::make_shared<Debugger>())
{

}

void Controller::onBreakpointHit()
{

}

void Controller::onStepped()
{

}

void Controller::onPaused()
{

}
