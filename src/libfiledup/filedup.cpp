#include <libfiledup/filedup.hpp>


namespace fdup
{

file_info::file_info(boost::filesystem::path path, std::size_t size)
    : path(path)
    , size(size)
{}

std::vector<file_info> fdup::get_files(boost::filesystem::path path)
{
    std::vector<file_info> file_infos;

    for (const auto &entry: boost::filesystem::directory_iterator(path)) {
        if (boost::filesystem::is_regular_file(entry)) {
            const auto &path = entry.path();
            file_infos.emplace_back(path, boost::filesystem::file_size(path));
        }
    }

    return file_infos;
}

} // namespace fdup
