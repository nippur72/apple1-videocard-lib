#!/usr/bin/env node

const fs = require('fs');
const WavDecoder = require("wav-decoder");
const parseOptions = require("./parseOptions");
const { ACI } = require("./aci");

function samples2phases(samples) {
   let phases = [];

   // quantize data
   let s = samples.map(e=> e<0 ? 0 : 1);

   let counter = 0;
   let last_state = 0;

   for(let i=0;i<s.length-1;i++) {
      counter++;
      if(last_state != s[i]) {
         let phase_width = counter;
         counter = 0;
         last_state = s[i];
         phases.push(phase_width);
      }
   }
   return phases;
}

function getDataPhases(phases, mid_point) {

   // how many header long phases before the short phase of the starting bit
   let header_len = (8*2)*256;

   for(let t=header_len;t<phases.length;t++) {
      if(phases[t] <= mid_point) {
         // if we found a starting bit half phase, check the header before
         let start_found = true;
         for(let i=1;i<header_len;i++) {
            if(phases[t-i] <= mid_point) {
               start_found = false;
               break;
            }
         }
         if(start_found) {
            return phases.slice(t+2); // also skip the second phase of the start bit
         }
      }
   }
   throw "start bit not found";
}

function phases2bits(pulses, mid_point) {
   let cycles = [];

   // compat two consecutive half phases into one full cycle
   for(let t=0; t<pulses.length; t+=2) {
      cycles.push(pulses[t]+pulses[t+1]);
   }

   let bits = [];
   for(let t=0; t<cycles.length; t++) {
      let width = cycles[t];
      if(width > mid_point) bits.push(1);
      else bits.push(0);
   }

   return bits;
}

function bits2bytes(bits) {
   let bytes = [];
   for(let t=0; t<bits.length; t+=8) {
      let byte =
         (bits[t+0] << 7) +
         (bits[t+1] << 6) +
         (bits[t+2] << 5) +
         (bits[t+3] << 4) +
         (bits[t+4] << 3) +
         (bits[t+5] << 2) +
         (bits[t+6] << 1) +
         (bits[t+7] << 0) ;
      bytes.push(byte);
   }
   return bytes;
}

////////////////////////////////////////////////////////////////////////////////////////

const options = parseOptions([
    { name: 'input', alias: 'i', type: String },
    { name: 'output', alias: 'o', type: String },
    { name: 'start', alias: 's', type: String },
    { name: 'end', alias: 'e', type: String }
 ]);

if(options.input === undefined || options.output === undefined) {
    console.log("usage: wav2prg -i inputfile.wav -o outputfile [-s hexstart] [-e hexend]");
    process.exit(-1);
}


let wavfile = fs.readFileSync(options.input);
let audioData = WavDecoder.decode.sync(wavfile);
let samples = audioData.channelData[0];

let samplerate = audioData.sampleRate;

const long_cycle  = samplerate / ACI.one;
const short_cyle = samplerate / ACI.zero;
const cycle_mid_point = (long_cycle + short_cyle) / 2;
const phase_mid_point = cycle_mid_point / 2;

let phases = samples2phases(samples);
let dataPhases = getDataPhases(phases, phase_mid_point);
let bits = phases2bits(dataPhases, cycle_mid_point);
let bytes = bits2bytes(bits);

if(options.start !== undefined && options.end !== undefined) {
   let start_address = parseInt(options.start,16);
   let end_address = parseInt(options.end,16);
   let len = end_address - start_address + 1;  // end address included
   if(bytes.length<len) throw `decoded file (${bytes.length} bytes) smaller than specified (${len} bytes)`;

   // cut file
   bytes = bytes.slice(0,len);

   // add prg header
   let hi = (start_address >> 8) & 0xff;
   let lo = (start_address >> 0) & 0xff;
   bytes = [ lo, hi, ...bytes];

   let prgFile = new Uint8Array(bytes);
   let prgName = `${options.output}.prg`;
   fs.writeFileSync(prgName, prgFile);
   console.log(`file "${prgName}" generated (it has two bytes header start address)`);
}
else
{
   let binFile = new Uint8Array(bytes);
   let binName = `${options.output}.bin`;
   fs.writeFileSync(binName, binFile);
   console.log(`no address specified, writing raw binary`);
   console.log(`file "${binName}" generated`);
}

