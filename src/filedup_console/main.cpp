#include <iostream>
#include <utility>

#include <boost/filesystem/exception.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <libfiledup/filedup.hpp>
#include <libfiledup/exception.hpp>


boost::optional<fdup::Options>
parse_options(int argc, const char *argv[])
{
    namespace po = boost::program_options;

    po::options_description options_desc{"Options"};
    options_desc.add_options()
        ("help", "display this help message")
        ("dirs", po::value<std::vector<boost::filesystem::path>>(), "paths to the two directories to compare")
        ("recursively,r", po::bool_switch()->default_value(false), "search for duplicate files in subfolders recursively")
    ;

    po::positional_options_description positional_options_desc{};
    positional_options_desc.add("dirs", -1);

    po::variables_map vm{};
    po::store(po::command_line_parser(argc, argv).positional(positional_options_desc).options(options_desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "\nUsage: filedup_console DIR1 DIR2 [options]\n\n"
            << options_desc << std::endl;
        return boost::none;
    }
    if (vm.count("dirs") == 0) {
        std::cout << "You must specify a paths to the two directories" << std::endl;
        return boost::none;
    }

    const auto dirs = vm["dirs"].as<std::vector<boost::filesystem::path>>();
    // TODO: Check that there are actually two of them?

    fdup::Options options;
    options.dir1 = dirs[0];
    options.dir2 = dirs[1];
    options.search_recursively = vm["recursively"].as<bool>();

    return options;
}

int main(int argc, const char *argv[])
{
    try {
        if (auto options_optional = parse_options(argc, argv)) {
            auto &options = options_optional.value();

            for (const auto &duplicate_group: fdup::get_duplicate_files(options)) {
                std::cout << duplicate_group.files.size() << " files, " << duplicate_group.file_size << " bytes each:\n";
                for (const auto &duplicate_file: duplicate_group.files) {
                    std::cout << '\t' << duplicate_file << '\n';
                }
            }
        }
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cout << "ERROR at boost::filesystem: " << e.code().message() << std::endl;
    }
    catch (const fdup::options_error &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}
