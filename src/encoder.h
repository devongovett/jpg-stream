extern "C" {
#include <stdio.h>
#include <jpeglib.h>
}

#include <vector>
#include <string>

#include <emscripten/val.h>
#include <emscripten/bind.h>

using namespace emscripten;

class JPEGEncoder {
public:
  JPEGEncoder();
  ~JPEGEncoder();
  void encode(uint8_t *buffer, size_t length);
  void emptyOutput();
  void end();
  void encodeStr(std::string buf) {
    encode((uint8_t *) buf.data(), buf.size());
  }
  
  int getWidth() const {
    return enc.image_width;
  }
  
  void setWidth(int w) {
    enc.image_width = w;
  }
  
  int getHeight() const {
    return enc.image_height;
  }
  
  void setHeight(int h) {
    enc.image_height = h;
  }
  
  int getQuality() const {
    return quality;
  }
  
  void setQuality(int q) {
    quality = q;
  }
  
  std::string getColorSpace() const {
    switch (enc.in_color_space) {
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
  
  void setColorSpace(std::string cs) {
    if (cs.compare("gray") == 0) {
      enc.input_components = 1;
      enc.in_color_space = JCS_GRAYSCALE;
    } else if (cs.compare("cmyk") == 0) {
      enc.input_components = 4;
      enc.in_color_space = JCS_CMYK;
    } else {
      enc.input_components = 3;
      enc.in_color_space = JCS_RGB;
    }
    
    scanlineLength = enc.image_width * enc.input_components;
  }
  
  val getCallback() const {
    return callback;
  }

  void setCallback(val cb) {
    callback = cb;
  }
  
  void error(char *message);

private:
  struct jpeg_compress_struct enc;
  struct jpeg_error_mgr err;
    
  int quality;
  std::string colorSpace;
  val callback;
  bool decoding;
  int scanlineLength;
  std::vector<uint8_t> buf;
  uint8_t *output;
};

EMSCRIPTEN_BINDINGS(encoder) {
  class_<JPEGEncoder>("JPEGEncoder")
    .constructor()
    .property("callback", &JPEGEncoder::getCallback, &JPEGEncoder::setCallback)
    .property("width", &JPEGEncoder::getWidth, &JPEGEncoder::setWidth)
    .property("height", &JPEGEncoder::getHeight, &JPEGEncoder::setHeight)
    .property("quality", &JPEGEncoder::getQuality, &JPEGEncoder::setQuality)
    .property("colorSpace", &JPEGEncoder::getColorSpace, &JPEGEncoder::setColorSpace)
    .function("encode", &JPEGEncoder::encodeStr)
    .function("end", &JPEGEncoder::end)
    ;
}
