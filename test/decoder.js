var JPEGDecoder = require('../decoder');
var assert = require('assert');
var fs = require('fs');
var concat = require('concat-frames');

describe('JPEGDecoder', function() {
  it('can probe to see if a file is a jpeg', function() {
    var file = fs.readFileSync(__dirname + '/images/j1.jpg');
    assert(JPEGDecoder.probe(file));
    assert(!JPEGDecoder.probe(new Buffer(100)));
  });
  
  it('decodes a file', function(done) {    
    fs.createReadStream(__dirname + '/images/j1.jpg')
      .pipe(new JPEGDecoder)
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 261);
        assert.equal(frames[0].height, 202);
        assert.equal(frames[0].colorSpace, 'rgb');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 261 * 202 * 3);
        done();
      }));
  });
  
  it('decodes a progressive file', function(done) {
    fs.createReadStream(__dirname + '/images/j2.jpg')
      .pipe(new JPEGDecoder)
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 261);
        assert.equal(frames[0].height, 202);
        assert.equal(frames[0].colorSpace, 'rgb');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 261 * 202 * 3);
        done();
      }));
  });
  
  it('can decode an rgb image', function(done) {
    fs.createReadStream(__dirname + '/images/rgb.jpg')
      .pipe(new JPEGDecoder)
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 620);
        assert.equal(frames[0].height, 371);
        assert.equal(frames[0].colorSpace, 'rgb');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 620 * 371 * 3);
        done();
      }));
  });
  
  it('can decode a grayscale image', function(done) {
    fs.createReadStream(__dirname + '/images/gray.jpg')
      .pipe(new JPEGDecoder)
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 620);
        assert.equal(frames[0].height, 371);
        assert.equal(frames[0].colorSpace, 'gray');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 620 * 371);
        done();
      }));
  });
  
  it('can decode a cmyk image', function(done) {
    fs.createReadStream(__dirname + '/images/cmyk.jpg')
      .pipe(new JPEGDecoder)
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 620);
        assert.equal(frames[0].height, 371);
        assert.equal(frames[0].colorSpace, 'cmyk');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 620 * 371 * 4);
        done();
      }));
  });
  
  it('can downscale an image while decoding', function(done) {
    // the original image is 1600x1195
    // this tests that we can scale down while decoding
    // (much faster than decoding and then resizing afterward)
    fs.createReadStream(__dirname + '/images/tetons.jpg')
      .pipe(new JPEGDecoder({ width: 600, height: 400 }))
      .pipe(concat(function(frames) {
        assert.equal(frames.length, 1);
        assert.equal(frames[0].width, 800);
        assert.equal(frames[0].height, 598);
        assert.equal(frames[0].colorSpace, 'rgb');
        assert(Buffer.isBuffer(frames[0].pixels));
        assert.equal(frames[0].pixels.length, 800 * 598 * 3);
        done();
      }));
  });
});
