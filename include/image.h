#ifndef FP_CARDSLICER_IMAGE_H
#define FP_CARDSLICER_IMAGE_H
#include <string>
#include <memory>
#include <vector>
#include <vector>
namespace fpcard_slicer {
  namespace image {
    typedef unsigned char Pixel;

    const Pixel BLACK_COLOR = 0;
    const Pixel WHITE_GRAYSCALE = 255;
    const Pixel WHITE_BINARY = 1;

    enum ColorMode {
      Grayscale,
      Binary
    };
    enum Format {
      JPEG,
      PNG,
      Other
    };
    struct Size {
      unsigned width;
      unsigned height;
    };

    class Clip {
    public:
      Clip():
        _left(0),_right(0),_top(0),_bottom(0){}
      Clip(unsigned left, unsigned right, unsigned top, unsigned bottom):
        _left(left),_right(right),_top(top),_bottom(bottom){}
      inline const unsigned left() {
        return _left;
      }
      inline const unsigned right() {
        return _right;
      }
      inline const unsigned top() {
        return _top;
      }
      inline const unsigned bottom() {
        return _bottom;
      }
      inline void set_left(unsigned value) {
        _left = value;
      }
      inline void set_right(unsigned value) {
        _right = value;
      }
      inline void set_top(unsigned value) {
        _top = value;
      }
      inline void set_bottom(unsigned value) {
        _bottom = value;
      }
      inline const unsigned width() {
        return abs((int)_right - (int)_left);
      }
      inline const unsigned height() {
        return abs((int)(_top - _bottom));
      }
      inline const Size size() {
        return {width(), height()};
      }
      inline const unsigned length() {
        return width()*height();
      }
      inline const void Scale(unsigned factor) {
        _left *= factor;
        _right *= factor;
        _top *= factor;
        _bottom *= factor;
      }
      inline const void Expand(unsigned width, unsigned height) {
        _left += width;
        _right += width;
        _top += height;
        _bottom += height;
      }
    private:
      unsigned _left, _right, _top, _bottom;
    };

    class Image {
    public:
      Image() = default;
      Image(const std::string&);
      Image(std::vector<Pixel> data, Size size): _data(data), _size(size), _mode(Grayscale) {}
      Image(std::vector<Pixel> data, Size size, ColorMode mode): _data(data), _size(size), _mode(mode) {}
      Image(std::vector<Pixel> data, int w, int h, ColorMode mode): _data(data), _size(Size{w,h}), _mode(mode) {}
      Image(Size size, ColorMode mode): _size(size), _mode(mode) {
        Clear();
        _data.resize(length());
      }
      ~Image();
      void Save(const std::string&);
      void Save(const std::string&, int);
      std::shared_ptr<Image> Scale(float);
      std::shared_ptr<Image> Cut(Clip);
      void ApplyBinarizedFilter(unsigned);
      void ApplyAverageFilter(unsigned bw, unsigned bh);
      void ApplyVerticalFilter(unsigned, unsigned);
      void ApplyHorizontalWhiteFilter(unsigned, unsigned);
      void ApplyEdgeFilter(unsigned, Pixel);
      void ApplyHorizontalBlackFilter(unsigned, unsigned);
      inline const Pixel white() {
        return _mode==Grayscale?WHITE_GRAYSCALE:WHITE_BINARY;
      }
      inline std::vector<Pixel>::iterator get() {
        return _data.begin();
      }
      inline std::vector<Pixel>::iterator end() {
        return _data.end();
      }
      inline const Pixel black() {
        return BLACK_COLOR;
      }
      inline const Size size() {
        return _size;
      }
      inline const unsigned length() {
        return _size.width * _size.height;
      }

      inline const unsigned width() {
        return _size.width;
      }
      inline const unsigned height() {
        return _size.height;
      }
      inline const Pixel pixel(unsigned index) {
        CheckRange(index);

        return _data[index];
      }
      inline const Pixel pixel(unsigned x, unsigned y) {
        return pixel(XY2Index(x, y));
      }
      void set_pixel(unsigned x, unsigned y, const Pixel value) {
        set_pixel(XY2Index(x, y),value);
      }
      void add_pixel(Pixel value) {
        _data.push_back(value);
      }
      void set_pixel(unsigned index, const Pixel value) {
        CheckRange(index);

        _data[index] = value;
      }
      inline const unsigned SumVertical(unsigned x) {
        unsigned sum = 0;
        for (unsigned y = 0; y < height(); ++y)
          sum += _data[XY2Index(x,y)];
        return sum;
      }
      inline const unsigned SumHorizontal(unsigned y) {
        unsigned sum = 0;
        for (unsigned x = 0; x < width(); ++x)
          sum += _data[XY2Index(x,y)];
        return sum;
      }
    private:
      int _id = 0, _len = 0, _ppi = 0, _img_type = 0;
      std::vector<Pixel> _data;
      ColorMode _mode;
      Size _size = {};
      Format extension(const std::string& file);
      void ReadPNG(const std::string&);
      void ReadJPEG(const std::string &);
      void SavePNG(const std::string&);
      void SaveJPEG(const std::string&, int);
      inline void CheckRange(unsigned index) {
        if(index < 0 && index > length())
          throw std::invalid_argument("Out of range");
      }
      inline unsigned XY2Index(unsigned x, unsigned y) {
        return y * width() + x;
      }
      inline void Clear() {
        _data.clear();
      }
      inline void ToBinary() {
        for(auto &pixel : _data) pixel*=WHITE_GRAYSCALE;
      }
      inline void ToGrayscale() {
        for(auto &pixel : _data) pixel/=WHITE_GRAYSCALE;
      }
    };
  }// namespace image
}// namespace fpcard_slicer


#endif //FP_CARDSLICER_IMAGE_H
