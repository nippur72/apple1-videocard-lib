function bitsToSamples(bits, samplerate, one_freq, zero_freq) {

   let clock = 1000000;

   let cycles = bits.map(b=>{
      if(b==1) return samplerate / one_freq;    // 1000 Hz
      else     return samplerate / zero_freq;   // 2000 Hz
   });

   let samples = [];
   let volume = 0.75;

   let ptr = 0;
   for(let i=0;i<cycles.length;i++) {
      let nsamples = cycles[i];
      while(ptr<nsamples) {
         if(ptr<nsamples/2) samples.push(volume);
         else               samples.push(-volume);
         ptr++;
      }
      ptr-=nsamples;
   }
   return samples;
}

module.exports = { bitsToSamples };