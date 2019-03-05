#ifndef FPCARD_SLICER_PARSE_ARGUMENTS_H
#define FPCARD_SLICER_PARSE_ARGUMENTS_H
namespace fpcard_slicer {
  namespace application {
    class SlicerConfig {
    public:
      SlicerConfig() = default;
      inline void set_source_list(std::vector<std::string> values) {
        _source_list = values;
      }
      inline void set_destination(const std::string& value) {
        _destination = value;
      }
      inline void set_output_format(const std::string& value) {
        _output_format = value;
      }
      inline void set_output_quality(int value) {
        _output_quality = value;
      }
      inline void set_demo_mode(bool value) {
        _demo_mode = value;
      }
      inline std::vector<std::string> source_list() const {
        return _source_list;
      }
      inline const std::string& destination() const {
        return _destination;
      }
      inline const std::string& output_format() const {
        return _output_format;
      }
      inline int output_quality() {
        return _output_quality;
      }
      inline bool demo_mode() {
        return _demo_mode;
      }
    private:
      int _output_quality;
      bool _demo_mode;
      std::string _destination, _output_format;
      std::vector<std::string> _source_list;
    };
    bool ParseArguments(int argc, char** argv, SlicerConfig&);
  }
}
#endif //FPCARD_SLICER_PARSE_ARGUMENTS_H
