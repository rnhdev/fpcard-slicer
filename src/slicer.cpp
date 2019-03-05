#include <slicer.h>

namespace fpcard_slicer {
  namespace slicer {
    const std::vector<image::Clip> Slicer::CalculateSlice(std::shared_ptr<image::Image> &image,
                                                          const std::string& partial_out) {
      auto scaled_image = image->Scale(1.0 / Z_FAC);
      scaled_image->ApplyBinarizedFilter(_bin_umbral);

      auto clip_edges = SearchEdges(scaled_image, 0.8, 10);
      auto clip_image = scaled_image->Cut(clip_edges);

      image::Clip clip_top(0, clip_image->width(), 0, clip_image->height() / 2);
      image::Clip clip_bottom(0, clip_image->width(), clip_image->height() / 2, clip_image->height());

      auto image_top = clip_image->Cut(clip_top);
      auto image_bottom = clip_image->Cut(clip_bottom);

      ApplyFilters(image_top);
      ApplyFilters(image_bottom);

      auto clip_top_list = SearchFingerprints(image_top);
      auto clip_bottom_list = SearchFingerprints(image_bottom);

      std::vector<image::Clip> result;
      for (auto &clip:clip_top_list) {
        clip.Scale(Z_FAC);
        result.push_back(clip);
      }

      for (auto &clip:clip_bottom_list) {
        clip.Expand(0, image_top->height());
        clip.Scale(Z_FAC);
        result.push_back(clip);
      }

      clip_edges.Scale(Z_FAC);

      for (auto &clip:result) {
        clip.set_left(clip.left() + clip_edges.left());
        clip.set_right(clip.right() + clip_edges.left());
        clip.set_top(clip.top() + clip_edges.top());
        clip.set_bottom(clip.bottom() + clip_edges.top());
      }

      if(partial_out.length()>0) {
        scaled_image->Save(partial_out + "/01_binarized.jpg", 80);
        clip_image->Save(partial_out   + "/02_clip.jpg", 80);
        image_top->Save(partial_out    + "/03_top.jpg", 80);
        image_bottom->Save(partial_out + "/04_bottom.jpg", 80);
      }

      return result;
    }

    image::Clip Slicer::SearchEdges(std::shared_ptr<image::Image> &image, int umbral, int padding) {
      image::Clip clip;

      //For white space
      clip.set_left(padding);
      for (unsigned x = padding; x < image->width(); ++x) {
        unsigned sum = 0;
        for (unsigned y = 0; y < image->height(); ++y)
          sum += image->pixel(y * image->width() + x);

        if (sum / image->height() > umbral) {
          clip.set_left(x);
          continue;
        }

        break;
      }

      clip.set_right(image->width() - padding);
      for (unsigned x = image->width() - padding; x >= 0; --x) {
        unsigned sum = 0;
        for (unsigned y = 0; y < image->height(); ++y)
          sum += image->pixel(y * image->width() + x);

        if (sum / image->height() > umbral) {
          clip.set_right(x);
          continue;
        }

        break;
      }

      clip.set_top(padding);
      for (unsigned y = padding; y < image->height(); ++y) {
        unsigned sum = 0;
        for (unsigned x = 0; x < image->width(); ++x)
          sum += image->pixel(y * image->width() + x);

        if (sum / image->width() > umbral) {
          clip.set_top(y);
          continue;
        }

        break;
      }

      clip.set_bottom(image->height() - padding);
      for (unsigned y = image->height() - padding; y >= 0; --y) {
        unsigned sum = 0;
        for (unsigned x = 0; x < image->width(); ++x)
          sum += image->pixel(y * image->width() + x);

        if (sum / image->width() > umbral) {
          clip.set_bottom(y);
          continue;
        }
        break;
      }

      //For black space
      clip.set_left(padding);
      for (unsigned x = padding; x < image->width(); ++x) {
        unsigned sum = 0;
        for (unsigned y = 0; y < image->height(); ++y)
          sum += !image->pixel(y * image->width() + x);

        if (sum / image->height() > umbral) {
          clip.set_left(x);
          continue;
        }

        break;
      }

      clip.set_right(image->width() - padding);
      for (unsigned x = image->width() - padding; x >= 0; --x) {
        unsigned sum = 0;
        for (unsigned y = 0; y < image->height(); ++y)
          sum += !image->pixel(y * image->width() + x);

        if (sum / image->height() > umbral) {
          clip.set_right(x);
          continue;
        }

        break;
      }

      clip.set_top(padding);
      for (unsigned y = padding; y < image->height(); ++y) {
        unsigned sum = 0;
        for (unsigned x = 0; x < image->width(); ++x)
          sum += !image->pixel(y * image->width() + x);

        if (sum / image->width() > umbral) {
          clip.set_top(y);
          continue;
        }

        break;
      }

      clip.set_bottom(image->height() - padding);
      for (unsigned y = image->height() - padding; y >= 0; --y) {
        unsigned sum = 0;
        for (unsigned x = 0; x < image->width(); ++x)
          sum += !image->pixel(y * image->width() + x);

        if (sum / image->width() > umbral) {
          clip.set_bottom(y);
          continue;
        }
        break;
      }

      return clip;
    }

    void Slicer::ApplyFilters(std::shared_ptr<image::Image> &image) {
      image->ApplyAverageFilter(5, 5);
      image->ApplyAverageFilter(5, 9);
      image->ApplyVerticalFilter(3, 7);
      image->ApplyVerticalFilter(7, 7);
      image->ApplyHorizontalWhiteFilter(7, 11);
      image->ApplyVerticalFilter(11, 7);
      image->ApplyVerticalFilter(15, 7);
      image->ApplyEdgeFilter(5, image->white());
      image->ApplyHorizontalBlackFilter(5, 21);
      image->ApplyEdgeFilter(5, image->white());
    }

    image::Clip
    Slicer::SearchFingerprint(std::shared_ptr<image::Image> &image, int last, std::vector<int> vec_sum_black,
                              std::vector<int> vec_sum_black_max) {
      image::Clip coord;
      START:
      // Set h_max
      int h_max = 0, h_max_val = 0;
      bool start = false;
      unsigned end = image->width();
      for (unsigned k = last; k < end; k++) {
        if (!start) {
          if (vec_sum_black_max[k] > 20) {
            start = true;
            end = k + MAXIMUM_SIZE_WIDTH;
            h_max_val = vec_sum_black_max[k];
            h_max = k;
          }
        } else {
          if (vec_sum_black_max[k] == 0)
            break;

          if (vec_sum_black_max[k] > h_max_val) {
            h_max_val = vec_sum_black_max[k];
            h_max = k;
          }
        }
      }

      if (h_max_val == 0)
        return coord;

      // Set left
      int k;
      for (k = h_max; k >= last; k--) {
        if (vec_sum_black[k] < 10) {
          k--;
          break;
        }
      }
      coord.set_left(k);

      // Set right
      for (k = h_max; k < image->width(); k++) {
        if (vec_sum_black[k] < 10) {
          k++;
          break;
        }
      }
      coord.set_right(k);
      if (coord.width() < MINIMUM_WIDTH) {
        last = h_max + 1;
        goto START;
      }


      // Check join
      if (coord.width() > MAXIMUM_SIZE_WIDTH) {
        unsigned min_pos = 0;
        unsigned min_val = image->height();
        for (k = coord.left() + 30; k < coord.left() + MAXIMUM_SIZE_WIDTH; k++) {
          if (vec_sum_black[k] < min_val) {
            min_val = vec_sum_black[k];
            min_pos = k;
          }
        }
        coord.set_right(min_pos);
      }

      //SearchHeight(image, coord);
      unsigned sum;
      unsigned init = image->height() * LIMIT_SEARCH_INIT;
      unsigned fin = image->height() * LIMIT_SEARCH_FIN;

      // Accum horizontal BLACK and save to vector
      std::vector<int> vec(image->height());
      for (unsigned y = 0; y < image->height(); y++) {
        sum = 0;
        for (unsigned x = coord.left(); x < coord.right(); x++) {
          sum += !image->pixel(y * image->width() + x);
        }
        vec[y] = sum;
      }

      // Find max value position of vector
      unsigned max = 0, max_val = 0;
      for (unsigned y = init; y < fin; y++) {
        if (vec[y] > max_val) {
          max_val = vec[y];
          max = y;
        }
      }

      // Find top
      for (k = max; k >= 0; --k) {
        if (vec[k] < 10) {
          --k;
          break;
        }
      }
      coord.set_top(k);

      // Find bottom coord
      for (k = max; k < image->height(); ++k) {
        if (vec[k] < 10) {
          ++k;
          break;
        }
      }
      coord.set_bottom(k);

      return coord;
    }

    std::vector<image::Clip> Slicer::SearchFingerprints(std::shared_ptr<image::Image> &image) {
      std::vector<image::Clip> coord_list;
      std::vector<int> vec_sum_black(image->width());

      for (auto &value:vec_sum_black) value = 0;

      int index = 0;
      for (auto it = image->get(); it < image->end(); ++it) {
        vec_sum_black[index % image->width()] += image->white() - *it;
        ++index;
      }


      std::vector<int> vec_sum_black_max(vec_sum_black.begin(), vec_sum_black.end());

      unsigned last = 0;
      for (unsigned k = 0; k < (unsigned) _fp_number / 2; k++) {
        auto coord = SearchFingerprint(image, last, vec_sum_black, vec_sum_black_max);

        for (unsigned i = 0; i <= coord.right(); i++) {
          vec_sum_black_max[i] = 0;
        }
        coord_list.push_back(coord);
        last = coord.right();
      }

      return coord_list;
    }
  }
}