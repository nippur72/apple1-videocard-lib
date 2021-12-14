/*
const fs = require('fs');

let prg = fs.readFileSync("tetris_apple1.prg");

prg = prg.slice(2);

fs.writeFileSync("tetris_apple1.bin",prg);

console.log("bin written");
*/

const fs = require('fs');

let code = fs.readFileSync("apple1_codeseg.bin");
let data = fs.readFileSync("apple1_dataseg.bin");

let mem = new Uint8Array(65536).fill(0xAA);

let code_address = 0x4000;
let data_address = 0x8000-0x7ff-0x280+1;

for(let i=0;i<code.length;i++) mem[code_address+i] = code[i];
for(let i=0;i<data.length;i++) mem[data_address+i] = data[i];

let prg = mem.slice(0x4000,0x8000);

fs.writeFileSync("tetris_apple1.bin", prg);

console.log("bin written");
