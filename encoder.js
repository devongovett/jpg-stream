var jpeg = require('./build/jpeg');
var Encoder = jpeg.JPEGEncoder;
var util = require('util');
var PixelStream = require('pixel-stream');

function JPEGEncoder(width, height, opts) {
  PixelStream.apply(this, arguments);
  if (typeof width === 'object')
    opts = width;
  
  this.encoder = new Encoder;
  this.ended = false;
        
  var self = this;
  this.encoder.callback = function(type, ptr, len) {
    switch (type) {
      case 'data':
        self.push(new Buffer(jpeg.HEAPU8.subarray(ptr, ptr + len)));
        break;
        
      case 'error':
        self.ended = true;
        self.encoder.delete();
        self.emit('error', new Error(ptr));
        break;
    }
  };
}

util.inherits(JPEGEncoder, PixelStream);

JPEGEncoder.prototype.supportedColorSpaces = ['rgb', 'gray', 'cmyk'];

JPEGEncoder.prototype._start = function(done) {
  this.encoder.width = this.format.width;
  this.encoder.height = this.format.height;
  this.encoder.colorSpace = this.format.colorSpace;
  this.encoder.quality = this.format.quality || 100;
  done();
};

JPEGEncoder.prototype._writePixels = function(data, done) {
  if (!this.ended) {
    var buf = data instanceof Uint8Array ? data : new Uint8Array(data);
    this.encoder.encode(buf);
  }
  
  done();
};

JPEGEncoder.prototype._endFrame = function(done) {
  if (!this.ended) {
    this.ended = true;
    this.encoder.end();
    this.encoder.delete();
  }
  
  done();
};

module.exports = JPEGEncoder;
