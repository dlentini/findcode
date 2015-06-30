/* File -- */

#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

struct Entry {
  Entry() : name(nullptr), dir(false) {}
  Entry(const char *name, bool dir) : name(name), dir(dir) {}
  const char *name;
  bool dir;
};

struct File {
  File(int fd, const char *path) {
    entry.name = path;
    dfd = fd < 0 ? open(path, O_RDONLY) : openat(fd, path, O_RDONLY);
    dir = fdopendir(dfd);
  }
  ~File() {
    // Leak!!!
    // if (dir != nullptr) closedir(dir);
    // if (dfd != -1) close(dfd);
  }

  template <typename Filter> Entry *next_entry(Filter filter) {
    struct dirent *dirp;
    struct stat sb;
    while (dir && (dirp = readdir(dir)) != NULL) {
      entry.name = dirp->d_name;
      if (std::string("..") == entry.name || std::string(".") == entry.name) {
        continue;
      }
      if (fstatat(dfd, entry.name, &sb, 0) == -1) {
        continue;
      }
      entry.dir = S_ISDIR(sb.st_mode);
      if (!entry.dir && !S_ISREG(sb.st_mode)) {
        continue;
      }
      if (filter(entry)) {
        continue;
      }
      return &entry;
    }
    return nullptr;
  }

  std::string path() {
    // TODO: platform?
    char fullpath[MAXPATHLEN];
    if (fcntl(dfd, F_GETPATH, fullpath) < 0) {
      fullpath[0] = '0';
    }
    return fullpath;
  }

  std::pair<const char *, const char *> map() {
    char *corpus_begin = nullptr;
    char *corpus_end = nullptr;
    struct stat statbuf;
    const int rv = fstat(dfd, &statbuf);
    if (rv == 0) {
      const off_t size = statbuf.st_size;
      char *result = (char *)mmap(0, size, PROT_READ, MAP_SHARED, dfd, 0);
      if (result != MAP_FAILED) {
        corpus_begin = result;
        corpus_end = corpus_begin + size;
        madvise(corpus_begin, size, MADV_SEQUENTIAL);
      }
    }
    return {corpus_begin, corpus_end};
  }

  int dfd;
  DIR *dir;
  Entry entry;
};
