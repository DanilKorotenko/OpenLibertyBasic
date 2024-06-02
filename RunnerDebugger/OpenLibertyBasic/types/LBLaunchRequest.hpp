//
//  LBLaunchRequest.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/2/24.
//

#ifndef LBLaunchRequest_hpp
#define LBLaunchRequest_hpp

#include <dap/typeof.h>
#include "dap/protocol.h"

namespace dap
{

// Naming of this class follows the naming convention followed in DAP specification. These names map
// to the strings using in the launch request and hence will have to be in camel case as used here.
class LBLaunchRequest : public LaunchRequest
{
public:
    string program;
};

DAP_DECLARE_STRUCT_TYPEINFO(LBLaunchRequest);

}  // namespace dap

#endif /* LBLaunchRequest_hpp */
