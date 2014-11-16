var JPEGEncoder = require('../encoder');
var JPEGDecoder = require('../decoder');
var assert = require('assert');
var fs = require('fs');
var concat = require('concat-frames');

describe('JPEGEncoder', function() {
  it('encodes an RGB image', function(done) {
    var pixels = new Buffer(10 * 10 * 3);
    for (var i = 0; i < pixels.length; i += 3) {
      pixels[i] = 204;
      pixels[i + 1] = 0;
      pixels[i + 2] = 151;
    }
    
    var enc = new JPEGEncoder(10, 10);
    
    enc.pipe(new JPEGDecoder)
       .pipe(concat(function(frames) {
         assert.equal(frames.length, 1);
         assert.equal(frames[0].width, 10);
         assert.equal(frames[0].height, 10);
         assert.equal(frames[0].colorSpace, 'rgb');
         assert.deepEqual(frames[0].pixels.slice(0, 3), new Buffer([ 204, 0, 151 ]));
         done();
       }));
    
    enc.end(pixels);
  });
  
  it('encodes a CMYK image', function(done) {
    var pixels = new Buffer(10 * 10 * 4);
    for (var i = 0; i < pixels.length; i += 4) {
      pixels[i] = 0;
      pixels[i + 1] = 56;
      pixels[i + 2] = 128;
      pixels[i + 3] = 32;
    }
    
    var enc = new JPEGEncoder(10, 10, { colorSpace: 'cmyk' });
    
    enc.pipe(new JPEGDecoder)
       .pipe(concat(function(frames) {
         assert.equal(frames.length, 1);
         assert.equal(frames[0].width, 10);
         assert.equal(frames[0].height, 10);
         assert.equal(frames[0].colorSpace, 'cmyk');
         assert.deepEqual(frames[0].pixels.slice(0, 4), new Buffer([ 0, 56, 128, 32 ]));
         done();
       }));
    
    enc.end(pixels);
  });
  
  it('encodes a grayscale image', function(done) {
    var pixels = new Buffer(10 * 10);
    pixels.fill(128);
    
    var enc = new JPEGEncoder(10, 10, { colorSpace: 'gray' });
    
    enc.pipe(new JPEGDecoder)
       .pipe(concat(function(frames) {
         assert.equal(frames.length, 1);
         assert.equal(frames[0].width, 10);
         assert.equal(frames[0].height, 10);
         assert.equal(frames[0].colorSpace, 'gray');
         assert.equal(frames[0].pixels[0], 128);
         done();
       }));
    
    enc.end(pixels);
  });
});
