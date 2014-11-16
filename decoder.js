var jpeg = require('./build/jpeg');
var Decoder = jpeg.JPEGDecoder;
var util = require('util');
var Transform = require('stream').Transform;

function JPEGDecoder(opts) {
  Transform.call(this);
  this.decoder = new Decoder;
  
  if (opts && opts.width && opts.height)
    this.decoder.setDesiredSize(opts.width, opts.height);
    
  this.width = 0;
  this.height = 0;
    
  var self = this;
  this.decoder.callback = function(type, ptr, len) {
    switch (type) {
      case 'outputSize':
        self.width = self.decoder.width;
        self.height = self.decoder.height;
        self.colorSpace = self.decoder.colorSpace;
        self.emit('format');
        break;
        
      case 'scanline':
        self.push(new Buffer(jpeg.HEAPU8.subarray(ptr, ptr + len)));
        break;
        
      case 'end':
        self.push(null);
        break;
        
      case 'error':
        self.decoder.delete();
        self.emit('error', new Error(ptr));
        break;
    }
  };
}

util.inherits(JPEGDecoder, Transform);

JPEGDecoder.probe = function(buf) {
  return buf[0] === 0xff && buf[1] === 0xd8 && buf[2] === 0xff;
};

JPEGDecoder.prototype._transform = function(data, encoding, done) {
  var buf = data instanceof Uint8Array ? data : new Uint8Array(data);
  this.decoder.decode(buf);
  done();
};

JPEGDecoder.prototype._flush = function(done) {
  this.decoder.delete();
  done();
};

module.exports = JPEGDecoder;
