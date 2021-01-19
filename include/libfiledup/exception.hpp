#pragma once

#include <stdexcept>


namespace fdup
{

class options_error : public std::runtime_error
{
public:
    options_error(const char * message)
        : std::runtime_error(message)
    {}
};

} // namespace fdup
