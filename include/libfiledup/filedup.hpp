#pragma once

#include <vector>
#include <boost/filesystem.hpp>


namespace fdup
{

struct file_info
{
    boost::filesystem::path path;
    boost::uintmax_t        size;

    file_info(boost::filesystem::path path, boost::uintmax_t size);
};

std::vector<file_info> get_files(boost::filesystem::path path);

} // namespace fdup
