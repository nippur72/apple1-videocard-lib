/*
const fs = require('fs');
function prg2bin(){
   let prg = fs.readFileSync("tetris_apple1.prg");
   prg = prg.slice(2);
   fs.writeFileSync("tetris_apple1.bin",prg);
   console.log("bin written");
}
*/

const fs = require('fs');

if(process.argv.length < 3) {
   console.log("usage: node mkeprom.js <inputdir> <outputfile>");
   console.log("   <inputdir> is the directory containing 'apple1_codeseg.bin' and 'apple1_dataseg.bin'");
   console.log("   <outputfile> is outfile binary eprom file ($4000-$7FFF)");
   process.exit(0);
}

let file_in  = process.argv[2];
let file_out = process.argv[3];

let code = fs.readFileSync(`${file_in}/apple1_codeseg.bin`);
let data = fs.readFileSync(`${file_in}/apple1_dataseg.bin`);

let mem = new Uint8Array(65536).fill(0xAA);

let code_address = 0x4000;
let data_address = 0x8000-0x7ff-0x280+1;

for(let i=0;i<code.length;i++) mem[code_address+i] = code[i];
for(let i=0;i<data.length;i++) mem[data_address+i] = data[i];

let prg = mem.slice(0x4000,0x8000);

fs.writeFileSync(file_out, prg);

console.log(`bindump: written to ${file_out}`);

