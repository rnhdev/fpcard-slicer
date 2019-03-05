#include <string>
#include <memory>
#include <image.h>
#include <iostream>
#include <slicer.h>
#include <parse_arguments.h>

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
using namespace fpcard_slicer::application;

std::string GetName(std::string name) {
  int start = name.rfind('/')+1;
  int len = name.rfind('.') - start;
  return name.substr(start, len);
}

int main(int argc, char** argv) {
  SlicerConfig config;

  if(!ParseArguments(argc, argv, config)) {
    return -1;
  }

  //TODO: add slicer.ini
  Slicer slicer(10, 1, Mode::General, 20);

  for(auto &source : config.source_list()) {
    std::string output_path = config.destination() + "/" + GetName(source);
    system(std::string("mkdir -p " + output_path).c_str());
    std::cout << output_path << " ... ";

    auto fpcard = std::make_shared<Image>(source);
    auto clip_list = slicer.CalculateSlice(fpcard, config.demo_mode()?output_path:"");

    //Save result
    int index = 0;
    for(auto &clip : clip_list) {
      std::string out = output_path + "/fp_" + std::to_string(index++) + "." + config.output_format();
      (fpcard->Cut(clip))->Save(out, config.output_quality());
    }

    std::cout << " OK" << endl;
  }

  return 0;
}
