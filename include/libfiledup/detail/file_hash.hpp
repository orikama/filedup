#pragma once

#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/uuid/detail/md5.hpp>


namespace fdup {
namespace detail {

class FileHash
{
    using hash_t = boost::uuids::detail::md5::digest_type;

public:
    bool operator==(const FileHash &other) const noexcept;

    static std::vector<FileHash> get_partial_hashes(const std::vector<boost::filesystem::path> &files);
    static std::vector<FileHash> get_full_hashes(const boost::uintmax_t bytes_to_read,
                                                 const std::vector<boost::filesystem::path> &files);

private:
    FileHash(const hash_t &hash);
    static std::vector<FileHash> get_hashes(const boost::uintmax_t bytes_to_read,
                                            const std::vector<boost::filesystem::path> &files);

private:
    hash_t m_hash;
};

} // namespace detail
} // namespace fdup
