/* search -- */

#include "taskpool/Async.h"
#include "file/File.h"
#include "stringlib/fastsearch.hpp"

#include <boost/utility/string_ref.hpp>

#include <sstream>
#include <iostream>

using namespace parallel;

// Global vars
boost::string_ref g_begin_path;
boost::string_ref g_path;
boost::string_ref g_name;
bool g_list;
bool g_quick;

namespace {
const char sgr_start[] = "\33[01;31m";
const char sgr_end[] = "\33[0m";
const char g_extensions[] = "h hh hpp c cc cpp txt ";
boost::string_ref g_color_name;
stringlib::fastsearch search;
}

const char *rfind_newline(const char *begin, const char *stop) {
  char token = *begin;
  while (begin != stop && token != '\n') {
    token = *begin--;
  }
  return begin == stop ? stop : begin + 2;
}

boost::string_ref make_string_ref(const char* begin, const char* end) {
  return boost::string_ref(begin, end - begin);
}

boost::string_ref name() { return g_color_name; }

void outs(const std::ostringstream& oss) {
  static std::mutex cout_mtx;
  std::lock_guard<std::mutex> lck(cout_mtx);
  std::cout << oss.str();
}

// TODO: Fix this function!
std::string build_path(File &file) {
  char fullpath[MAXPATHLEN];
  file.path(fullpath);
  std::ostringstream out;
  out << g_path
      << boost::string_ref(fullpath).substr(g_begin_path.size());
  return out.str();
}

const char *build(File &file, std::ostringstream &out, const char *found,
                  const char *begin, const char *end) {
  const char *first = rfind_newline(found, begin);
  const char *last = (char *)memchr(found, '\n', end - found);
  out << build_path(file) << ": " << make_string_ref(first, found) << name()
      << make_string_ref(found + g_name.size(), last) << '\n';
  return last;
}

void SearchFnc(int fd, const char *path) {
  File file(fd, path);
  boost::string_ref corpus = file.map();

  const char *found = corpus.begin();
  const char *begin = corpus.begin();
  std::ostringstream out;
  do {
    found = search(begin, corpus.end());
    if (found != corpus.end()) {
      begin = build(file, out, found, corpus.begin(), corpus.end());
    }
  } while (!g_quick && found != corpus.end());

  outs(out);
}

void PrintFnc(std::string entry) {
  const char *found = stringlib::fast_search(entry, g_name);
  if (found != (entry.c_str() + entry.size())) {
    std::ostringstream out;
    if (g_name.empty())
      out << entry << '\n';
    else
      out << make_string_ref(entry.c_str(), found) << name()
          << make_string_ref(found + g_name.size(), entry.c_str() + entry.size())
          << '\n';

    outs(out);
  }
}

struct Filter {
  bool operator()(const Entry &entry) {
    boost::string_ref extensions = g_extensions;
    boost::string_ref name = entry.name;
    const size_t ext = name.find_last_of('.');
    if (!entry.dir &&
        (ext == std::string::npos ||
         extensions.find(entry.name + ext + 1) == std::string::npos)) {
      return true;
    }
    return false;
  }
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

void start_search(bool no_color) {
  std::string color_name;
  if (no_color) {
    g_color_name = g_name;
  } else {
    std::ostringstream out;
    out << sgr_start << g_name << sgr_end;
    color_name = out.str();
    g_color_name = color_name;
  }

  search = stringlib::make_fast_search(g_name);

  TaskCompletion completion;
  async(FileFnc, &completion, -1, g_begin_path.data());
  WorkerThread::current()->work_until_done(&completion);
}
