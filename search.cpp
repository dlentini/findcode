/* search -- */

#include <taskpool/Async.h>
#include <file/File.h>

#include <boost/algorithm/searching/boyer_moore.hpp>
#include <stringlib/fastsearch.hpp>

#include <sstream>

using namespace parallel;

// Global vars
std::string g_begin_path;
std::string g_path;
std::string g_name;
bool g_list;
bool g_nocolor;
bool g_quick;

namespace {
std::mutex printf_mtx;
std::string sgr_start = "\33[01;31m";
std::string sgr_end = "\33[0m";
}

const char *rfind_newline(const char *begin, const char *stop) {
  char token = *begin;
  while (begin != stop && token != '\n') {
    token = *begin--;
  }
  return begin == stop ? stop : begin + 2;
}

std::string build_path(File &file) {
  return g_path + file.path().substr(g_begin_path.size());
}

std::string name() { return g_nocolor ? g_name : sgr_start + g_name + sgr_end; }

const char *Build(File &file, std::ostringstream &out, const char *found,
                  const char *begin, const char *end) {
  const char *first = rfind_newline(found, begin);
  const char *last = (char *)memchr(found, '\n', end - found);
  out << build_path(file) << ": " << std::string(first, found) << name()
      << std::string(found + g_name.size(), last) << '\n';
  return last;
}

void SearchFnc(int fd, const char *path) {
  File file(fd, path);
  auto corpus = file.map();

  auto search = stringlib::make_fast_search(g_name);
  // auto search = boost::algorithm::make_boyer_moore(g_name);

  const char *found = corpus.first;
  const char *begin = corpus.first;
  std::ostringstream out;
  do {
    found = search(begin, corpus.second);
    if (found != corpus.second) {
      begin = Build(file, out, found, corpus.first, corpus.second);
    }
  } while (!g_quick && found != corpus.second);

  std::lock_guard<std::mutex> lck(printf_mtx);
  std::cout << out.str();
}

void PrintFnc(std::string entry) {
  const char *found = stringlib::fast_search(entry, g_name);
  if (found != (entry.c_str() + entry.size())) {
    std::ostringstream out;
    if (g_name.empty())
      out << entry << '\n';
    else
      out << std::string(entry.c_str(), found) << name()
          << std::string(found + g_name.size(), entry.c_str() + entry.size())
          << '\n';

    std::lock_guard<std::mutex> lck(printf_mtx);
    std::cout << out.str();
  }
}

struct Filter {
  bool operator()(const Entry &entry) {
    static std::string extensions = "h hh hpp c cc cpp txt ";
    name = entry.name;
    const size_t ext = name.find_last_of('.');
    if (!entry.dir &&
        (ext == std::string::npos ||
         extensions.find(entry.name + ext + 1) == std::string::npos)) {
      return true;
    }
    return false;
  }
  std::string name;
};

void FileFnc(int fd, const char *path) {
  File file(fd, path);
  Entry *entry = nullptr;
  while ((entry = file.next_entry(Filter())) != nullptr) {
    if (entry->dir) {
      async(FileFnc, file.dfd, entry->name);
    } else if (!g_name.empty() && !g_list) {
      async(SearchFnc, file.dfd, entry->name);
    } else {
      async(PrintFnc, build_path(file) + "/" + entry->name);
    }
  }
}

void start_search() {
  TaskCompletion completion;
  async(FileFnc, &completion, -1, g_begin_path.c_str());
  WorkerThread::current()->work_until_done(&completion);
}
