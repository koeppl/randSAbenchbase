#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>

#include <utility>
#include <typeinfo>

//@ call munmap after usage!
//@ see https://stackoverflow.com/questions/17925051/fast-textfile-reading-in-c
template<typename T>
inline static std::pair<T*,size_t> map_file(const char* filename, const int prot = PROT_READ, const int flags = MAP_PRIVATE) {
    const int fd = open(filename, O_RDONLY);
    if(fd == -1) { error(1, 0, "open failed for %s", filename); }

    // obtain file size
    struct stat filestat;
    if(fstat(fd, &filestat) == -1) { error(1, 0, "fstat failed for %s", filename); }

    if(filestat.st_size % sizeof(T) != 0) { error(1, 0, "contents of file %s are a fraction of type %s", filename, typeid(T).name()); }

    const size_t length = filestat.st_size;

    T* addr = static_cast<T*>(mmap(nullptr, length, prot, flags, fd, 0u));
    if(addr == MAP_FAILED) { error(1, 0, "mmap failed for %s ", filename); }

    return std::make_pair(addr, length/sizeof(T));
}

