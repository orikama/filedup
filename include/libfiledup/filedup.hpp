#pragma once

#include <vector>

#include <boost/filesystem.hpp>


namespace fdup
{

struct DuplicateGroup
{
    boost::uintmax_t file_size;
    std::vector<boost::filesystem::path> files;

    DuplicateGroup(boost::uintmax_t file_size, const std::vector<boost::filesystem::path> &files)
        : file_size{file_size}, files{files}
    {}
};

std::vector<DuplicateGroup>
get_duplicate_files(const boost::filesystem::path &dir1, const boost::filesystem::path &dir2);

} // namespace fdup
