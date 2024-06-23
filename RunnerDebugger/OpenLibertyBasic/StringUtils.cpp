//
//  StringUtils.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/23/24.
//

#include "StringUtils.hpp"
#include <algorithm> 
#include <cctype>
#include <locale>


void StringUtils::ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }));
}

void StringUtils::rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

void StringUtils::trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}
