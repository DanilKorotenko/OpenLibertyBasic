
#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"

#include "debugger.hpp"
#include "event.hpp"
#include "controller.hpp"

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <unordered_set>

#ifdef _MSC_VER
#define OS_WINDOWS 1
#endif

#ifdef OS_WINDOWS
#include <fcntl.h>  // _O_BINARY
#include <io.h>     // _setmode
#endif              // OS_WINDOWS

// main() entry point to the DAP server.
int main(int, char*[])
{
#ifdef OS_WINDOWS
    // Change stdin & stdout from text mode to binary mode.
    // This ensures sequences of \r\n are not changed to \n.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif  // OS_WINDOWS

    Controller::PtrT controller = Controller::create();

    controller->waitConfigured();

    controller->pause();

    controller->waitTerminate();

    return 0;
}
