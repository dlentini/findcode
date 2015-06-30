
#include <taskpool/TaskPool.h>
#include <string.h>
#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

// Global vars :)
extern std::string g_begin_path;
extern std::string g_path;
extern std::string g_name;
extern bool g_list;
extern bool g_nocolor;
extern bool g_quick;

extern void start_search();

int main(int ac, char *const av[]) try {

  std::cout.sync_with_stdio(false);
  std::cout << std::nounitbuf;

  char cwd[PATH_MAX + 1];
  getcwd(cwd, sizeof(cwd));
  unsigned threads = std::thread::hardware_concurrency();

  po::options_description desc("Usage: ext [options] [name] ...\nOptions");
  desc.add_options()("help,h", "Produce help message.")(
      "path,p", po::value<std::string>(&g_begin_path)->default_value(cwd),
      "The path to search in.")(
      "name,n", po::value<std::string>(&g_name)->implicit_value(""),
      "The name to search for.")(
      "threads,j", po::value<unsigned>(&threads)->default_value(threads),
      "Number of threads.")("list,l", "List or search for files.")(
      "nocolor,c",
      "Disable color output.")("quick,q", "Stop searching after first match.");

  po::positional_options_description p;
  p.add("name", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(),
            vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  g_path = g_begin_path;
  g_list = vm.count("list") || g_name.empty();
  g_nocolor = vm.count("nocolor") || !isatty(fileno(stdout));
  g_quick = vm.count("quick");

  char actualpath[PATH_MAX + 1];
  g_begin_path = realpath(g_begin_path.c_str(), actualpath);
  if (g_path == g_begin_path)
    g_path = ".";

  parallel::TaskPool task_pool;
  task_pool.start(threads);

  start_search();
} catch (std::exception &e) {
  std::cerr << "ERROR: Unhandled exception\n";
  std::cerr << "  what(): " << e.what() << "\n";
  return 1;
}
