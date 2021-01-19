#pragma once

#include <vector>

#include <libfiledup/options.hpp>
#include <libfiledup/exception.hpp>


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

std::vector<DuplicateGroup> get_duplicate_files(Options &options);

} // namespace fdup
