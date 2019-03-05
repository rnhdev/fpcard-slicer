#ifndef FPCARD_SLICER_JPEG_H
#define FPCARD_SLICER_JPEG_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct jpeg_error_mgr;

namespace jpeg{
  void load_file(const std::string&, std::vector<unsigned char>&, unsigned&, unsigned&);
  void save_file(const std::string& filename, std::vector<unsigned char>in, unsigned w, unsigned h, int quality );
}

#endif //FPCARD_SLICER_JPEG_H
