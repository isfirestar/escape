#if !defined ESCAPE_FILEMODE_H
#define ESCAPE_FILEMODE_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <atomic>
#include <cstdint>

#if _WIN32
#include <Windows.h>
#endif

class file_mode {
#if _WIN32
    HANDLE fd = (HANDLE) (~0);
#else
    int fd = -1;
#endif
    uint64_t previous_offset = 0;
    std::atomic<uint64_t> offset{ 0};
    uint64_t file_size = 0;
    uint32_t block_size = 1024;
    std::string file_path;

public:
    file_mode(const std::string &path, uint32_t block_size);
    ~file_mode();

    const std::string getfilenam() const;
    const uint64_t get_previous_offset() const;
    const uint32_t get_blocksize() const;

    int open_it();
    int creat_it();
    int cover_it();

    int read_block(char *block);
    int write_block(const char *block, int woff, int cb);
};

#endif // !ESCAPE_FILEMODE_H