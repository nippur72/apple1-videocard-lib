const fs = require('fs');

let prg = fs.readFileSync("montyr_apple1.prg");

prg = prg.slice(2);

fs.writeFileSync("montyr.0280R.bin",prg);

console.log("bin written");
