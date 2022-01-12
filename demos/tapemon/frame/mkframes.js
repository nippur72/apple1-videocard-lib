const fs = require("fs");

const FRAMELEN = 64;

let data_block = [];
for(let t=0;t<FRAMELEN;t++) data_block.push(t);

let frame = [
   0xff, 0xff, 0xff, 0xff, 0xfe,       // header plus start bit
   ...data_block,                      // the actual data block
   0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, // filler
]

let frames = [];

for(let t=0;t<1000;t++) {
   frames = [...frames, ...frame];
}

let data = new Uint8Array(frames);

fs.writeFileSync("frames.bin", data);


