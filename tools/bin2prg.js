const fs = require('fs');
const parseOptions = require("./parseOptions");

const options = parseOptions([
   { name: 'input', alias: 'i', type: String },
   { name: 'output', alias: 'o', type: String },
   { name: 'start', alias: 's', type: String }
]);

if(options.input === undefined || options.output === undefined || options.start === undefined) {
   console.log("usage: bin2prg -i inputfile.prg -o outputfile.bin -s hexaddress");
   process.exit(-1);
}

let bin = fs.readFileSync(options.input);

let start_address = parseInt(options.start, 16);
let lo = (start_address >> 0) & 0xFF;
let hi = (start_address >> 8) & 0xFF;

let prg = new Uint8Array([ lo, hi, ...bin ]);

fs.writeFileSync(options.output, prg);

console.log(`${options.output} written`);

