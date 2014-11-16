# jpg-stream

A streaming JPEG encoder and decoder for Node and the browser. It is a direct compilation
of [libjpeg](http://www.ijg.org) to JavaScript using [Emscripten](http://emscripten.org/).

## Installation

    npm install jpg-stream

For the browser, you can build using [Browserify](http://browserify.org/).

## Decoding

This example uses the [concat-frames](https://github.com/devongovett/concat-frames)
module to collect the output of the JPEG decoder into a single buffer.

```javascript
var JPEGDecoder = require('jpg-stream/decoder');
var concat = require('concat-frames');

// decode a JPEG file to RGB pixels
fs.createReadStream('in.jpg')
  .pipe(new JPEGDecoder)
  .pipe(concat(function(frames) {
    // frames is an array of frame objects (one for JPEGs)
    // each element has a `pixels` property containing
    // the raw RGB pixel data for that frame, as
    // well as the width, height, etc.
  }));
```

### Scaling

Large JPEGs from DSLRs can be somewhat slow to decode.  If you don't need the image at
its full size for preview, or will be resizing the image anyway, there is an option to
perform scaling at decode time.  This improves performance dramatically since only the
DCT coefficients necessary for the desired size are decoded.

To specify decode scaling, provide `width` and `height` options to the decoder.  This
represents the minimum size you want, and the decoder will output an image of at least
this size, but likely not exactly that size. For exact resizing, provide your minimum
allowed size to the decoder and use the [resize-pixels](https://github.com/devongovett/resize-pixels)
module to resize the JPEG decoder's output to the exact size.

```javascript
fs.createReadStream('large.jpg')
  .pipe(new JPEGDecoder({ width: 600, height: 400 }))
  .pipe(concat(function(frames) {
    // frames[0].width >= 600 and frames[0].height >= 400
  }));
```

## Encoding

You can encode a JPEG by writing or piping pixel data to a `JPEGEncoder` stream.
You can set the `quality` option to a number between 1 and 100 to control the
size vs quality tradeoff made by the encoder.

The JPEG encoder supports writing data in the RGB, grayscale, or CMYK color spaces.
If you need to convert from another unsupported color space, first pipe your data
through the [color-transform](https://github.com/devongovett/color-transform) module.

```javascript
var JPEGEncoder = require('jpg-stream/encoder');
var ColorTransform = require('color-transform');

// convert a PNG to a JPEG
fs.createReadStream('in.png')
  .pipe(new PNGDecoder)
  .pipe(new JPEGEncoder({ quality: 80 }))
  .pipe(fs.createWriteStream('out.jpg'));
  
// colorspace conversion to convert from RGBA to RGB
fs.createReadStream('rgba.png')
  .pipe(new PNGDecoder)
  .pipe(new ColorTransform('rgb'))
  .pipe(new JPEGEncoder)
  .pipe(fs.createWriteStream('rgb.jpg'));
```

## License

MIT
