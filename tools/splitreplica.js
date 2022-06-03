const fs = require('fs');

//let bin = fs.readFileSync("replica1-firmware.double-ROM.bin");
let bin = fs.readFileSync("replica1-firmware.double-ROM.modded.bin");

let rom1 = new Uint8Array(bin.slice(0, 0x2000));
let rom2 = new Uint8Array(bin.slice(0x2000, 0x4000));

//fs.writeFileSync("replica.rom1.e000.bin", rom1);
//fs.writeFileSync("replica.rom2.e000.bin", rom2);

fs.writeFileSync("replica.rom1.e000.modded.bin", rom1);
fs.writeFileSync("replica.rom2.e000.modded.bin", rom2);

console.log(`written`);




