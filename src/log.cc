/***

Copyright 2013-2016 Dave Cridland
Copyright 2014-2016 Surevine Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

***/

#include "log.h"
#include "config.h"
#include <fstream>
#include <iostream>

void Metre::Log::log(Log::LEVEL lvlm, std::string const &filename, int line, std::string const &stuff) {
    spdlog::level::level_enum lvl = spdlog::level::trace;
    switch (lvlm) {
        case EMERG:
        case ALERT:
        case CRIT:
            lvl = spdlog::level::critical;
            break;
        case ERR:
            lvl = spdlog::level::err;
            break;
        case WARNING:
        case NOTICE:
            lvl = spdlog::level::warn;
            break;
        case INFO:
            lvl = spdlog::level::info;
            break;
        case DEBUG:
            lvl = spdlog::level::debug;
            break;
        case TRACE:
            lvl = spdlog::level::trace;
            break;
        default:
            lvl = spdlog::level::critical;
    }
    Metre::Config::config().logger().log(lvl, "{}:{} : {}", filename, line, stuff);
}
