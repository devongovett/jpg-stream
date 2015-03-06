#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include "decoder.h"

#define EXIF_MARKER (JPEG_APP0 + 1)

void init_source(j_decompress_ptr) {}
void term_source(j_decompress_ptr jd) {}

void skip_input_data(j_decompress_ptr jd, long numBytes) {
  JPEGDecoder *decoder = static_cast<JPEGDecoder *>(jd->client_data);
  decoder->skipBytes(numBytes);
}

boolean fill_input_buffer(j_decompress_ptr) {
  return FALSE;
}

static void error_exit(j_common_ptr cinfo) {
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);
  
  JPEGDecoder *decoder = static_cast<JPEGDecoder *>(cinfo->client_data);
  decoder->error(buffer);
}

JPEGDecoder::JPEGDecoder() : callback(val::undefined()) {
  dec.err = jpeg_std_error(&err);
  err.error_exit = error_exit;
  jpeg_create_decompress(&dec);
  
  jpeg_source_mgr *src = (jpeg_source_mgr *) calloc(1, sizeof(jpeg_source_mgr));
  dec.src = src;
  dec.client_data = this;
  
  src->init_source = init_source;
  src->fill_input_buffer = fill_input_buffer;
  src->skip_input_data = skip_input_data;
  src->resync_to_restart = jpeg_resync_to_restart;
  src->term_source = term_source;
  
  // Keep APP1 blocks, for obtaining exif data.
  jpeg_save_markers(&dec, EXIF_MARKER, 0xFFFF);
  
  imageWidth = 0;
  imageHeight = 0;
  desiredWidth = 0;
  desiredHeight = 0;
  outputWidth = 0;
  outputHeight = 0;
  
  bytesToSkip = 0;
  output = NULL;
  
  state = JPEG_HEADER;
}

JPEGDecoder::~JPEGDecoder() {
  free(dec.src);
  jpeg_destroy_decompress(&dec);
  if (output)
    free(output);
}

void JPEGDecoder::skipBytes(long numBytes) {
  long skip = std::min(numBytes, (long) dec.src->bytes_in_buffer);
  dec.src->bytes_in_buffer -= (size_t) skip;
  dec.src->next_input_byte += skip;
  bytesToSkip = std::max(numBytes - skip, 0L);
}

bool JPEGDecoder::decode(uint8_t *buffer, size_t length) {
  int offset = data.size() - dec.src->bytes_in_buffer;
  
  data.erase(data.begin(), data.begin() + offset);
  data.insert(data.end(), buffer, buffer + length);
  
  dec.src->bytes_in_buffer += length;
  dec.src->next_input_byte = &data[0];
  
  if (bytesToSkip)
    skipBytes(bytesToSkip);
    
  switch (state) {
    case JPEG_HEADER:
      if (!readHeader())
        return false;
      
    case JPEG_START_DECOMPRESS:
      if (!startDecompress())
        return false;
      
    case JPEG_DECOMPRESS:
      if (!decompress())
        return false;
      
    case JPEG_DONE:
      return jpeg_finish_decompress(&dec);
      
    case JPEG_ERROR:
      return false;
  }
  
  return true;
}
  
bool JPEGDecoder::readHeader() {
  if (jpeg_read_header(&dec, TRUE) == JPEG_SUSPENDED)
    return false; // I/O suspension.
  
  imageWidth = dec.image_width;
  imageHeight = dec.image_height;
  callback(std::string("inputSize"));
  
  state = JPEG_START_DECOMPRESS;
  return true;
}

bool JPEGDecoder::startDecompress() {
  switch (dec.jpeg_color_space) {
    case JCS_YCbCr:
    case JCS_RGB:
      dec.out_color_space = JCS_RGB;
      break;
      
    case JCS_GRAYSCALE:
      dec.out_color_space = JCS_GRAYSCALE;
      break;
      
    case JCS_CMYK:
    case JCS_YCCK:
      dec.out_color_space = JCS_CMYK;
      break;
      
    default:
      callback(std::string("error"), std::string("Unknown JPEG color space"));
      return false;
  }
  
  // Calculate scale so we only decode what we need
  if (desiredWidth && desiredHeight) {
    int wdeg = imageWidth / desiredWidth;
    int hdeg = imageHeight / desiredHeight;
    dec.scale_num = 1;
    dec.scale_denom = std::max(1, std::min(std::min(wdeg, hdeg), 8));
  }
  
  jpeg_calc_output_dimensions(&dec);
  outputWidth = dec.output_width;
  outputHeight = dec.output_height;
  callback(std::string("outputSize"));
  
  findExif();
  
  if (!jpeg_start_decompress(&dec))
    return false; // I/O suspension.
  
  // Allocate output buffer for a single scanline
  output = (uint8_t *) malloc(dec.output_width * dec.output_components);
  
  state = JPEG_DECOMPRESS;
  return true;
}

void JPEGDecoder::findExif() {
  for (jpeg_saved_marker_ptr marker = dec.marker_list; marker; marker = marker->next) {
    if (marker->marker == EXIF_MARKER && marker->data_length >= 14 && !memcmp(marker->data, "Exif", 5)) {
      callback(std::string("exif"), (unsigned int) marker->data, (size_t) marker->data_length);
    }
  }
}

bool JPEGDecoder::decompress() {
  while (dec.output_scanline < dec.output_height) {
    if (jpeg_read_scanlines(&dec, &output, 1) != 1)
      return false;
    
    callback(std::string("scanline"), (unsigned int) output, (size_t) (dec.output_width * dec.output_components));
  }
  
  callback(std::string("end"));
  
  state = JPEG_DONE;
  return true;
}

void JPEGDecoder::error(char *message) {
  state = JPEG_ERROR;
  callback(std::string("error"), std::string(message));
}
