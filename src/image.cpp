#include <cstdlib>
#include <stdexcept>

#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _JPEGL_H
#include <jpegl.h>
#endif

#ifndef _LFS_H
#include <lfs.h>
#endif

#ifndef _IMG_IO_H
#include <img_io.h>
#endif

#ifndef _IMGDECOD_H
#include <imgdecod.h>
#endif

#include <imgavg.h>

#ifdef __cplusplus
} // extern "C"
#endif

using namespace fpcard_slicer::image;

void Image::LoadFromFile(const std::string &file) {
  Clear();

  int ret = read_and_decode_grayscale_image((char *)file.c_str(), &_img_type,
                                            &_data, &_len, &_size.width,
                                            &_size.height, &_id, &_ppi);

  if(ret) {
    throw std::invalid_argument("Image reading failed. Error code: " +
                                std::to_string(ret));
  }

  _mode = Grayscale;
}

void Image::Save(const std::string& file,  Format format, int qlty) {
  Pixel *cdata;
  int ret, clen, depth = 8;
  auto comment_text = (char *) nullptr;

  if(_mode == Binary)
    ToBinary();

  switch (format) {
    case JPEG:
      ret = jpegb_encode_mem(&cdata, &clen, qlty, _data, _size.width, _size.height, depth, _ppi, comment_text);
      break;
    case WSQ_5:
      ret = wsq_encode_mem(&cdata, &clen, 2.25, _data, _size.width, _size.height, depth, _ppi, comment_text);
      break;
    case WSQ_15:
      ret = wsq_encode_mem(&cdata, &clen, 0.75, _data, _size.width, _size.height, depth, _ppi, comment_text);
    default:
      clen = _size.width*_size.height;
      ret = malloc_uchar_ret(&cdata,  clen, (char *)"write_parsefings cdata");

      break;
  }

  if(!ret) {
    ret = write_raw_from_memsize((char *)file.c_str(), cdata, clen);
    free(cdata);

    if(_mode == Binary)
      ToGrayscale();
  }

  if(ret) {
    throw std::logic_error( "Image saving failed. Error code: " + std::to_string(ret));
  }
}

std::unique_ptr<Image> Image::Scale(float factor) {
  int w, h;
  Pixel *rdata;

  average_blk(_data, _size.width, _size.height, factor, factor, &rdata, &w, &h);

  return std::unique_ptr<Image>(new Image(rdata, {w, h}));
}

void Image::FilterBinarize(int umbral) {
  const int max_black = 30;
  int mean = 0, count = 0;

  //average calc
  for(int k = 0; k < length(); ++k) {
    if(pixel(k) > max_black) {
      mean += pixel(k);
      count++;
    }
  }
  mean = (count != 0)? mean/count : 0;

  for(int k = 0; k < length(); ++k) {
    Pixel value = (pixel(k) > (mean + umbral)) ? WHITE_BINARY:black();

    set_pixel(k, value);
  }

  _mode = Binary;
}

std::unique_ptr<Image> Image::Cut(Clip clip) {
  std::unique_ptr<Image> image(new Image({clip.width(), clip.height()}, _mode));

  int rindex = 0;
  for(int y = clip.top(); y < clip.bottom(); ++y ) {
    for (int x = clip.left(); x < clip.right(); ++x) {
      image->set_pixel(rindex, pixel(x, y));
      ++rindex;
    }
  }

  return image;
}

void Image::FilterMean(int bw, int bh) {
  int x, y, xx, yy;
  int sum;
  Pixel *filt;

  filt =(Pixel*)malloc((size_t)length());
  for(x = 0; x < length(); ++x) filt[x] = (_data)[x];

  for(y = 0; y < height() ; ++y) {
    for(x = 0; x < width() ; ++x) {
      //Check limits

      if((y-bh/2) >= 0  && (y+bh/2) < height() &&
         (x-bw/2) >= 0 && (x+bw/2) < width())	{
        //Calculate block average
        sum = 0;
        double index = 0;
        for(yy = y-bh/2; yy <= y+bh/2; ++yy) {
          for (xx = x - bw / 2; xx <= x + bw / 2; ++xx) {
            ++index;
            sum += _data[(yy * width()) + xx];
          }
        }

        filt[y*width() + x] = (sum/index > 0.5)? white():black();
      }
    }
  }

  //Set result
  for(x = 0; x < length(); ++x) {
    _data[x] = filt[x];
  }

  free(filt);
}

void Image::FilterRemoveVerticalLines(int umbral, int lw) {
  for(int x = 0; x < lw; ++x) {
    // Vertical sum
    int sum = 0;
    for(int y = 0; y < height(); ++y) {
      if (!_data[(y * width()) + x])
        sum++; // Only black sum
      sum += _data[y * width() + x];
    }

    //Remove line by umbral
    if(sum <= height()*(umbral/100))
      for(int y = 0; y < height(); ++y)
        _data[y*width() + x] = white();
  }
}

void Image::FilterRemoveHorizontalLines(int bw, int bh) {
  Pixel *clean;

  clean = (Pixel*)malloc((size_t)length());
  for(int x = 0; x < length(); ++x)
    clean[x] = _data[x];

  for(int y = bh/2; y < height()-bh/2 - 1; ++y) {
    for(int x = bw/2; x < width()-bw/2 - 1; ++x) {
      int lsum = 0;
      int rsum = 0;
      //Horizontal sum
      for(int yy = y-bh/2; yy <= y+bh/2; ++yy) {
        lsum += _data[ (yy*width()) + x-bw/2 ];
        rsum += _data[ (yy*width()) + x+bw/2 ];
      }

      //Check by umbral
      if((lsum + rsum) == bh*2) {
        //Remove block
        for(int yy = y-bh/2; yy <= y+bh/2; ++yy)
          for(int xx = x-bw/2; xx <= x+bw/2; ++xx)
            clean[(yy*width()) + xx] = white();
      }
    }
  }

  //Set result
  for(int k = 0; k < length(); k++)
    _data[k] = clean[k];
  free(clean);
}

void Image::FilterRemoveVerticalLines2(int bw, int bh, int limit) {
  auto clean = (Pixel*)malloc((size_t)length());
  
  for(int x = 0; x < length(); ++x)
    clean[x] = _data[x];

  // Check limits
  if(limit < bh/2 || limit >= height() )
    limit = bh/2;

  for(int y = limit; y < height()-bh/2 - 1; ++y) {
    for(int x = bw/2; x < width()-bw/2 - 1; ++x)	{
      //Vertical sum
      int tsum = 0;
      int bsum = 0;
      for(int xx = x-bw/2; xx <= x+bw/2; ++xx) {
        tsum += _data[((y-bh/2)*width()) + xx];
        bsum += _data[((y+bh/2)*width()) + xx];
      }

      // Verify complete white condicion
      if((tsum+bsum) == bw*2)	{
        // Clean entire block
        for(int yy = y-bh/2; yy <= y+bh/2; ++yy)
          for(int xx = x-bw/2; xx <= x+bw/2; ++xx)
            clean[ (yy*width()) + xx ] = 1;
      }

    }
  }

  //Set result
  for(int x = 0; x < length(); ++x)
    _data[x] = clean[x];
  free(clean);
}

void Image::FilterEdge(int size, Pixel color) {
  //Set left edge
  for(int y = 0; y < height(); ++y) {
    for (int x = 0; x < size; ++x)
      _data[y * width() + x] = color;
  }

  //Set edge
  for(int y = 0; y < height(); ++y) {
    for (int x = width() - size; x < width(); ++x)
      _data[y * width() + x] = color;
  }

  //Set top edge
  for(int x = 0; x < width(); ++x) {
    for (int y = 0; y < size; ++y)
      _data[y * width() + x] = color;
  }

  //Set bottom edge
  for(int x = 0; x < width(); ++x) {
    for (int y = height() - size; y < height(); ++y)
      _data[y * width() + x] = color;
  }
}

void Image::FilterRemoveHorizontalLines2(int bw, int bh) {
  auto rec = (Pixel*)malloc((size_t)length());
  for(int x = 0; x < length(); ++x)
    rec[x] = _data[x];

  //Loop with each poin
  for(int y = bh/2; y < height()-bh/2 - 1; ++y) {
    for(int x = bw/2; x < width()-bw/2 - 1; ++x) {
      //Horizontal sum
      int tsum = 0;
      int bsum = 0;
      for(int xx = x-bw/2; xx <= x+bw/2; ++xx) {
        tsum += !_data[((y-bh/2)*width()) + xx];
        bsum += !_data[((y+bh/2)*width()) + xx];
      }

      //Check condition
      if((tsum+bsum) == bw*2)	{
        //Apply filter
        for(int yy = y-bh/2; yy <= y+bh/2; ++yy)
          for(int xx = x-bw/2; xx <= x+bw/2; ++xx)
            rec[(yy*width()) + xx] = black();
      }

    }
  }

  for(int x = 0; x < length(); ++x)
    _data[x] = rec[x];
  free(rec);
}

Image::~Image() {
  Clear();
}