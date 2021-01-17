#pragma once

#include <vector>

#include <boost/filesystem.hpp>


namespace fdup
{

std::vector<std::vector<boost::filesystem::path>>
get_duplicate_files(const boost::filesystem::path &dir1, const boost::filesystem::path &dir2);

} // namespace fdup
