const fs = require("fs");

const PACKETSIZE = 64;

let data = [];
for(let t=0;t<PACKETSIZE;t++) data.push(t);

let packet = [
   0xff, 0xff, 0xff, 0xff, 0xfe,       // header plus start bit
   ...data,                          // the actual data block
   0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, // filler
]

let packets = [];

for(let t=0;t<1000;t++) {
   packets = [...packets, ...packet];
}

let file = new Uint8Array(packets);

fs.writeFileSync("packets.bin", file);


