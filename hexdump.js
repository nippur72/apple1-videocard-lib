const fs = require('fs');

let prg = fs.readFileSync("test_apple1.prg");

//prg = prg.slice(2); // skip header

let s = appledump(prg, 0x4000);

console.log(s);

function appledump(bytes, start) {
   let rows=16;
   let s="\r\n";
   for(let r=0;r<bytes.length;r+=rows) {
      s+= hex(r+start, 4) + ": ";
      for(let c=0;c<rows;c++) {
         let index = r+c;
         if(index < bytes.length) {
            const byte = bytes[r+c];
            s+= hex(byte)+" ";   
         }
      }
      s+="\n";
   }
   return s;
}

function hex(value, size) {
   if(size === undefined) size = 2;
   let s = "0000" + value.toString(16);
   return s.substr(s.length - size);
}
