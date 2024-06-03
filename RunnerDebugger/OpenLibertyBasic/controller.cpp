//
//  controller.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 5/13/24.
//

#include "controller.hpp"

#include <sstream>

#include <dap/typeof.h>

const dap::integer threadId = 100;
const dap::integer frameId = 200;
const dap::integer variablesReferenceId = 300;
const dap::integer sourceReferenceId = 400;

namespace
{

    // sourceContent holds the synthetic file source.
    constexpr char sourceContent[] = R"(// Hello Debugger!

test test test

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
{
}

Controller::~Controller()
{
}


void Controller::init()
{
    _session->onError(std::bind(&Controller::onSessionError, this, std::placeholders::_1));

    _session->registerHandler([&](const dap::InitializeRequest &request)
        { return initializeRequest(request); });

    // When the Initialize response has been sent, we need to send the initialized
    // event.
    // We use the registerSentHandler() to ensure the event is sent *after* the
    // initialize response.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Initialized
    _session->registerSentHandler(
        [&](const dap::ResponseOrError<dap::InitializeResponse>&)
        {
            _session->send(dap::InitializedEvent());
        });

    _session->registerHandler([&](const dap::LBLaunchRequest &request)
        { return launchRequest(request); });

    // The Threads request queries the debugger's list of active threads.
    // This example debugger only exposes a single thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
    _session->registerHandler(
        [&](const dap::ThreadsRequest&)
        {
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
            _debugger->pause();
            return dap::PauseResponse();
        });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    _session->registerHandler(
        [&](const dap::ContinueRequest&)
        {
            _debugger->run();
            return dap::ContinueResponse();
        });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    _session->registerHandler(
        [&](const dap::NextRequest&)
        {
            _debugger->stepForward();
            return dap::NextResponse();
        });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    _session->registerHandler(
        [&](const dap::StepInRequest&)
        {
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
            output("set breakpoint");

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
            return dap::SetExceptionBreakpointsResponse();
        });

    // The Source request retrieves the source code for a given source file.
    // This example debugger only exposes one synthetic source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    _session->registerHandler(
        [&](const dap::SourceRequest& request)
            -> dap::ResponseOrError<dap::SourceResponse>
            {
                if (request.sourceReference != sourceReferenceId)
                {
                    return dap::Error("Unknown source reference '%d'",
                        int(request.sourceReference));
                }

                dap::SourceResponse response;
                response.content = sourceContent;
                return response;
            });

    _session->registerHandler(
        [&](const dap::EvaluateRequest &request)
        {
            output("Evaluate request: %s\n", request.expression.c_str());
            return dap::EvaluateResponse();
        });

    // Handler for disconnect requests
    _session->registerHandler(
        [&](const dap::DisconnectRequest& request)
        {
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
            _configured.fire();
            return dap::ConfigurationDoneResponse();
        });


    // All the handlers we care about have now been registered.
    // We now bind the session to stdin and stdout to connect to the client.
    // After the call to bind() we should start receiving requests, starting with
    // the Initialize request.

    std::shared_ptr<dap::Reader> in = dap::file(stdin, false);
    std::shared_ptr<dap::Writer> out = dap::file(stdout, false);
    _session->bind(in, out);
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
    output("stepped");
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
    _terminate.fire();
}

void Controller::output(const std::string msg, ...)
{
    va_list                 args;
    std::unique_ptr<char[]> formattedOutput;

    int size         = 0;
    int requiredSize = static_cast<int>(msg.size());

    do
    {
        size += abs(requiredSize - size + 1);
        formattedOutput.reset(new char[size]);

        strcpy(formattedOutput.get(), msg.c_str());
        va_start(args, msg);
        requiredSize = vsnprintf(&formattedOutput[0], static_cast<size_t>(size), msg.c_str(), args);
        va_end(args);

    }
    while (requiredSize < 0 || requiredSize >= size);

    dap::OutputEvent outputEvent;
    outputEvent.output = std::string(formattedOutput.get());
    _session->send(outputEvent);
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

// The Initialize request is the first message sent from the client and
// the response reports debugger capabilities.
// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
dap::InitializeResponse Controller::initializeRequest(const dap::InitializeRequest&)
{
    dap::InitializeResponse response;
    response.supportsConfigurationDoneRequest = true;
    return response;
}

dap::LaunchResponse Controller::launchRequest(const dap::LBLaunchRequest &request)
{
    output("Start debugging\n");
    output("Program: %s", request.program.c_str());

    return dap::LaunchResponse();
}
