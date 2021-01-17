#include <iostream>
#include <utility>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <libfiledup/filedup.hpp>


boost::optional<std::pair<boost::filesystem::path, boost::filesystem::path>>
parse_options(int argc, const char *argv[])
{
    namespace po = boost::program_options;

    po::options_description options_desc{"Options"};
    options_desc.add_options()
        ("help", "produce this help message")
        ("dirs", po::value<std::vector<boost::filesystem::path>>(), "paths to the two directories to compare")
    ;

    po::positional_options_description positional_options_desc{};
    positional_options_desc.add("dirs", -1);

    po::variables_map vm{};
    po::store(po::command_line_parser(argc, argv).positional(positional_options_desc).options(options_desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "\nUsage: filedup_console DIR1 DIR2\n\n"
            << options_desc << std::endl;
        return boost::none;
    }
    if (vm.count("dirs") == 0) {
        std::cout << "You must specify a paths to the two directories" << std::endl;
        return boost::none;
    }

    const auto dirs = vm["dirs"].as<std::vector<boost::filesystem::path>>();
    // TODO: Check that there are actually two of them?

    return std::make_pair(dirs[0], dirs[1]);
}

int main(int argc, const char *argv[])
{
    try {
        if (const auto two_dirs_optional = parse_options(argc, argv)) {
            const auto &two_dirs = two_dirs_optional.value();

            for (auto &duplicate_group: fdup::get_duplicate_files(two_dirs.first, two_dirs.second)) {
                for(auto &duplicate_file: duplicate_group) {
                    std::cout << '\t' << duplicate_file.make_preferred() << '\n';
                }
                std::cout << '\n';
            }
        }
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cout << "ERROR at boost::filesystem: " << e.code().message() << std::endl;
    }

    return 0;
}
