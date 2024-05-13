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

class Controller : public DebuggerDelegate
{
public:
    using PtrT = std::shared_ptr<Controller>;
    static PtrT create();

    Controller();

    void onBreakpointHit();
    void onStepped();
    void onPaused();

private:
    Debugger::PtrT _debugger;
};

#endif /* controller_hpp */
