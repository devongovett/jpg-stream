extern "C" {
#include <stdio.h>
#include <jpeglib.h>
}

#include <vector>
#include <string>
#include <emscripten/val.h>
#include <emscripten/bind.h>

using namespace emscripten;

enum JPEGState {
  JPEG_HEADER,
  JPEG_START_DECOMPRESS,
  JPEG_DECOMPRESS,
  JPEG_DONE,
  JPEG_ERROR
};

class JPEGDecoder {
public:
  JPEGDecoder();
  ~JPEGDecoder();
  void setDesiredSize(int dw, int dh) {
    desiredWidth = dw;
    desiredHeight = dh;
  }
  void skipBytes(long numBytes);
  bool decode(uint8_t *buffer, size_t length);
  bool decodeStr(std::string buf) {
    return decode((uint8_t *) buf.data(), buf.size());
  }
  
  int getWidth() const {
    return outputWidth;
  }
  
  int getHeight() const {
    return outputHeight;
  }
  
  val getCallback() const {
    return callback;
  }
  
  void setCallback(val cb) {
    callback = cb;
  }
  
  std::string getColorSpace() const {    
    switch (dec.out_color_space) {
      case JCS_RGB:
        return std::string("rgb");
        
      case JCS_GRAYSCALE:
        return std::string("gray");
        
      case JCS_CMYK:
        return std::string("cmyk");
        
      default:
        return std::string("rgb");
    }
  }
  
  void error(char *message);
    
private:
  bool readHeader();
  bool startDecompress();
  void findExif();
  bool decompress();
  
  jpeg_error_mgr err;
  jpeg_decompress_struct dec;
  JPEGState state;
  int imageWidth, imageHeight;
  int desiredWidth, desiredHeight;
  int outputWidth, outputHeight;
  uint8_t *output;
  int bytesToSkip;
  std::vector<uint8_t> data;
  val callback;
};

EMSCRIPTEN_BINDINGS(decoder) {
  class_<JPEGDecoder>("JPEGDecoder")
    .constructor()
    .property("callback", &JPEGDecoder::getCallback, &JPEGDecoder::setCallback)
    .function("setDesiredSize", &JPEGDecoder::setDesiredSize)
    .property("width", &JPEGDecoder::getWidth)
    .property("height", &JPEGDecoder::getHeight)
    .property("colorSpace", &JPEGDecoder::getColorSpace)
    .function("decode", &JPEGDecoder::decodeStr)
    ;
}
