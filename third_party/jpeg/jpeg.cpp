#include "jpeg.h"

#include <jpeglib.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace jpeg {
  void load_file(const std::string& filename, std::vector<unsigned char>&out, unsigned& w, unsigned& h) {
    auto dt = []( ::jpeg_decompress_struct *ds )
    {
      ::jpeg_destroy_decompress( ds );
    };
    std::unique_ptr<::jpeg_decompress_struct, decltype(dt)> decompress_info(
      new ::jpeg_decompress_struct,
      dt
    );

    auto error = std::make_shared<::jpeg_error_mgr>();

    auto fdt = []( FILE* fp )
    {
      fclose( fp );
    };
    std::unique_ptr<FILE, decltype(fdt)> infile(
      fopen( filename.c_str(), "rb" ),
      fdt
    );
    if ( infile.get() == NULL )
    {
      throw std::runtime_error( "Could not open " + filename );
    }

    decompress_info->err = ::jpeg_std_error( error.get() );

    ::jpeg_create_decompress( decompress_info.get() );

    ::jpeg_stdio_src( decompress_info.get(), infile.get() );

    int rc = ::jpeg_read_header( decompress_info.get(), TRUE );
    if (rc != 1)
    {
      throw std::runtime_error(
        "File does not seem to be a normal JPEG"
      );
    }
    ::jpeg_start_decompress( decompress_info.get() );

    w = decompress_info->output_width;
    h = decompress_info->output_height;
    auto pixelsize = decompress_info->output_components;
    auto colour_space = decompress_info->out_color_space;

    if(colour_space != JCS_GRAYSCALE)
      throw std::runtime_error("Only support grayscale color space");

    size_t row_stride = w * pixelsize;

    out.clear();
    while ( decompress_info->output_scanline < h )
    {
      std::vector<uint8_t> vec(row_stride);
      uint8_t* p = vec.data();
      ::jpeg_read_scanlines( decompress_info.get(), &p, 1 );
      out.insert(out.end(), &p[0], &p[row_stride]);
    }
    ::jpeg_finish_decompress( decompress_info.get() );
  }

  void save_file(const std::string& filename, std::vector<unsigned char>in, unsigned w, unsigned h, int quality ) {
    if (quality < 0) quality = 0;
    if (quality > 100) quality = 100;

    FILE* outfile = fopen( filename.c_str(), "wb" );
    if ( outfile == NULL )
    {
      throw std::runtime_error(
        "Could not open " + filename + " for writing"
      );
    }

    auto dt = []( ::jpeg_compress_struct *cs )
    {
      ::jpeg_destroy_compress( cs );
    };
    std::unique_ptr<::jpeg_compress_struct, decltype(dt)> compress_info(
      new ::jpeg_compress_struct,
      dt );
    ::jpeg_create_compress( compress_info.get() );
    ::jpeg_stdio_dest( compress_info.get(), outfile);
    compress_info->image_width = w;
    compress_info->image_height = h;
    compress_info->input_components = 1;
    compress_info->in_color_space = static_cast<::J_COLOR_SPACE>( JCS_GRAYSCALE );
    auto error = std::make_shared<::jpeg_error_mgr>();
    compress_info->err = ::jpeg_std_error( error.get() );
    ::jpeg_set_defaults( compress_info.get() );
    ::jpeg_set_quality( compress_info.get(), quality, TRUE );
    ::jpeg_start_compress( compress_info.get(), TRUE);

    for(int line =0; line <h; ++line) {
      std::vector<unsigned char> vec_line(in.begin()+(w*line), in.begin()+(w*line) + w);
      ::JSAMPROW rowPtr[1];
      rowPtr[0] = const_cast<::JSAMPROW>( vec_line.data() );
      ::jpeg_write_scanlines(
        compress_info.get(),
        rowPtr,
        1
      );
    }
    ::jpeg_finish_compress( compress_info.get() );
    fclose( outfile );
  }
}
