function bytesToBits(data) {
   let bits = [];
   data.forEach(byte => {
      for(let t=7;t>=0;t--) {
         let bit = (byte >> t) & 1;
         bits.push(bit);
      }
   });
   return bits;
}

module.exports = { bytesToBits };