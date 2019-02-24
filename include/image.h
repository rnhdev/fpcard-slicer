#ifndef FP_CARDSLICER_IMAGE_H
#define FP_CARDSLICER_IMAGE_H
#include <string>
#include <memory>

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
    struct Size {
      int width;
      int height;
    };

    class Clip {
    public:
      Clip():
        _left(0),_right(0),_top(0),_bottom(0){}
      Clip(int left, int right, int top, int bottom):
        _left(left),_right(right),_top(top),_bottom(bottom){}
      inline const int left() {
        return _left;
      }
      inline const int right() {
        return _right;
      }
      inline const int top() {
        return _top;
      }
      inline const int bottom() {
        return _bottom;
      }
      inline void set_left(int value) {
        _left = value;
      }
      inline void set_right(int value) {
        _right = value;
      }
      inline void set_top(int value) {
        _top = value;
      }
      inline void set_bottom(int value) {
        _bottom = value;
      }
      inline const int width() {
        return abs(_right - _left);
      }
      inline const int height() {
        return abs(_top - _bottom);
      }
      inline const Size size() {
        return {width(), height()};
      }
      inline const int length() {
        return width()*height();
      }
      inline const void Scale(int factor) {
        _left *= factor;
        _right *= factor;
        _top *= factor;
        _bottom *= factor;
      }
      inline const void Expand(int width, int height) {
        _left += width;
        _right += width;
        _top += height;
        _bottom += height;
      }
    private:
      int _left, _right, _top, _bottom;
    };

    enum Format {
      JPEG,
      WSQ_5,
      WSQ_15,
      RAW
    };

    class Image {
    public:
      Image() = default;
      Image(Size size, ColorMode mode):
        _size(size), _mode(mode){
        _data = (Pixel *)malloc((size_t)length());
      }
      Image(Pixel* data, const Size size):
        _data(data), _size(size), _mode(Grayscale){
      }
      Image(Pixel* data, const Size size, ColorMode mode):
        _data(data), _size(size), _mode(mode){
      }
      ~Image();
      void LoadFromFile(const std::string&);
      inline void Save(const std::string& path, Format format) {
        Save(path, format, 100);
      }
      void Save(const std::string&, Format, int);
      std::unique_ptr<Image> Scale(float);
      std::unique_ptr<Image> Cut(Clip);
      void FilterBinarize(int);
      void FilterMean(int bw, int bh);
      void FilterRemoveVerticalLines(int, int);
      void FilterRemoveHorizontalLines(int, int);
      void FilterRemoveVerticalLines2(int, int, int);
      void FilterEdge(int, Pixel);
      void FilterRemoveHorizontalLines2(int, int);
      inline const Pixel white() {
        return _mode==Grayscale?WHITE_GRAYSCALE:WHITE_BINARY;
      }
      inline const Pixel black() {
        return BLACK_COLOR;
      }
      inline const Size size() {
        return _size;
      }
      inline const int length() {
        return _size.width * _size.height;
      }

      inline const int width() {
        return _size.width;
      }
      inline const int height() {
        return _size.height;
      }
      inline const Pixel pixel(int index) {
        CheckRange(index);

        return _data[index];
      }
      inline const Pixel pixel(int x, int y) {
        return pixel(XY2Index(x, y));
      }
      void set_pixel(int x, int y, const Pixel value) {
        set_pixel(XY2Index(x, y),value);
      }
      void set_pixel(int index, const Pixel value) {
        CheckRange(index);

        _data[index] = value;
      }
      inline const int SumVertical(int x) {
        int sum = 0;
        for (int y = 0; y < height(); ++y)
          sum += _data[XY2Index(x,y)];
        return sum;
      }
      inline const int SumHorizontal(int y) {
        int sum = 0;
        for (int x = 0; x < width(); ++x)
          sum += _data[XY2Index(x,y)];
        return sum;
      }
    private:
      int _id = 0, _len = 0, _ppi = 0, _img_type = 0;
      Pixel* _data = nullptr;
      ColorMode _mode;
      Size _size = {};
      inline void CheckRange(int index) {
        if(index < 0 && index > length())
          throw std::invalid_argument("Out of range");
      }
      inline int XY2Index(int x, int y) {
        return y * width() + x;
      }
      inline void Clear() {
        if (_data != nullptr)
          free(_data);
      }
      inline void ToBinary() {
        for(int k = 0; k < length(); k++)
          _data[k] *= WHITE_GRAYSCALE;
      }
      inline void ToGrayscale() {
        for(int k = 0; k < length(); k++)
          _data[k] /= WHITE_GRAYSCALE;
      }
    };
  }// namespace image
}// namespace fpcard_slicer


#endif //FP_CARDSLICER_IMAGE_H
