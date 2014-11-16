#include <stdlib.h>
#include "encoder.h"

#define BUF_SIZE 8192

void initDestination(jpeg_compress_struct *enc) {}
void termDestination(jpeg_compress_struct *enc) {}

boolean emptyOutputBuffer(jpeg_compress_struct *enc) {
  JPEGEncoder *encoder = static_cast<JPEGEncoder *>(enc->client_data);
  encoder->emptyOutput();  
  return TRUE;
}

static void error_exit(j_common_ptr cinfo) {
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message) (cinfo, buffer);
  
  JPEGEncoder *encoder = static_cast<JPEGEncoder *>(cinfo->client_data);
  encoder->error(buffer);
}

JPEGEncoder::JPEGEncoder() : callback(val::undefined()) {
  enc.err = jpeg_std_error(&err);
  err.error_exit = error_exit;
  jpeg_create_compress(&enc);
  
  enc.image_width = 0;
  enc.image_height = 0;
  enc.input_components = 3;
  enc.in_color_space = JCS_RGB;
  
  // set up the destination manager
  jpeg_destination_mgr *dest = (jpeg_destination_mgr *) calloc(1, sizeof(jpeg_destination_mgr));
  enc.dest = dest;
  enc.client_data = this;
  
  dest->init_destination = initDestination;
  dest->empty_output_buffer = emptyOutputBuffer;
  dest->term_destination = termDestination;
  
  output = (uint8_t *) malloc(BUF_SIZE);
  dest->next_output_byte = output;
  dest->free_in_buffer = BUF_SIZE;
    
  quality = 100;
  decoding = false;
  scanlineLength = 0;
  buf.resize(0);
}

JPEGEncoder::~JPEGEncoder() {
  free(enc.dest);
  jpeg_destroy_compress(&enc);
  if (output)
    free(output);
}

void JPEGEncoder::encode(uint8_t *buffer, size_t length) {
  if (!decoding) {
    jpeg_set_defaults(&enc);
    jpeg_set_quality(&enc, quality, TRUE);
    jpeg_start_compress(&enc, TRUE);
    decoding = true;
  }
  
  buf.insert(buf.end(), buffer, buffer + length);
  uint8_t *data = &buf[0];
  
  while (length >= scanlineLength) {
    jpeg_write_scanlines(&enc, &data, 1);
    data += scanlineLength;
    length -= scanlineLength;
  }
  
  buf.erase(buf.begin(), buf.end() - length);
}

void JPEGEncoder::emptyOutput() {
  callback(std::string("data"), (unsigned int) output, (size_t) BUF_SIZE);
  enc.dest->next_output_byte = output;
  enc.dest->free_in_buffer = BUF_SIZE;
}

void JPEGEncoder::end() {    
  jpeg_finish_compress(&enc);
  
  size_t remaining = BUF_SIZE - enc.dest->free_in_buffer;
  if (remaining > 0)
    callback(std::string("data"), (unsigned int) output, remaining);
}

void JPEGEncoder::error(char *message) {
  callback(std::string("error"), std::string(message));
}
