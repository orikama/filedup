#include <libfiledup/filedup.hpp>

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>


using FilesGroupedBySize = std::unordered_map<boost::uintmax_t, std::vector<boost::filesystem::path>>;


FilesGroupedBySize
get_files_grouped_by_size(const boost::filesystem::path &directory)
{
    FilesGroupedBySize file_groups{};

    for (const auto &entry: boost::filesystem::directory_iterator(directory)) {
        if (boost::filesystem::is_regular_file(entry)) {
            const auto &file_path = entry.path();
            const auto file_size = boost::filesystem::file_size(file_path);

            const auto size_group_it = file_groups.find(file_size);
            if (size_group_it == file_groups.end()) {
                file_groups.emplace(file_size, std::vector<boost::filesystem::path>{file_path});
            }
            else {
                size_group_it->second.push_back(file_path);
            }
        }
    }

    return file_groups;
}

// NOTE: returns size groups with at least 2 files in a group
std::vector<std::vector<boost::filesystem::path>>
merge_size_groups(const FilesGroupedBySize &dir1_size_groups, const FilesGroupedBySize &dir2_size_groups)
{
    std::vector<std::vector<boost::filesystem::path>> merged_groups{};

    for (const auto &dir1_group: dir1_size_groups) {
        const auto dir2_group_it = dir2_size_groups.find(dir1_group.first);

        if (dir2_group_it != dir2_size_groups.end()) {
            const auto &dir2_group = dir2_group_it->second;

            std::vector<boost::filesystem::path> merged_group{dir1_group.second}; // NOTE: move ?
            merged_group.insert(merged_group.end(), dir2_group.begin(), dir2_group.end());

            merged_groups.push_back(std::move(merged_group));
        }
    }

    return merged_groups;
}

bool
is_files_binary_equal(const boost::filesystem::path &file1, const boost::filesystem::path &file2)
{
    std::ifstream file1_stream{file1.c_str(), std::ifstream::binary};
    std::ifstream file2_stream{file2.c_str(), std::ifstream::binary};

    std::istreambuf_iterator<char> file1_stream_begin{file1_stream};
    std::istreambuf_iterator<char> file2_stream_begin{file2_stream};
    std::istreambuf_iterator<char> file_stream_end{};

    return std::equal(file1_stream_begin, file_stream_end,
                      file2_stream_begin, file_stream_end);
}

std::vector<std::vector<boost::filesystem::path>>
find_duplicates(const std::vector<std::vector<boost::filesystem::path>> &file_groups)
{
    std::vector<std::vector<boost::filesystem::path>> duplicates{};

    for (const auto &size_group: file_groups) {
        const auto size_group_size = size_group.size(); // NOTE: nice name
        std::unordered_set<std::size_t> indices_to_skip{};

        for (std::size_t i = 0; i != size_group_size - 1; ++i) {
            if (indices_to_skip.find(i) != indices_to_skip.end()) {
                continue;
            }

            std::vector<boost::filesystem::path> duplicate_group{};
            const auto &file_for_comparison = size_group[i];

            for(std::size_t j = i + 1; j != size_group_size; ++j) {
                if (indices_to_skip.find(j) == indices_to_skip.end()) {
                    const auto &file_to_compare = size_group[j];
                    if (is_files_binary_equal(file_for_comparison, file_to_compare)) {
                        duplicate_group.push_back(file_to_compare);
                        indices_to_skip.insert(j);
                    }
                }
            }

            if (duplicate_group.size() > 0) {
                duplicate_group.push_back(file_for_comparison);
                duplicates.push_back(std::move(duplicate_group));
            }
        }

        // NOTE: Too complicated, dropped for now
        // for(auto end_it = std::prev(size_entry.end(), 1); end_it != size_entry.begin(); ) {
        //     const auto &file_to_compare = *end_it;
        //     std::vector<boost::filesystem::path> duplicate_group;
        //     auto new_end_it = std::remove_if(size_entry.begin(), end_it,
        //         [file_to_compare, &duplicate_group](auto file){
        //             bool equal = is_files_binary_equal(file_to_compare, file);
        //             if (equal) {
        //                 duplicate_group.push_back(file);
        //             }
        //             return equal;
        //         }
        //     );
        //     if (duplicate_group.size() > 0) {
        //         duplicate_group.push_back(file_to_compare);
        //         duplicates.push_back(std::move(duplicate_group));
        //     }
        // }
    }

    return duplicates;
}


namespace fdup
{

std::vector<std::vector<boost::filesystem::path>>
get_duplicate_files(const boost::filesystem::path &dir1, const boost::filesystem::path &dir2)
{
    // TODO: Check that dir1 != dir2
    // TODO: Probably move options validation to its own function (when I made them)
    if (boost::filesystem::is_directory(dir1) == false || boost::filesystem::is_directory(dir2) == false) {
        throw std::exception("dir1 or dir2 is not a directory");
    }

    const auto possible_duplicates = merge_size_groups(get_files_grouped_by_size(dir1),
                                                       get_files_grouped_by_size(dir2));

    return find_duplicates(possible_duplicates);
}

} // namespace fdup
