const fs = require('fs');
const parseOptions = require("./parseOptions");

const options = parseOptions([
   { name: 'input', alias: 'i', type: String },
   { name: 'output', alias: 'o', type: String }
]);

if(options.input === undefined || options.output === undefined) {
   console.log("usage: prg2bin -i inputfile.prg -o outputfile.bin");
   process.exit(-1);
}

let prg = fs.readFileSync(options.input);

prg = prg.slice(2);

fs.writeFileSync(options.output,prg);

console.log(`${options.output} written`);

