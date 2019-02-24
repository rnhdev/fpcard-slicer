//
// Created by nico on 18/02/19.
//

#include <string>
#include <memory>
#include <image.h>
#include <iostream>
#include <slicer.h>

#ifdef __cplusplus
extern "C" {
#endif
void print_usage(char *) {
}
int debug=0;
#ifdef __cplusplus
} // extern "C"
#endif

using namespace std;
using namespace fpcard_slicer::image;
using namespace fpcard_slicer::slicer;

int main()
{
  string rootpath = "../test/";
  string name = "A002";
  string cardpath = rootpath + name + ".JPG";

  system(string("mkdir -p " + rootpath + name).c_str());

  unique_ptr<Image> image (new Image());

  try {
    image->LoadFromFile(cardpath);
  }
  catch (std::exception& e) {
    cerr <<  e.what() << std::endl;
    return 0;
  }

  Slicer slicer(10, 10, Mode::General, 20);

  try {
    auto coord_list = slicer.CalculateSlice(image);

    for(uint k =0; k < coord_list.size(); k++) {
      (image->Cut(coord_list[k]))->Save(rootpath + name + "/" + to_string(k) + ".jpg", Format::JPEG);
    }
  }
  catch(std::exception& e) {
    cerr <<  e.what() << std::endl;
    return 0;
  }
}
