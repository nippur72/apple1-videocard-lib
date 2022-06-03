const fs = require('fs');
const parseOptions = require("./parseOptions");

const options = parseOptions([
   { name: 'input', alias: 'i', type: String }      
]);

if(options.input === undefined) {
   console.log("usage: pdos2prg -i inputfile#[...] ");
   process.exit(-1);
}

let s = options.input.split("#");
if(s.length != 2) {
   console.log("pdos2prg: filename must contain # tag");
   process.exit(-1);
}

let tag = s[1];
let start_address = tag.substr(2);
let file_type = tag.substr(0,2).toUpperCase();

let ext = (function(){
        if(file_type == "F1") return `_BAS_${start_address}`;
   else if(file_type == "F8") return `_ASOFT_${start_address}`;
   else                       return `_BIN_${start_address}`;
})();

let filename = `${s[0]}${ext}.prg`;

start_address = parseInt(start_address,16);

let bin = fs.readFileSync(options.input);

if(file_type == "F1") {
   bin = bas2bin(bin, start_address);
   start_address = 0x0002;
}

let lo = (start_address >> 0) & 0xFF;
let hi = (start_address >> 8) & 0xFF;

let prg = new Uint8Array([ lo, hi, ...bin ]);

fs.writeFileSync(filename, prg);

console.log(`${filename} written`);


// converts pdos integer basic program
//
// pdos: $0000-$0049 junk
//       $004A-$01FF pointers
//       $0200-end   basic program to relocate at start address
//
// returns:
//       $004A-$01FF   pointers
//       $0200-start-1 filler
//       start - end   basic program

function bas2bin(data, addr) {
   let pointers = data.slice(0x0002, 0x0200);
   let prg = data.slice(0x0200);
   let diff = new Array(addr - 0x0200).fill(0);
   let joined = [ ...pointers, ...diff, ...prg];
   return joined;
}
