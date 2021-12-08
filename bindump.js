const fs = require('fs');

let prg = fs.readFileSync("test_apple1.prg");

prg = prg.slice(2);

fs.writeFileSync("test_apple1.bin",prg);

console.log("bin written");
