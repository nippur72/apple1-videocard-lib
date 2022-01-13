function checksum_byte(bytes) {
   let checksum = 0xFF;
   for(let t=0; t<bytes.length; t++) {
      checksum = (checksum ^ bytes[t]) & 0xFF;
   }
   return checksum;
}

module.exports = { checksum_byte };