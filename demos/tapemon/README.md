# TAPEMON

Tapemon is a Apple-I tool that helps finding the optimal playback volume to
the ACI audio cassette interface.

## How to use it

- Load `tapemon.0280.bin` on the Apple-1 and execute it with `280R`.

- Connect your playback device (PC, Smartphone, iPod etc) to the ACI

- Play the file `packets.wav` contained in `packets_wav/packets.zip` archive. The file
will send a continous stream of small data packets that are catched by the program
running on the Apple-1. Each packet is 64 bytes long.

For each received packet a character will be displayed:

- `*` indicates a good packet
- `.` indicates packet not received
- an hex digit 0-F indicates the packet is partially corrupt

Adjust the volume level so that you have a lot of `*`.








