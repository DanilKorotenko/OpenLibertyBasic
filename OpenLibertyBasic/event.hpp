//
//  event.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/21/24.
//

#ifndef event_hpp
#define event_hpp

#include <stdio.h>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <unordered_set>

// Event provides a basic wait and signal synchronization primitive.
class Event
{
public:
    // wait() blocks until the event is fired.
    void wait();

    // fire() sets signals the event, and unblocks any calls to wait().
    void fire();

private:
    std::mutex              _mutex;
    std::condition_variable _cv;
    bool                    _fired = false;
};


#endif /* event_hpp */
