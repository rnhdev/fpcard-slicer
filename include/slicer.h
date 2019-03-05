#ifndef FP_CARDSLICER_SLICER_H
#define FP_CARDSLICER_SLICER_H

#include <vector>
#include "image.h"

namespace fpcard_slicer {
  namespace slicer {
    const int MINIMUN_DISTANCE = 10;
    const int MINIMUM_WIDTH = 25;
    const int MINIMUM_HEIGHT = 25;
    const float LIMIT_SEARCH_INIT = 3/10.0;
    const float LIMIT_SEARCH_FIN  = 7/10.0;
    const int MAXIMUM_SIZE_WIDTH = 100;
    const int MAXIMUM_SIZE_LEFT = 50;
    const int MAXIMUM_SIZE_RIGHT = 70;
    const float Z_FAC = 8.0;

    enum Mode {
      General,
      Sector
    };

    class Slicer {
    public:
      Slicer():
        _fp_number(10), _bin_umbral(10), _mode(General), _size_block(20) {
      }
      Slicer(int fp_number, int bin_umbral, Mode mode, int size_block ):
        _fp_number(fp_number), _bin_umbral(bin_umbral), _mode(mode), _size_block(size_block) {

      }
      ~Slicer(){}
      const std::vector<fpcard_slicer::image::Clip> CalculateSlice(std::shared_ptr<image::Image>& img) {
        return CalculateSlice(img, "");
      }
      const std::vector<fpcard_slicer::image::Clip> CalculateSlice(std::shared_ptr<image::Image>&, const std::string&);
    private:
      int _bin_umbral, _size_block, _fp_number;
      Mode _mode;
      void ApplyFilters(std::shared_ptr<image::Image> &);
      image::Clip SearchEdges(std::shared_ptr<image::Image> &, int, int);
      image::Clip SearchFingerprint(std::shared_ptr<image::Image>& image, int index,
                                    std::vector<int> vec_sum_black,std::vector<int> vec_sum_black_max);
      std::vector<image::Clip> SearchFingerprints(std::shared_ptr<image::Image>& image);
    };
  }// namespace image
}// namespace fpcard_slicer
#endif //FP_CARDSLICER_SLICER_H
