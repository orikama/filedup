#include <iostream>
#include <libfiledup/filedup.hpp>


int main()
{
    try {
        for (const auto &file_info: fdup::get_files("./")) {
            std::cout << "Path: " << file_info.path << "\n\tSize: " << file_info.size << std::endl;
        }
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cout << "ERROR at boost::filesystem: " << e.code().message() << std::endl;
    }

    return 0;
}
