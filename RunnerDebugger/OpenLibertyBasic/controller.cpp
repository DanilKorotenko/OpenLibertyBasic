//
//  controller.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 5/13/24.
//

#include "controller.hpp"
//#include <os/log.h>\''\}


const dap::integer threadId = 100;
const dap::integer frameId = 200;
const dap::integer variablesReferenceId = 300;
const dap::integer sourceReferenceId = 400;

namespace
{

    // sourceContent holds the synthetic file source.
    constexpr char sourceContent[] = R"(// Hello Debugger!

This is a synthetic source file provided by the DAP debugger.

You can set breakpoints, and single line step.

You may also notice that the locals contains a single variable for the currently executing line number.)";

    // Total number of newlines in source.
    constexpr int64_t numSourceLines = 7;

}  // anonymous namespace


Controller::PtrT Controller::create()
{
    Controller::PtrT result = std::make_shared<Controller>();

    result->_debugger->setDelegate(result);
    result->init();

    return result;
}

Controller::Controller()
    : _debugger(std::make_shared<Debugger>())
    , _session(dap::Session::create())
    , _configured()
    , _terminate()
    , _log(dap::file("log.log"))
{
}

Controller::~Controller()
{
    if (_dapSocketFile)
    {
        fclose(_dapSocketFile);
    }
}


void Controller::init()
{
//    dap::writef(_log, "init()\n");

    _session->onError(std::bind(&Controller::onSessionError, this, std::placeholders::_1));

    // The Initialize request is the first message sent from the client and
    // the response reports debugger capabilities.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
    _session->registerHandler(
        [&](const dap::InitializeRequest&)
        {
//            os_log_debug(OS_LOG_DEFAULT, "InitializeRequest");
//            dap::writef(_log, "InitializeRequest\n");

            dap::InitializeResponse response;
            response.supportsConfigurationDoneRequest = true;
//            os_log_debug(OS_LOG_DEFAULT, "InitializeResponse");
            return response;
        });

    // When the Initialize response has been sent, we need to send the initialized
    // event.
    // We use the registerSentHandler() to ensure the event is sent *after* the
    // initialize response.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Initialized
    _session->registerSentHandler(
        [&](const dap::ResponseOrError<dap::InitializeResponse>&)
        {
//            dap::writef(_log, "InitializedEvent\n");

            _session->send(dap::InitializedEvent());
        });

    // The Threads request queries the debugger's list of active threads.
    // This example debugger only exposes a single thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
    _session->registerHandler(
        [&](const dap::ThreadsRequest&)
        {
//            dap::writef(_log, "ThreadsRequest\n");

            dap::ThreadsResponse response;
            dap::Thread thread;
            thread.id = threadId;
            thread.name = "TheThread";
            response.threads.push_back(thread);
            return response;
        });

    // The StackTrace request reports the stack frames (call stack) for a given
    // thread. This example debugger only exposes a single stack frame for the
    // single thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
    _session->registerHandler(
        [&](const dap::StackTraceRequest& request)
            -> dap::ResponseOrError<dap::StackTraceResponse>
            {
//                dap::writef(_log, "StackTraceRequest\n");

                if (request.threadId != threadId)
                {
                    return dap::Error("Unknown threadId '%d'", int(request.threadId));
                }

                dap::Source source;
                source.sourceReference = sourceReferenceId;
                source.name = "HelloDebuggerSource";

                dap::StackFrame frame;
                frame.line = _debugger->currentLine();
                frame.column = 1;
                frame.name = "HelloDebugger";
                frame.id = frameId;
                frame.source = source;

                dap::StackTraceResponse response;
                response.stackFrames.push_back(frame);
                return response;
            });

    // The Scopes request reports all the scopes of the given stack frame.
    // This example debugger only exposes a single 'Locals' scope for the single
    // frame.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
    _session->registerHandler(
        [&](const dap::ScopesRequest& request)
            -> dap::ResponseOrError<dap::ScopesResponse>
            {
//                dap::writef(_log, "ScopesRequest\n");

                if (request.frameId != frameId)
                {
                    return dap::Error("Unknown frameId '%d'", int(request.frameId));
                }

                dap::Scope scope;
                scope.name = "Locals";
                scope.presentationHint = "locals";
                scope.variablesReference = variablesReferenceId;

                dap::ScopesResponse response;
                response.scopes.push_back(scope);
                return response;
            });

    // The Variables request reports all the variables for the given scope.
    // This example debugger only exposes a single 'currentLine' variable for the
    // single 'Locals' scope.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
    _session->registerHandler(
        [&](const dap::VariablesRequest& request)-> dap::ResponseOrError<dap::VariablesResponse>
            {
//                dap::writef(_log, "VariablesRequest\n");

                if (request.variablesReference != variablesReferenceId)
                {
                    return dap::Error("Unknown variablesReference '%d'",
                        int(request.variablesReference));
                }

                dap::Variable currentLineVar;
                currentLineVar.name = "currentLine";
                currentLineVar.value = std::to_string(_debugger->currentLine());
                currentLineVar.type = "int";

                dap::VariablesResponse response;
                response.variables.push_back(currentLineVar);
                return response;
            });

    // The Pause request instructs the debugger to pause execution of one or all
    // threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
    _session->registerHandler(
        [&](const dap::PauseRequest&)
        {
            dap::writef(_log, "PauseRequest\n");

            _debugger->pause();
            return dap::PauseResponse();
        });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    _session->registerHandler(
        [&](const dap::ContinueRequest&)
        {
//            dap::writef(_log, "ContinueRequest\n");
            _debugger->run();
            return dap::ContinueResponse();
        });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    _session->registerHandler(
        [&](const dap::NextRequest&)
        {
//            dap::writef(_log, "NextRequest\n");
            _debugger->stepForward();
            return dap::NextResponse();
        });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    _session->registerHandler(
        [&](const dap::StepInRequest&)
        {
//            dap::writef(_log, "StepInRequest\n");
            // Step-in treated as step-over as there's only one stack frame.
            _debugger->stepForward();
            return dap::StepInResponse();
        });

    // The StepOut request instructs the debugger to step-out for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
    _session->registerHandler(
        [&](const dap::StepOutRequest&)
        {
//            dap::writef(_log, "StepOutRequest\n");

            // Step-out is not supported as there's only one stack frame.
            return dap::StepOutResponse();
        });

    // The SetBreakpoints request instructs the debugger to clear and set a number
    // of line breakpoints for a specific source file.
    // This example debugger only exposes a single source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetBreakpoints
    _session->registerHandler(
        [&](const dap::SetBreakpointsRequest& request)
        {
//            dap::writef(_log, "SetBreakpointsRequest\n");

            dap::SetBreakpointsResponse response;

            auto breakpoints = request.breakpoints.value({});
            if (request.source.sourceReference.value(0) == sourceReferenceId)
            {
                _debugger->clearBreakpoints();
                response.breakpoints.resize(breakpoints.size());
                for (size_t i = 0; i < breakpoints.size(); i++)
                {
                    _debugger->addBreakpoint(breakpoints[i].line);
                    response.breakpoints[i].verified = breakpoints[i].line < numSourceLines;
                }
            }
            else
            {
                response.breakpoints.resize(breakpoints.size());
            }

            return response;
        });

    // The SetExceptionBreakpoints request configures the debugger's handling of
    // thrown exceptions.
    // This example debugger does not use any exceptions, so this is a no-op.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetExceptionBreakpoints
    _session->registerHandler(
        [&](const dap::SetExceptionBreakpointsRequest&)
        {
//            dap::writef(_log, "SetExceptionBreakpointsRequest\n");

            return dap::SetExceptionBreakpointsResponse();
        });

    // The Source request retrieves the source code for a given source file.
    // This example debugger only exposes one synthetic source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    _session->registerHandler(
        [&](const dap::SourceRequest& request)
            -> dap::ResponseOrError<dap::SourceResponse>
            {
//                dap::writef(_log, "SourceRequest\n");

                if (request.sourceReference != sourceReferenceId)
                {
//                    dap::writef(_log, "Unknown source reference\n");
                    return dap::Error("Unknown source reference '%d'",
                        int(request.sourceReference));
                }

                dap::SourceResponse response;
                response.content = sourceContent;
                return response;
            });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    // This example debugger does nothing with this request.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    _session->registerHandler(
        [&](const dap::LaunchRequest&)
        {
//            dap::writef(_log, "LaunchRequest\n");

            return dap::LaunchResponse();
        });

    // Handler for disconnect requests
    _session->registerHandler(
        [&](const dap::DisconnectRequest& request)
        {
//            dap::writef(_log, "DisconnectRequest\n");

            if (request.terminateDebuggee.value(false))
            {
                _terminate.fire();
            }
            return dap::DisconnectResponse();
        });

    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    // This example debugger uses this request to 'start' the debugger.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    _session->registerHandler(
        [&](const dap::ConfigurationDoneRequest&)
        {
//            dap::writef(_log, "ConfigurationDoneRequest\n");

            _configured.fire();
            return dap::ConfigurationDoneResponse();
        });

    // All the handlers we care about have now been registered.
    // We now bind the session to stdin and stdout to connect to the client.
    // After the call to bind() we should start receiving requests, starting with
    // the Initialize request.
    _dapSocketFile = fopen("/tmp/openLibertyBasickDAP", "w+");

    std::shared_ptr<dap::Reader> in = dap::file(_dapSocketFile, false);
    std::shared_ptr<dap::Writer> out = dap::file(_dapSocketFile, false);
//    if (_log)
//    {
//        _session->bind(spy(in, _log), spy(out, _log));
//    }
//    else
//    {
        _session->bind(in, out);
//    }

}

void Controller::onBreakpointHit()
{
    // The debugger has been suspended. Inform the client.
    dap::StoppedEvent event;
    event.reason = "pause";
    event.threadId = threadId;
    _session->send(event);
}

void Controller::onStepped()
{
    // The debugger has single-line stepped. Inform the client.
    dap::StoppedEvent event;
    event.reason = "step";
    event.threadId = threadId;
    _session->send(event);
}

void Controller::onPaused()
{
    // The debugger has been suspended. Inform the client.
    dap::StoppedEvent event;
    event.reason = "pause";
    event.threadId = threadId;
    _session->send(event);
}

void Controller::onSessionError(const char *msg)
{
    if (_log)
    {
//        dap::writef(_log, "dap::Session error: %s\n", msg);
        _log->close();
    }
    _terminate.fire();
}

void Controller::waitConfigured()
{
    _configured.wait();
}

void Controller::threadStarted()
{
    // Broadcast the existance of the single thread to the client.
    dap::ThreadEvent threadStartedEvent;
    threadStartedEvent.reason = "started";
    threadStartedEvent.threadId = threadId;
    _session->send(threadStartedEvent);
}

void Controller::pause()
{
    _debugger->pause();
}

void Controller::waitTerminate()
{
    _terminate.wait();
}


/*

    // Start the debugger in a paused state.
    // This sends a stopped event to the client.
//    debugger.pause();

    // Block until we receive a 'terminateDebuggee' request or encounter a session
    // error.
    terminate.wait();

*/
