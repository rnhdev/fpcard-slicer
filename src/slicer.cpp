//
// Created by nico on 18/02/19.
//

#include <slicer.h>

extern "C" {
#ifndef _AN2K_H
#include <an2k.h>
#endif

#ifndef _NFSEG_H
#include <nfseg.h>
#endif

#ifndef _IMG_IO_H
#include <img_io.h>
#endif
}

using namespace fpcard_slicer;
using namespace fpcard_slicer::slicer;

const std::vector<image::Clip> Slicer::CalculateSlice(std::unique_ptr<image::Image>& image) {
  auto scaled_image = image->Scale(1.0/Z_FAC);
  scaled_image->FilterBinarize(_bin_umbral);

  auto clip_edges = RemoveEdges(scaled_image, 0.8, 10);
  auto bin_image2 = scaled_image->Cut(clip_edges);

  image::Clip clip_top(0, bin_image2->width(), 0, bin_image2->height()/2);
  image::Clip clip_bottom(0, bin_image2->width(), bin_image2->height()/2, bin_image2->height());

  auto image_top = bin_image2->Cut(clip_top);
  auto image_bottom = bin_image2->Cut(clip_bottom);

  ApplyFilters(image_top);
  ApplyFilters(image_bottom);

  auto clip_top_list = SearchFingerprints(image_top);
  auto clip_bottom_list = SearchFingerprints(image_bottom);

  std::vector<image::Clip> result;
  for(int k = 0; k < clip_top_list.size(); k++) {
    clip_top_list[k].Scale(Z_FAC);
    result.push_back(clip_top_list[k]);
  }

  for(int k = 0; k < clip_bottom_list.size(); k++) {
    clip_bottom_list[k].Expand(0, image_top->height());
    clip_bottom_list[k].Scale(Z_FAC);

    result.push_back(clip_bottom_list[k]);
  }

  clip_edges.Scale(Z_FAC);

  for(int k = 0; k < result.size(); ++k) {
    result[k].set_left(result[k].left() + clip_edges.left());
    result[k].set_right(result[k].right() + clip_edges.left());
    result[k].set_top(result[k].top() + clip_edges.top());
    result[k].set_bottom(result[k].bottom() + clip_edges.top());
  }

  return result;
}

image::Clip Slicer::RemoveEdges(std::unique_ptr<image::Image> &image, int umbral, int padding) {
  image::Clip clip;

  //Remove white lines
  clip.set_left(padding);
  for(int x = padding; x < image->width(); ++x) {
    int sum = 0;
    for (int y = 0; y < image->height(); ++y)
      sum += image->pixel(y * image->width() + x);

    if(sum/image->height() > umbral) {
      clip.set_left(x);
      continue;
    }

    break;
  }

  clip.set_right(image->width() - padding);
  for(int x = image->width() - padding; x >= 0; --x) {
    int sum = 0;
    for (int y = 0; y < image->height(); ++y)
      sum += image->pixel(y * image->width() + x);

    if(sum/image->height() > umbral) {
      clip.set_right(x);
      continue;
    }

    break;
  }

  clip.set_top(padding);
  for(int y = padding; y < image->height(); ++y) {
    int sum = 0;
    for (int x = 0; x < image->width(); ++x)
      sum += image->pixel(y * image->width() + x);

    if(sum/image->width() > umbral) {
      clip.set_top(y);
      continue;
    }

    break;
  }

  clip.set_bottom(image->height() - padding);
  for(int y = image->height() - padding; y >=0 ; --y) {
    int sum = 0;
    for (int x = 0; x < image->width(); ++x)
      sum += image->pixel(y * image->width() + x);

    if(sum/image->width() > umbral) {
      clip.set_bottom(y);
      continue;
    }
    break;
  }

  //Remove black lines
  clip.set_left(padding);
  for(int x = padding; x < image->width(); ++x) {
    int sum = 0;
    for (int y = 0; y < image->height(); ++y)
      sum += !image->pixel(y * image->width() + x);

    if(sum/image->height() > umbral) {
      clip.set_left(x);
      continue;
    }

    break;
  }

  clip.set_right(image->width() - padding);
  for(int x = image->width() - padding; x >= 0; --x) {
    int sum = 0;
    for (int y = 0; y < image->height(); ++y)
      sum += !image->pixel(y * image->width() + x);

    if(sum/image->height() > umbral) {
      clip.set_right(x);
      continue;
    }

    break;
  }

  clip.set_top(padding);
  for(int y = padding; y < image->height(); ++y) {
    int sum = 0;
    for (int x = 0; x < image->width(); ++x)
      sum += !image->pixel(y * image->width() + x);

    if(sum/image->width() > umbral) {
      clip.set_top(y);
      continue;
    }

    break;
  }

  clip.set_bottom(image->height() - padding);
  for(int y = image->height() - padding; y >=0 ; --y) {
    int sum = 0;
    for (int x = 0; x < image->width(); ++x)
      sum += !image->pixel(y * image->width() + x);

    if(sum/image->width() > umbral) {
      clip.set_bottom(y);
      continue;
    }
    break;
  }

  return clip;
}

void Slicer::ApplyFilters(std::unique_ptr<image::Image> &image) {
  image->FilterMean(5, 5);
  image->FilterMean(5, 9);

  image->FilterRemoveVerticalLines(20, 10);

  image->FilterRemoveHorizontalLines(3, 7);
  image->FilterRemoveHorizontalLines(7, 7);

  image->FilterRemoveVerticalLines2(7, 11, 0);

  image->FilterRemoveHorizontalLines(11, 7);
  image->FilterRemoveHorizontalLines(15, 7);

  image->FilterEdge(5, image::WHITE_BINARY);
  image->FilterRemoveHorizontalLines2(5, 21);
  image->FilterEdge(5, image::WHITE_BINARY);
}

image::Clip Slicer::SearchFingerprint(std::unique_ptr<image::Image>& image, int last, int vec_sum_black[],
                                      int vec_sum_black_max[]) {
  int h_max = 0, h_max_val = 0;
  image::Clip coord;

  // Set h_max
  bool start = false;
  int end = image->width();
  for(int k = last; k < end; k++) {
    if(!start) {
      if(vec_sum_black_max[k] > 20) {
        start = true;
        end = k + MAXIMUM_SIZE_WIDTH;
        h_max_val = vec_sum_black_max[k];
        h_max = k;
      }
    }
    else {
      if(vec_sum_black_max[k] == 0)
        break;

      if (vec_sum_black_max[k] > h_max_val) {
        h_max_val = vec_sum_black_max[k];
        h_max = k;
      }
    }
  }

  if(h_max_val == 0)
    return coord;

  // Set left
  int k;
  for(k = h_max; k >= last; k--) {
    if (vec_sum_black[k]<10) {
      k--;
      break;
    }
  }
  coord.set_left(k);

  // Set right
  for(k = h_max; k < image->width() ; k++) {
    if (vec_sum_black[k] < 10) {
      k++;
      break;
    }
  }
  coord.set_right(k);

  // Check join
  if(coord.width() > MAXIMUM_SIZE_WIDTH) {
    int min_pos = 0;
    int min_val = image->height();
    for(k = coord.left() + 30; k < coord.left() + MAXIMUM_SIZE_WIDTH; k++) {
      if(vec_sum_black[k] < min_val)
      {
        min_val = vec_sum_black[k];
        min_pos = k;
      }
    }
    coord.set_right(min_pos);
  }

  //SearchHeight(image, coord);
  int sum;
  int *vec;

  int init = image->height() * LIMIT_SEARCH_INIT;
  int fin  = image->height() * LIMIT_SEARCH_FIN;

  // Accum horizontal BLACK and save to vector
  vec = (int *)malloc(sizeof(int)*image->height());
  for(int y = 0; y < image->height(); y++) {
    sum = 0;
    for(int x = coord.left() ; x < coord.right(); x++) {
      sum += !image->pixel(y * image->width() + x);
    }
    vec[y] = sum;
  }

  // Find max value position of vector
  int max = 0, max_val = 0;
  for(int y = init; y < fin; y++) {
    if(vec[y] > max_val) {
      max_val = vec[y];
      max = y;
    }
  }

  // Find top
  for(k = max; k >= 0 ; --k) {
    if (vec[k] < 10) {
      --k;
      break;
    }
  }
  coord.set_top(k);

  // Find bottom coord
  for(k = max; k < image->height() ; ++k) {
    if (vec[k]<10) {
      ++k;
      break;
    }
  }
  coord.set_bottom(k);
  free(vec);

  return coord;
}

std::vector<image::Clip> Slicer::SearchFingerprints(std::unique_ptr<image::Image>& image) {
  int *vec_sum_black, *vec_sum_black_max;
  std::vector<image::Clip> coord_list;

  vec_sum_black     = (int *)malloc(sizeof(int) * image->width());
  vec_sum_black_max = (int *)malloc(sizeof(int) * image->width());

  for(int k=0; k < image->width(); k++)
    vec_sum_black[k] = 0;

  for(int k = 0; k < image->length(); k++)
    vec_sum_black[k%image->width()] += !image->pixel(k);

  for(int k = 0; k < image->width(); k++)
    vec_sum_black_max[k] = vec_sum_black[k];

  int last = 0;
  for(int k = 0; k < _fp_number/2; k++) {
    auto coord = SearchFingerprint(image, last, vec_sum_black, vec_sum_black_max);

    for(int i = 0; i <= coord.right(); i++) {
      vec_sum_black_max[i] = 0;
    }
    coord_list.push_back(coord);
    last = coord.right();
  }

  free(vec_sum_black);
  free(vec_sum_black_max);

  return coord_list;
}