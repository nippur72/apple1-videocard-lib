function hex(value, size) {
   if(size === undefined) size = 2;
   let s = "0".repeat(size) + value.toString(16);
   return s.substr(s.length - size).toUpperCase();
}

module.exports = { hex };