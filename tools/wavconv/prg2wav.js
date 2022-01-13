#!/usr/bin/env node

const fs = require('fs');
const parseOptions = require("./parseOptions");
const { samples2wav } = require("./samples2wav");
const { bytesToBits } = require("./bytes2bits");
const { bitsToSamples } = require("./bits2samples");
const { checksum_byte } = require("./checksum");
const { hex } = require("./hex");
const { ACI } = require("./aci");

const options = parseOptions([
   { name: 'input', alias: 'i', type: String },
   { name: 'output', alias: 'o', type: String },
   { name: 'samplerate', alias: 's', type: Number },
   { name: 'binary', alias: 'b', type: String }
]);

if(options.input === undefined || options.output === undefined) {
   console.log("usage: prg2wav -i inputfile.prg -o outputfile [-s samplerate] [-b hexaddress]");
   console.log("");
   console.log("The input is a binary file with two bytes start address header (usually with .prg extension).");
   console.log("If -b <hexaddress> is specified, the input file is treated as a binary with no header");
   console.log("and the start address must be specified (in hexadecimal format).");
   console.log("Samplerate is the rate of the output WAV file (44100 Hz default)");
   process.exit(-1);
}

let samplerate = options.samplerate == undefined ? 44100 : options.samplerate;

let binfile,startaddress;
if(options.binary) {
   binfile = fs.readFileSync(options.input);
   startaddress = parseInt(options.binary,16);
}
else {
   let prgfile = fs.readFileSync(options.input);
   startaddress = prgfile[0]+prgfile[1]*256;
   binfile = prgfile.slice(2);
}
let endaddress = startaddress + binfile.length - 1;

// header is composed of a 10 seconds of long cycles ("1") ending with a short cyles ("0")
let header = new Uint8Array(1250).fill(255);
let startbyte = 254; // 7 long cycles and a short one as start bit
let checksum = checksum_byte(binfile);
let slipbytes = [ 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE ];

let data = [ ...header, startbyte, ...binfile, checksum, ...slipbytes ];

let bits = bytesToBits(data);
let samples = bitsToSamples(bits, samplerate, ACI.one, ACI.zero);
let wavfile = samples2wav(samples, samplerate);
let wavName = options.output;

let s_start = hex(startaddress,4);
let s_end   = hex(endaddress,4);
wavName = `${wavName}_${s_start}.${s_end}R.wav`;

fs.writeFileSync(wavName, wavfile);

console.log(`file "${wavName}" generated, load it on the Apple-1 with:`);
console.log(`C100R (RETURN) ${s_start}.${s_end}R (RETURN)`);
console.log(``);

