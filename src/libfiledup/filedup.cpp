#include <libfiledup/filedup.hpp>
#include <libfiledup/detail/file_hash.hpp>

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/type_erased.hpp>


namespace fs = boost::filesystem;
using FilesGroupedBySize = std::unordered_map<boost::uintmax_t, std::vector<fs::path>>;


// https://stackoverflow.com/a/60512261
auto make_directory_range(const fs::path &directory, bool search_recursively)
{
    return search_recursively
        ? boost::make_iterator_range(fs::recursive_directory_iterator(directory), fs::recursive_directory_iterator()) | boost::adaptors::type_erased<>()
        : boost::make_iterator_range(fs::directory_iterator(directory), fs::directory_iterator());
}

FilesGroupedBySize
get_files_grouped_by_size(const fs::path &directory, bool search_recursively)
{
    FilesGroupedBySize file_groups{};

    for (const auto &entry: make_directory_range(directory, search_recursively)) {
        if (fs::is_regular_file(entry)) {
            const auto &file_path = entry.path();
            const auto file_size = fs::file_size(file_path);

            const auto size_group_it = file_groups.find(file_size);
            if (size_group_it == file_groups.end()) {
                file_groups.emplace(file_size, std::vector<fs::path>{file_path});
            }
            else {
                size_group_it->second.push_back(file_path);
            }
        }
    }

    return file_groups;
}

// NOTE: returns size groups with at least 2 files in a group
std::vector<fdup::DuplicateGroup>
merge_size_groups(FilesGroupedBySize &&dir1_size_groups, FilesGroupedBySize &&dir2_size_groups)
{
    std::vector<fdup::DuplicateGroup> merged_groups{};

    for (auto &dir1_group: dir1_size_groups) {
        const auto dir2_group_it = dir2_size_groups.find(dir1_group.first);

        if (dir2_group_it != dir2_size_groups.end()) {
            auto &dir2_group = dir2_group_it->second;

            std::vector<fs::path> merged_group{std::move(dir1_group.second)};
            merged_group.insert(merged_group.end(),
                                std::make_move_iterator(dir2_group.begin()), std::make_move_iterator(dir2_group.end()));

            merged_groups.emplace_back(dir1_group.first, std::move(merged_group));
        }
    }

    return merged_groups;
}

bool
are_files_binary_equal(const boost::uintmax_t files_size, const fs::path &file1, const fs::path &file2)
{
    // NOTE: boost mapped_file?
    constexpr auto CHUNK_SIZE = 32 * 512;
    char buffer1[CHUNK_SIZE];
    char buffer2[CHUNK_SIZE];

    std::ifstream file1_stream{file1.c_str(), std::ifstream::binary};
    std::ifstream file2_stream{file2.c_str(), std::ifstream::binary};

    auto bytes_left = files_size;
    while (bytes_left > 0) {
        const auto next_read = bytes_left > CHUNK_SIZE ? CHUNK_SIZE : bytes_left;
        file1_stream.read(buffer1, next_read);
        file2_stream.read(buffer2, next_read);
        if (std::memcmp(buffer1, buffer2, next_read) != 0) {
            break;
        }
        bytes_left -= next_read;
    }

    return bytes_left == 0;
}

std::vector<fdup::DuplicateGroup>
split_by_hash(std::vector<fdup::DuplicateGroup> &&duplicate_groups, bool calculate_partial_hash)
{
    std::vector<fdup::DuplicateGroup> new_duplicate_groups{};

    for (auto &duplicate_group: duplicate_groups) {
        const auto duplicate_group_size = duplicate_group.files.size();
        // No need to mess with hashes if there are only two files in a group
        if (duplicate_group_size == 2) {
            new_duplicate_groups.push_back(std::move(duplicate_group));
            continue;
        }

        const auto hashes = calculate_partial_hash
            ? fdup::detail::FileHash::get_partial_hashes(duplicate_group.files)
            : fdup::detail::FileHash::get_full_hashes(duplicate_group.file_size, duplicate_group.files);

        std::unordered_set<std::size_t> indices_to_skip{};

        for (std::size_t i = 0; i != duplicate_group_size - 1; ++i) {
            if (indices_to_skip.find(i) != indices_to_skip.end()) {
                continue;
            }

            std::vector<fs::path> duplicate_files{}; // NOTE: not actually duplicates, just duplicate hashes
            const auto &hash_for_comparison = hashes[i];

            for (std::size_t j = i + 1; j != duplicate_group_size; ++j) {
                if (indices_to_skip.find(j) == indices_to_skip.end()) {
                    const auto &hash_to_compare = hashes[j];

                    if (hash_for_comparison == hash_to_compare) {
                            duplicate_files.push_back(std::move(duplicate_group.files[j]));
                        indices_to_skip.insert(j);
                    }
                }
            }

            if (duplicate_files.empty() == false) {
                    duplicate_files.push_back(std::move(duplicate_group.files[i]));
                    new_duplicate_groups.emplace_back(duplicate_group.file_size, std::move(duplicate_files));
            }
        }
    }

    return new_duplicate_groups;
}

std::vector<fdup::DuplicateGroup>
find_duplicates(const std::vector<fdup::DuplicateGroup> &&duplicate_groups)
{
    std::vector<fdup::DuplicateGroup> new_duplicate_groups{};

    for (auto &duplicate_group: duplicate_groups) {
        const auto files_in_group = duplicate_group.files.size();
        std::unordered_set<std::size_t> indices_to_skip{};

        // NOTE: Я так и не придумал как объединить этот кусок с почти таким же как и в split_by_hash()
        // Оба этих алгоритма убирают, уникальные элемпенты и группируют дубликаты
        for (std::size_t i = 0; i != files_in_group - 1; ++i) {
            if (indices_to_skip.find(i) != indices_to_skip.end()) {
                continue;
            }

            std::vector<fs::path> duplicate_files{};
            auto &file_for_comparison = duplicate_group.files[i];

            for (std::size_t j = i + 1; j != files_in_group; ++j) {
                if (indices_to_skip.find(j) == indices_to_skip.end()) {
                    auto &file_to_compare = duplicate_group.files[j];

                    if (are_files_binary_equal(duplicate_group.file_size, file_for_comparison, file_to_compare)) {
                        duplicate_files.push_back(file_to_compare);
                        indices_to_skip.insert(j);
                    }
                }
            }

            if (duplicate_files.empty() == false) {
                duplicate_files.push_back(file_for_comparison);
                new_duplicate_groups.emplace_back(duplicate_group.file_size, std::move(duplicate_files));
            }
        }
    }

    return new_duplicate_groups;
}

void
validate_options(const fdup::Options &options)
{
    if (fs::is_directory(options.dir1) == false) {
        throw std::exception("DIR1 is not a directory");
    }
    if (fs::is_directory(options.dir2) == false) {
        throw std::exception("DIR2 is not a directory");
    }

    const auto dir1_canonical = fs::canonical(options.dir1).string();
    const auto dir2_canonical = fs::canonical(options.dir2).string();

    if (dir1_canonical == dir2_canonical) {
        throw std::exception("DIR1 and DIR2 must not point to the same directory");
    }

    if (options.search_recursively) {
        const auto min_length = std::min(dir1_canonical.size(), dir2_canonical.size());
        if (dir1_canonical.compare(0, min_length, dir2_canonical, 0, min_length) == 0) {
            throw std::exception("One DIR cannot be subfolder of the other, if 'recursively' option is specified");
        }
    }
}

namespace fdup
{

std::vector<DuplicateGroup>
get_duplicate_files(Options &options)
{
    validate_options(options);

    options.dir1.make_preferred();
    options.dir2.make_preferred();

    auto dir1_files = get_files_grouped_by_size(options.dir1, options.search_recursively);
    auto dir2_files = get_files_grouped_by_size(options.dir2, options.search_recursively);

    auto possible_duplicates = merge_size_groups(std::move(dir1_files), std::move(dir2_files));
    auto duplicates_grouped_by_partial_hash = split_by_hash(std::move(possible_duplicates), true);
    auto duplicates_grouped_by_full_hash = split_by_hash(std::move(duplicates_grouped_by_partial_hash), false);

    return find_duplicates(std::move(duplicates_grouped_by_full_hash));
}

} // namespace fdup
