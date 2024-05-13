//
//  event.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 4/21/24.
//

#include "event.hpp"

void Event::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [&]
        {
            return _fired;
        });
}

void Event::fire()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _fired = true;
    _cv.notify_all();
}
