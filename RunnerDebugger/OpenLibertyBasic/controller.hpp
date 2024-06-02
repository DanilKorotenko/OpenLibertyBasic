//
//  controller.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 5/13/24.
//

#ifndef controller_hpp
#define controller_hpp

#include <stdio.h>

#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"

#include "debugger.hpp"
#include "event.hpp"

class Controller : public DebuggerDelegate
{
public:
    using PtrT = std::shared_ptr<Controller>;
    static PtrT create();

    Controller();
    ~Controller();

    void waitConfigured();
    void threadStarted();
    void pause();
    void waitTerminate();

private:
    void init();

    void onBreakpointHit();
    void onStepped();
    void onPaused();

    void onSessionError(const char *msg);

    void output(const char *msg);

    dap::InitializeResponse initializeRequest(const dap::InitializeRequest&);

private:
    Debugger::PtrT                  _debugger;
    std::unique_ptr<dap::Session>   _session;
    Event                           _configured;
    Event                           _terminate;
};

#endif /* controller_hpp */
