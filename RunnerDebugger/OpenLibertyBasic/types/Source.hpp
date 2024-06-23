//
//  Source.hpp
//  OpenLibertyBasic
//
//  Created by Danil Korotenko on 6/23/24.
//

#ifndef Source_hpp
#define Source_hpp

#include <stdio.h>
#include <dap/protocol.h>
#include <memory>
#include <filesystem>

class Source
{
public:
    using PtrT = std::shared_ptr<Source>;

    Source(const std::string &aSourcePath);

    dap::Source getDapSource();

    int referenceId()       { return 1;}
    std::string content()   { return _content; }

private:
    std::filesystem::path       _path;
    std::string                 _content;

};

#endif /* Source_hpp */
