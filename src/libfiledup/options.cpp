#include <libfiledup/options.hpp>

#include <libfiledup/exception.hpp>

#include <boost/filesystem/operations.hpp>


namespace fdup
{

void
Options::validate_options()
{
    namespace fs = boost::filesystem;

    if (fs::is_directory(dir1) == false) {
        throw fdup::options_error("DIR1 is not a directory");
    }
    if (fs::is_directory(dir2) == false) {
        throw fdup::options_error("DIR2 is not a directory");
    }

    const auto dir1_canonical = fs::canonical(dir1).string();
    const auto dir2_canonical = fs::canonical(dir2).string();

    if (dir1_canonical == dir2_canonical) {
        throw fdup::options_error("DIR1 and DIR2 must not point to the same directory");
    }

    if (search_recursively) {
        const auto min_length = std::min(dir1_canonical.size(), dir2_canonical.size());
        if (dir1_canonical.compare(0, min_length, dir2_canonical, 0, min_length) == 0) {
            throw fdup::options_error("One DIR cannot be subfolder of the other, if 'recursively' option is specified");
        }
    }
}

} // namespace fdup
