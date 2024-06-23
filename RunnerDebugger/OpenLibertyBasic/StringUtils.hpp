//
//  StringUtils.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/23/24.
//

#ifndef StringUtils_hpp
#define StringUtils_hpp

#include <stdio.h>
#include <string>

class StringUtils
{
public:
    // trim from start (in place)
    static void ltrim(std::string &s);

    // trim from end (in place)
    static void rtrim(std::string &s);

    static void trim(std::string &s);

};

#endif /* StringUtils_hpp */
