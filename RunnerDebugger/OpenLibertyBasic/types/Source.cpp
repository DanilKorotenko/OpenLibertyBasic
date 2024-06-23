//
//  Source.cpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/23/24.
//

#include "Source.hpp"
#include <fstream>
Source::Source(const std::string &aSourcePath)
    : _path(aSourcePath)
    , _content()
{
    std::ifstream sourceFile(aSourcePath, std::ios::in);
    if (sourceFile.is_open())
    {
        _content = std::string(std::istreambuf_iterator<char>(sourceFile), std::istreambuf_iterator<char>());
        sourceFile.close();
    }
}
