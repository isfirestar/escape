#include "filemode.h"
#include "os_util.hpp"
#include "icom/posix_ifos.h"
#include <errno.h>

#if _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

file_mode::file_mode(const std::string& path, uint32_t block_size) {
    file_path = path;
    this->block_size = block_size;
}

file_mode::~file_mode() {
    if (fd > 0) {
        ::posix__close((int) fd);
    }
}

int file_mode::open_it() {
#if _WIN32
    fd = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == fd) {
        return GetLastError();
    }
#else
    fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        return errno;
    }
#endif
    file_size = nsp::os::get_filesize(file_path);
    if (INVAILD_FILESIZE == file_size) {
        posix__close((int) fd);
        return -1;
    }
    return 0;
}

int file_mode::creat_it() {
#if _WIN32
    fd = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == fd) {
        return GetLastError();
    }
#else
    fd = open(file_path.c_str(), O_EXCL | O_RDWR | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return errno;
    }
#endif
    return 0;

}

int file_mode::cover_it() {
#if _WIN32
    fd = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == fd) {
        return GetLastError();
    }
#else
    fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return errno;
    }
#endif
    return 0;
}

int file_mode::read_block(char *block) {
    // 因为下层执行同步读取， 因此不允许读取偏移超文件大小
    if (offset >= this->file_size) {
        return -1;
    }

    int rdcb;
    int rdcb_r;

    if (offset + block_size >= file_size) {
        rdcb_r = (int)(file_size - offset);
    } else {
        rdcb_r = block_size;
    }
#if _WIN32
    LARGE_INTEGER move, pointer;
    move.QuadPart = offset;
    SetFilePointerEx((HANDLE) fd, move, &pointer, FILE_BEGIN);
#else
    lseek(fd, (__off_t) offset, SEEK_SET);
#endif
    rdcb = ::posix__read_file((int) fd, block, rdcb_r);
    previous_offset = offset;
    offset += rdcb;
    return rdcb;
}

int file_mode::write_block(const char *block, uint64_t woff, int cb) {
    int wrcb = 0;
#if _WIN32
    LARGE_INTEGER move, pointer;
    move.QuadPart = woff;
    SetFilePointerEx((HANDLE) fd, move, &pointer, FILE_BEGIN);
#else
    lseek(fd, (__off_t) woff, SEEK_SET);
#endif
    wrcb = ::posix__write_file((int) fd, block, cb);
    offset += wrcb;
    return wrcb;
}

const std::string file_mode::getfilenam() const {
    auto of = file_path.find_last_of('/', 0);
    if (of == std::string::npos) {
        of = file_path.find_last_of('\\', 0);
    }
    if (of == std::string::npos) {
        return "";
    }
    return std::string(&file_path[of]);
}

const uint64_t file_mode::get_previous_offset() const {
    return previous_offset;
}

const uint32_t file_mode::get_blocksize() const {
    return this->block_size;
}