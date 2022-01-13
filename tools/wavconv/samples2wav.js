const WavEncoder = require("wav-encoder");

// turns the samples into an actual WAV file
// returns the array of bytes to be written to file

function samples2wav(samples, samplerate) {
   const wavData = {
       sampleRate: samplerate,
       channelData: [ new Float32Array(samples) ]
   };
   const buffer = WavEncoder.encode.sync(wavData, { bitDepth: 16, float: false });
   return Buffer.from(buffer)
}

module.exports = { samples2wav };
