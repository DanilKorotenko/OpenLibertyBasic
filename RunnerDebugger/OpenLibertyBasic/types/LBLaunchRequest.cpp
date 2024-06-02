//
//  LBLaunchRequest.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/2/24.
//

#include "LBLaunchRequest.hpp"

namespace dap
{

DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT(LBLaunchRequest, LaunchRequest, "launch",
    DAP_FIELD(program, "program"),
    DAP_FIELD(stopOnEntry, "stopOnEntry"))

}  // namespace dap
