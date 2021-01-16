#include <iostream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include <libfiledup/filedup.hpp>


boost::optional<boost::filesystem::path> parse_options(int argc, const char *argv[])
{
    namespace po = boost::program_options;

    po::options_description options_desc{"Options"};
    options_desc.add_options()
        ("help", "produce this help message")
        ("dir", po::value<boost::filesystem::path>(), "path to the directory")
    ;

    po::positional_options_description positional_options_desc{};
    positional_options_desc.add("dir", -1);

    po::variables_map vm{};
    po::store(po::command_line_parser(argc, argv)
              .positional(positional_options_desc).options(options_desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "\nUsage: filedup_console DIR\n\n"
            << options_desc << std::endl;
        return boost::none;
    }
    if (vm.count("dir") == 0) {
        std::cout << "You must specify a path to the directory" << std::endl;
        return boost::none;
    }

    return vm["dir"].as<boost::filesystem::path>();
}

int main(int argc, const char *argv[])
{
    try {
        if (auto path = parse_options(argc, argv)) {
            for (const auto &file_info: fdup::get_files(path.value())) {
                std::cout << "Path: " << file_info.path << "\n\tSize: " << file_info.size << std::endl;
            }
        }
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cout << "ERROR at boost::filesystem: " << e.code().message() << std::endl;
    }

    return 0;
}
