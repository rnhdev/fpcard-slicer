#include <iostream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <memory>
#include "parse_arguments.h"
namespace fpcard_slicer {
  namespace application {
    struct stat info;

    void ShowUsage(std::string name)
    {
      std::cerr << "Usage: " << name << " <option(s)> SOURCES"
                << "Options:\n"
                << "\t-h,--help\tShow this help message\n"
                << "\t-s,--source SOURCE\tSpecify the image source with 500dpi. Can be source file or directory path\n"
                << "\t-d,--destination DESTINATION\tSpecify the destination path\n"
                << "\t-f,--format OUTPUT_FORMAT\tSpecify output format (png or jpeg)\n"
                << "\t-q,--quality OUTPUT_QUALITY\tSpecify the output quality (only for jpg output format)\n"
                << "\t-o,--demo DEMO_MODE\tSet demo mode. If set, partial output result\n"
                << std::endl;
    }

    std::vector<std::string> GetSources(std::string source) {
      std::vector<std::string> file_list;
      std::shared_ptr<DIR> directory_ptr(opendir(source.c_str()), [](DIR* dir){ dir && closedir(dir); });
      struct dirent *dirent_ptr;
      while ((dirent_ptr = readdir(directory_ptr.get())) != nullptr) {
        std::string name(dirent_ptr->d_name);

        if(name.rfind(".jpg") != std::string::npos || name.rfind(".png") != std::string::npos)
          file_list.push_back(source + "/" + name);
      }

      return file_list;
    }

    bool ParseArguments(int argc, char** argv, SlicerConfig& config) {
      if (argc < 3) {
        ShowUsage(argv[0]);
        return false;
      }

      std::vector <std::string> sources;
      int quality = 80;
      bool demo = false;
      std::string source, destination, format = "jpg";
      for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
          ShowUsage(argv[0]);
          return false;
        }
        else if ((arg == "-s") || (arg == "--source")) {
          if (i + 1 < argc) {
            source = argv[++i];
          } else {
            std::cerr << "--source option requires one argument." << std::endl;
            return false;
          }
        }
        else if ((arg == "-d") || (arg == "--destination")) {
          if (i + 1 < argc) {
            destination = argv[++i];
          } else {
            std::cerr << "--destination option requires one argument." << std::endl;
            return false;
          }
        }
        else if ((arg == "-f") || (arg == "--format")) {
          if (i + 1 < argc) {
            format = argv[++i];
            if(format != "jpg" && format != "png") {
              std::cerr << "--format " << format <<  " not support." << std::endl;
              return false;
            }
          } else {
            std::cerr << "--format option requires one argument." << std::endl;
            return false;
          }
        }
        else if ((arg == "-q") || (arg == "--quality")) {
          if (i + 1 < argc) {
            quality = atoi(argv[++i]);
          } else {
            std::cerr << "--quality option requires one argument." << std::endl;
            return false;
          }
        }
        else if ((arg == "-o") || (arg == "--demo")) {
          demo = true;
        }
      }

      if(source.empty()) {
        std::cerr << "--source is required." << std::endl;
        return false;
      }
      if(destination.empty()) {
        std::cerr << "--destination is required." << std::endl;
        return false;
      }

      std::vector<std::string> source_list;

      if( stat(source.c_str(),&info) == 0 ) {
        if( info.st_mode & S_IFDIR ) {
          source_list = GetSources(source);

          if(source_list.empty()) {
            std::cerr << "--source is empty if png or jpg images" << std::endl;
            return false;
          }
        }
        else if( info.st_mode & S_IFREG ) {
          source_list.push_back(source);
        }
        else {
          std::cerr << "--source is invalid." << std::endl;
          return false;
        }
      }
      else {
        std::cerr << "--source is invalid." << std::endl;
        return false;
      }

      if( stat(destination.c_str(), &info ) != 0 || info.st_mode & S_IFREG) {
        std::cerr << "--destination is invalid." << std::endl;
        return false;
      }

      config.set_source_list(source_list);
      config.set_destination(destination);
      config.set_output_format(format);
      config.set_output_quality(quality);
      config.set_demo_mode(demo);

      return true;
    }
  }
}