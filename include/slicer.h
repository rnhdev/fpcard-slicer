//
// Created by nico on 18/02/19.
//

#ifndef FP_CARDSLICER_SLICER_H
#define FP_CARDSLICER_SLICER_H

#include <vector>
#include "image.h"

#define MINIMUN_DISTANCE 10
#define MINIMUM_WIDTH 25
#define MINIMUM_HEIGHT 25
#define LIMIT_SEARCH_INIT 3/10.0
#define LIMIT_SEARCH_FIN 7/10.0
#define MAXIMUM_SIZE_WIDTH 100
#define MAXIMUM_SIZE_LEFT 50
#define MAXIMUM_SIZE_RIGHT 70


namespace fpcard_slicer {
  namespace slicer {
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
      const std::vector<fpcard_slicer::image::Clip> CalculateSlice(std::unique_ptr<image::Image>&);
    private:
      int _bin_umbral, _size_block, _fp_number;
      Mode _mode;
      void ApplyFilters(std::unique_ptr<image::Image> &);
      image::Clip RemoveEdges(std::unique_ptr<image::Image> &, int, int);
      image::Clip SearchFingerprint(std::unique_ptr<image::Image>& image, int index, int vec_sum_black[],
                                    int vec_sum_black_max[]);
      std::vector<image::Clip> SearchFingerprints(std::unique_ptr<image::Image>& image);
    };
  }// namespace image
}// namespace fpcard_slicer
#endif //FP_CARDSLICER_SLICER_H
