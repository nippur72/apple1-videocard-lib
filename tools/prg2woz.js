const fs = require('fs');
const parseOptions = require("./parseOptions");
const { hex } = require('./wavconv/hex');

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

const options = parseOptions([
   { name: 'input', alias: 'i', type: String },
   { name: 'output', alias: 'o', type: String }
]);


if(options.input === undefined || options.output === undefined) {
   console.log("usage: prg2woz -i inputfile.prg -o outputfile.woz");
   process.exit(-1);
}

let prgname = options.input;
let wozname = options.output;

let prg = fs.readFileSync(prgname);
let baseaddress = prg[0] + prg[1]*256;
let bin = prg.slice(2); // skip header
let woz = appledump(bin, baseaddress);
fs.writeFileSync(wozname, woz);

console.log(`file "${wozname}" generated`);
