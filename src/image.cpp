#include <cstdlib>
#include <stdexcept>
#include <lodepng.h>
#include <math.h>
#include <numeric>
#include <iostream>
#include <algorithm>


#include "image.h"
#include "../third_party/jpeg/jpeg.h"

#define sround(x) ((int) (((x)<0) ? (x)-0.5 : (x)+0.5))

namespace fpcard_slicer {
  namespace image {
    Image::Image(const std::string &filename) {
      switch (extension(filename)) {
        case Format::JPEG:
          ReadJPEG(filename);
          break;
        case Format::PNG:
          ReadPNG(filename);
          break;
        default:
          throw std::invalid_argument("Invalid extension");
      }
    }
    Image::~Image() {
      Clear();
    }
    void Image::Save(const std::string &filename) {
      Save(filename, 100);
    }
    void Image::Save(const std::string &filename, int qlt) {
      if (_mode == Binary)
        ToBinary();

      switch (extension(filename)) {
        case Format::JPEG:
          SaveJPEG(filename, qlt);
          break;
        case Format::PNG:
          SavePNG(filename);
          break;
        default:
          throw std::invalid_argument("Invalid extension");
      }


      if (_mode == Binary)
        ToGrayscale();
    }

    void Image::ReadPNG(const std::string &filename) {
      std::vector<unsigned char> png;
      unsigned w, h;

      unsigned error = lodepng::load_file(png, filename);

      if (!error) {
        Clear();
        error = lodepng::decode(_data, w, h, png, LodePNGColorType::LCT_GREY, 8);
      }

      if (error)
        throw std::invalid_argument("Decode error: " + std::string(lodepng_error_text(error)));

      _size.width = w;
      _size.height = h;
      _mode = Grayscale;
    }

    void Image::ReadJPEG(const std::string &filename) {
      jpeg::load_file(filename, _data, _size.width, _size.height);
      _mode = Grayscale;
    }

    void Image::SavePNG(const std::string &filename) {
      std::vector<unsigned char> png;
      unsigned error = lodepng::encode(png, _data, width(), height(), LodePNGColorType::LCT_GREY, 8);

      if (!error)
        lodepng::save_file(png, filename);

      if (error)
        throw std::invalid_argument("Encode error: " + std::string(lodepng_error_text(error)));
    }

    void Image::SaveJPEG(const std::string &filename, int qlt) {
      jpeg::save_file(filename, _data, _size.width, _size.height, qlt);
    }

    Format Image::extension(const std::string &file) {
      // to lower
      std::string filelower(file);
      for (auto &c : filelower) c = std::tolower(c);

      if (filelower.rfind(".jpg") != std::string::npos)
        return Format::JPEG;

      if (filelower.rfind(".png") != std::string::npos)
        return Format::PNG;

      return Format::Other;
    }

    std::shared_ptr<Image> Image::Scale(float factor) {
      auto h = (unsigned) sround((float) height() * factor);
      auto w = (unsigned) sround((float) width() * factor);
      std::vector<Pixel> new_data(h * w);

      auto inc_factor = (int) round(1.0 / factor);

      unsigned index = 0;
      for (auto &value : new_data) {
        unsigned y_start = (index / w) * inc_factor;
        unsigned x_start = (index % w) * inc_factor;

        unsigned sum = 0;
        for (unsigned line = y_start; line < y_start + inc_factor; ++line) {
          auto it_line = _data.begin() + (width() * line) + x_start;

          sum += std::accumulate(it_line, it_line + inc_factor, 0);
        }
        value = (Pixel) (sum / (inc_factor * inc_factor));
        ++index;
      }

      return std::make_shared<Image>(new_data, w, h, _mode);
    }

    void Image::ApplyBinarizedFilter(unsigned umbral) {
      const unsigned max_black = 30;
      unsigned mean = 0, count = 0;

      //average calc
      for (auto &value : _data) {
        if (value > max_black) {
          mean += value;
          count++;
        }
      }
      mean = (count != 0) ? mean / count : 0;

      _mode = Binary;
      for (auto &pixel : _data) pixel = (pixel > (mean + umbral)) ? white() : black();
    }

    std::shared_ptr<Image> Image::Cut(Clip clip) {
      std::vector<Pixel> new_data;

      for (unsigned line = clip.top(); line < clip.bottom(); ++line) {
        auto it_line = _data.begin() + (width() * line) + clip.left();
        std::vector<Pixel> vector_line(it_line, it_line + clip.width());

        new_data.insert(new_data.end(), vector_line.begin(), vector_line.end());
      }

      return std::make_shared<Image>(new_data, clip.width(), clip.height(), _mode);
    }

    void Image::ApplyAverageFilter(unsigned bw, unsigned bh) {
      std::vector<Pixel> new_data(_data.begin(), _data.end());
      int midblock_h = (int) floor(bh / 2.0);
      int midblock_w = (int) floor(bw / 2.0);
      int block_h = midblock_h * 2;
      int block_w = midblock_w * 2;
      auto block_size = (double) (block_h * block_w);

      unsigned index = 0;
      for (auto &value : new_data) {
        int y_start = index / width() - midblock_h;
        int x_start = index % width() - midblock_w;
        int y_end = y_start + block_h;
        int x_end = x_start + block_w;

        if (y_start >= 0 && y_end < height() && x_start >= 0 && x_end < width()) {
          int sum = 0;
          for (int line = y_start; line <= y_end; ++line) {
            sum = std::accumulate(_data.begin() + (width() * line) + x_start,
                                  _data.begin() + (width() * line) + x_end, sum);
          }

          value = (sum / block_size > white() / 2.0) ? white() : black();
        }

        ++index;
      }

      Clear();
      _data.insert(_data.end(), new_data.begin(), new_data.end());
    }

    void Image::ApplyVerticalFilter(unsigned bw, unsigned bh) {
      std::vector<Pixel> new_data(_data.begin(), _data.end());
      int midblock_h = (int) floor(bh / 2.0);
      int midblock_w = (int) floor(bw / 2.0);
      int block_h = midblock_h * 2;
      int block_w = midblock_w * 2;

      for (unsigned index = 0; index < length(); ++index) {
        int y_start = index / width() - midblock_h;
        int x_start = index % width() - midblock_w;
        int y_end = y_start + block_h;
        int x_end = x_start + block_w;

        if (y_start >= 0 && y_end < height() && x_start >= 0 && x_end < width()) {

          int lsum = 0;
          int rsum = 0;
          for (int y = y_start; y <= y_end; ++y) {
            lsum += pixel((unsigned) x_start, (unsigned) y);
            rsum += pixel((unsigned) x_end, (unsigned) y);
          }

          if ((lsum + rsum) == block_h * 2) {
            for (int line = y_start; line < y_end; ++line) {
              for (auto it = new_data.begin() + (width() * line) + x_start;
                   it < new_data.begin() + (width() * line) + x_end; ++it)
                *it = white();
            }
          }
        }
      }

      Clear();
      _data.insert(_data.end(), new_data.begin(), new_data.end());
    }

    void Image::ApplyHorizontalWhiteFilter(unsigned bw, unsigned bh) {
      std::vector<Pixel> new_data(_data.begin(), _data.end());
      int midblock_h = (int) floor(bh / 2.0);
      int midblock_w = (int) floor(bw / 2.0);
      int block_h = midblock_h * 2;
      int block_w = midblock_w * 2;

      for (unsigned index = 0; index < length(); ++index) {
        int y_start = index / width() - midblock_h;
        int x_start = index % width() - midblock_w;
        int y_end = y_start + block_h;
        int x_end = x_start + block_w;

        if (y_start >= 0 && y_end < height() && x_start >= 0 && x_end < width()) {
          int lsum = std::accumulate(_data.begin() + y_start * width() + x_start,
                                     _data.begin() + y_start * width() + x_end, 0);
          int rsum = std::accumulate(_data.begin() + (y_start + block_h) * width() + x_start,
                                     _data.begin() + (y_start + block_h) * width() + x_end, 0);

          if ((lsum + rsum) == block_w * 2) {
            for (int line = y_start; line < y_end; ++line) {
              for (auto it = new_data.begin() + (width() * line) + x_start;
                   it < new_data.begin() + (width() * line) + x_end; ++it)
                *it = white();
            }
          }
        }
      }

      Clear();
      _data.insert(_data.end(), new_data.begin(), new_data.end());
    }

    void Image::ApplyHorizontalBlackFilter(unsigned bw, unsigned bh) {
      std::vector<Pixel> new_data(_data.begin(), _data.end());
      int midblock_h = (int) floor(bh / 2.0);
      int midblock_w = (int) floor(bw / 2.0);
      int block_h = midblock_h * 2;
      int block_w = midblock_w * 2;

      for (unsigned index = 0; index < length(); ++index) {
        int y_start = index / width() - midblock_h;
        int x_start = index % width() - midblock_w;
        int y_end = y_start + block_h;
        int x_end = x_start + block_w;

        if (y_start >= 0 && y_end < height() && x_start >= 0 && x_end < width()) {
          int lsum = std::accumulate(_data.begin() + y_start * width() + x_start,
                                     _data.begin() + y_start * width() + x_end, 0);
          int rsum = std::accumulate(_data.begin() + (y_start + block_h) * width() + x_start,
                                     _data.begin() + (y_start + block_h) * width() + x_end, 0);

          lsum = block_w - lsum;
          rsum = block_w - rsum;

          if ((lsum + rsum) == block_w * 2) {
            for (int line = y_start; line < y_end; ++line) {
              for (auto it = new_data.begin() + (width() * line) + x_start;
                   it < new_data.begin() + (width() * line) + x_end; ++it)
                *it = black();
            }
          }
        }
      }

      Clear();
      _data.insert(_data.end(), new_data.begin(), new_data.end());
    }

    void Image::ApplyEdgeFilter(unsigned size, Pixel color) {
      //Left
      for (unsigned line = 0; line < height(); ++line) {
        for (auto it = _data.begin() + (width() * line); it < _data.begin() + (width() * line) + size; it++)
          *it = color;
      }

      //Right
      for (unsigned line = 1; line <= height(); ++line) {
        for (auto it = _data.begin() + (width() * line) - size; it < _data.begin() + (width() * line); it++)
          *it = color;
      }

      //Top
      unsigned index = 0;
      for (auto &value : _data) {
        if (index / width() < size) value = color;
        ++index;
      }

      //Bottom
      index = 0;
      for (auto &value : _data) {
        unsigned y = index / width();
        if (y >= height() - size && y < height()) value = color;
        ++index;
      }
    }
  }
}