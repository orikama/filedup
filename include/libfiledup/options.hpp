#pragma once

#include <boost/filesystem/path.hpp>


namespace fdup
{

struct Options
{
    boost::filesystem::path dir1;
    boost::filesystem::path dir2;
    bool search_recursively;
    // NOTE: Или static, или сделать конструктор и в нем вызывать этот метод,
    //  но если будет много опций будет здоровый конструктор...
    void validate_options();
};

} // namespace fdup
