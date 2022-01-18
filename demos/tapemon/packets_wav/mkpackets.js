const fs = require("fs");

function make_packets_bin(PACKETSIZE) {
   let data = [];
   for(let t=0;t<PACKETSIZE;t++) data.push(t);

   let packet = [
      0xff, 0xff, 0xff, 0xff, 0xfe,       // header plus start bit
      ...data,                            // the actual data block
      0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, // filler
   ]

   let packets = [];

   for(let t=0;t<1000;t++) {
      packets = [...packets, ...packet];
   }

   let file = new Uint8Array(packets);

   fs.writeFileSync(`packets_${PACKETSIZE}.bin`, file);
}

make_packets_bin(32);
make_packets_bin(64);
make_packets_bin(128);
make_packets_bin(255);

function make_single_tone(data_byte) {
   let data = [];
   for(let t=0;t<128*1024;t++) data.push(data_byte);

   let file = new Uint8Array(data);

   fs.writeFileSync(`single_tone_${data_byte}.bin`, file);
}

make_single_tone(0);
make_single_tone(255);



