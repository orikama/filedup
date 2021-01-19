#include <libfiledup/detail/file_hash.hpp>

#include <fstream>


namespace fs = boost::filesystem;

constexpr int kHashChunkSize = 4096;


namespace fdup {
namespace detail {

FileHash::FileHash(const hash_t &hash)
{
    std::memcpy(m_hash, hash, sizeof(hash_t));
}

bool
FileHash::operator==(const FileHash &other) const noexcept
{
    return std::memcmp(m_hash, other.m_hash, sizeof(hash_t)) == 0;
}

std::vector<FileHash>
FileHash::get_partial_hashes(const std::vector<fs::path> &files)
{
    return get_hashes(kHashChunkSize, files);
}

std::vector<FileHash>
FileHash::get_full_hashes(const boost::uintmax_t bytes_to_read, const std::vector<fs::path> &files)
{
    return get_hashes(bytes_to_read, files);
}

std::vector<FileHash>
FileHash::get_hashes(const boost::uintmax_t bytes_to_read, const std::vector<fs::path> &files)
{
    std::vector<FileHash> hashes{};
    hashes.reserve(files.size());

    char buffer[kHashChunkSize];
    for (const auto &file: files) {
        boost::uuids::detail::md5 hash{};
        boost::uuids::detail::md5::digest_type digest{};
        std::ifstream file_stream{file.c_str(), std::ifstream::binary};

        auto bytes_left = bytes_to_read;

        while (bytes_left > 0) {
            const auto next_read = bytes_left > kHashChunkSize ? kHashChunkSize : bytes_left;
            file_stream.read(buffer, next_read);
            hash.process_bytes(buffer, next_read);
            bytes_left -= next_read;
        }

        hash.get_digest(digest);
        hashes.push_back(digest);
    }

    return hashes;
}

} // namespace detail
} // namespace fdup
