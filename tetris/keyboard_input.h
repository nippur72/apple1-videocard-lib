#ifndef KEYBOARD_H
#define KEYBOARD_H

/* keyboard defititions */
#define KEY_LEFT   'J'
#define KEY_RIGHT  'L'
#define KEY_DOWN   'K'
#define KEY_DROP   ' '
#define KEY_ROTATE 'I'
#define KEY_RETURN 0x0d

byte test_key(byte key);
byte read_keyboard();
byte player_input();

// test a specific key
byte test_key(byte key) {
   return read_keyboard() == key ? 1 : 0;
}

// reads the keyboard and return the key pressed
byte read_keyboard() {
   return apple1_readkey();
}

byte player_input() {
   return read_keyboard();
}

#endif
