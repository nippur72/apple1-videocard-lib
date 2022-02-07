// 1 per SD card normale, 0 per SDFat

#include <Regexp.h>

#define USE_SD_H 0

#include <SPI.h>

#if USE_SD_H
#include <SD.h>
#else  
#include "SdFat.h"
SdFat SD;
#endif

#define SD_CS_PIN SS

/*
   +-----------+                     +-----------------+ 
   |           |                     |     ARDUINO     |
   | VIA 6522  |                     |      NANO       |
   |           |                     |                 |
   |       PA0 |<------ bit 0 ------>| D2              |                +-------\ 
   |       PA1 |<------ bit 1 ------>| D3          D10 |----- SS ------>|        \
   |       PA2 |<------ bit 2 ------>| D4          D11 |----- MOSI ---->|  SD    |
   |       PA3 |<------ bit 3 ------>| D5          D12 |<---- MISO -----|  CARD  |
   |       PA4 |<------ bit 4 ------>| D6          D13 |----- SCK ----->|        |
   |       PA5 |<------ bit 5 ------>| D7              |                +--------+
   |       PA6 |<------ bit 6 ------>| D8              |
   |       PA7 |<------ bit 7 ------>| D9              |
   |           |                     |                 |
   |       PB7 |<---- MCU_STROBE ----| D14/A0          |
   |       PB0 |--- CPU_STROBE ----->| D15/A1          |
   |           |                     |                 |
   +-----------+                     +-----------------+ 
*/

// pin definitions

#define BIT0 2   
#define BIT1 3   
#define BIT2 4   
#define BIT3 5   
#define BIT4 6   
#define BIT5 7   
#define BIT6 8   
#define BIT7 9   

/* 10,11,12,13 reserved for SD card */

#define MCU_STROBE  14   
#define CPU_STROBE  15   

// indicates that a timeout occurred during wait()
int TIMEOUT = 0;

void wait(int pin, int value) {
    
  unsigned long start_time = millis();  
  unsigned long elapsed;  

  if(TIMEOUT) return;
  
  while(digitalRead(pin) != value) {
    elapsed = millis() - start_time;
    if(elapsed > 500) {
      TIMEOUT = 1;
      break;
    }      
  }
}

const int DIR_INPUT  = 0;
const int DIR_OUTPUT = 1;

int last_dir; // remember last data port pins direction 

void set_data_port_direction(int dir) {

  // check if no need to set pins
  if(dir == last_dir) return;
  
  if(dir == DIR_INPUT) {
    pinMode(BIT0, INPUT);
    pinMode(BIT1, INPUT);
    pinMode(BIT2, INPUT);
    pinMode(BIT3, INPUT);
    pinMode(BIT4, INPUT);
    pinMode(BIT5, INPUT);
    pinMode(BIT6, INPUT);
    pinMode(BIT7, INPUT);     
  }
  else {
    pinMode(BIT0, OUTPUT);
    pinMode(BIT1, OUTPUT);
    pinMode(BIT2, OUTPUT);
    pinMode(BIT3, OUTPUT);
    pinMode(BIT4, OUTPUT);
    pinMode(BIT5, OUTPUT);
    pinMode(BIT6, OUTPUT);
    pinMode(BIT7, OUTPUT);
  }  

  // remember direction
  last_dir = dir;
}

int receive_byte_from_cpu() {

  // set data port pins as INPUT pins
  set_data_port_direction(DIR_INPUT);
  
  // both strobes are 0   
  
  // CPU deposits data byte and sets strobe high
  wait(CPU_STROBE, HIGH);           
  
  // read the data byte
  int data = 
    (digitalRead(BIT0) << 0) |
    (digitalRead(BIT1) << 1) |
    (digitalRead(BIT2) << 2) |
    (digitalRead(BIT3) << 3) |
    (digitalRead(BIT4) << 4) |
    (digitalRead(BIT5) << 5) |
    (digitalRead(BIT6) << 6) |
    (digitalRead(BIT7) << 7);

  // after reading the byte, MCU sets strobe high
  digitalWrite(MCU_STROBE, HIGH);

  // CPU now sets strobe low
  wait(CPU_STROBE, LOW);

  // and MCU sets strobe low
  digitalWrite(MCU_STROBE, LOW);
    
  return data;
}

void send_byte_to_cpu(int data) {

  // set data port pins as OUTPUT pins
  set_data_port_direction(DIR_OUTPUT);

  // both strobes are 0
  
  // put byte on the data port
  digitalWrite(BIT0, data &   1);
  digitalWrite(BIT1, data &   2);
  digitalWrite(BIT2, data &   4);
  digitalWrite(BIT3, data &   8);
  digitalWrite(BIT4, data &  16);
  digitalWrite(BIT5, data &  32);
  digitalWrite(BIT6, data &  64);
  digitalWrite(BIT7, data & 128);
  
  // after depositing data byte, MCU sets strobe high
  digitalWrite(MCU_STROBE, HIGH);
  
  // wait for CPU to set strobe high
  wait(CPU_STROBE, HIGH);  
  
  // tells CPU byte we are finished
  digitalWrite(MCU_STROBE, LOW); 

  // wait for CPU to set strobe low
  wait(CPU_STROBE, LOW);  
}

// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************

void setup() {
  // debug on serial
  Serial.begin(9600);  

#if USE_SD_H
  Serial.println(F("SDCARD library: SD.h"));
#else  // USE_SD_H
  Serial.println(F("SDCARD library: SDFat.h"));
#endif  // USE_SD_H

  // initialize SD card
  if (!SD.begin(SD_CS_PIN)) Serial.println("SD card initialization failed");    
  else                      Serial.println("SD card initialized");
  
  // control pins setup
  pinMode(CPU_STROBE, INPUT);
  pinMode(MCU_STROBE, OUTPUT);  
  digitalWrite(MCU_STROBE, LOW);   

  // data pins setup
  last_dir = -1;  // no previous data direction  
  set_data_port_direction(DIR_INPUT);

  MatchState ms;
  char buf [100] = { F("The quick brown fox jumps over the lazy wolf") };
  ms.Target (buf);
  char result = ms.Match ("f.x");
  if(result >0) {
    Serial.println("match!");  
  }
}

const int CMD_READ  = 0;
const int CMD_WRITE = 1;
const int CMD_DIR   = 2;

const int ERR_RESPONSE = 255;
const int OK_RESPONSE  =   0;

char filename[64]; 

// **************************************************************************************
// **************************************************************************************
// ********************************* DIR  ***********************************************
// **************************************************************************************
// **************************************************************************************

void printDirectory(File dir, int numTabs) {
  
  while (true) {
    if(TIMEOUT) break;

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }

    // indentazione delle sottodirectory
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
      send_byte_to_cpu(32);
      send_byte_to_cpu(32);
      send_byte_to_cpu(32);
    }

    // nome del file
    char *msg;
#if USE_SD_H    
    msg = entry.name();
#else
    entry.getName(filename, 64);
    msg = filename;
#endif
    Serial.print(msg);    
    for(int t=0; t<strlen(msg); t++) {
      send_byte_to_cpu(msg[t]);
    }
    
    if (entry.isDirectory()) {
      Serial.println("/");
      send_byte_to_cpu('/');
      send_byte_to_cpu('\r');
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      send_byte_to_cpu(' ');
      send_byte_to_cpu(' ');

      /*
      // file size
      Serial.println(entry.size(), DEC);
      msg = itoa(entry.size());
      for(int t=0; t<strlen(msg); t++) {
        send_byte_to_cpu(msg[t]);
      }
      */
      send_byte_to_cpu('\r');
      
    }
    entry.close();
  }
}

// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_READ  ******************************************
// **************************************************************************************
// **************************************************************************************

void comando_read() {
  Serial.println("command CMD_READ received from CPU");

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print("file to read: ");
  Serial.println(filename);

  if(!SD.exists(filename)) {
    Serial.println("error opening file");
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu("?FILE NOT FOUND");
    return;    
  }

  // open the file 
  File myFile = SD.open(filename);
  if(!myFile) {            
    Serial.println("error opening file");
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu("?CAN'T OPEN FILE");
    return;
  }  
  Serial.println("file opened on the SD card");

  // ok response
  send_byte_to_cpu(OK_RESPONSE);
  if(TIMEOUT) return;
  Serial.println("ok response sent to CPU");
    
  // sends size as low and high byte
  int size = myFile.size();
  send_byte_to_cpu(size & 0xFF);
  send_byte_to_cpu((size >> 8) & 0xFF);
  if(TIMEOUT) return;
  Serial.println("file size sent to CPU");

  int bytes_sent = 0;    
  while(myFile.available() && !TIMEOUT) {      
    send_byte_to_cpu(myFile.read());
    if(!TIMEOUT) bytes_sent++;
  }    
  myFile.close();

  if(TIMEOUT) {
    Serial.print("timeout, bytes sent: "); 
    Serial.println(bytes_sent); 
    return;
  }
    
  Serial.println("file read ok");
}

void send_string_to_cpu(char *msg) {
  while(1) {
    int c = *msg++;
    send_byte_to_cpu(c);
    if(TIMEOUT) break;
    if(c==0) break;
  }
}

void receive_string_from_cpu(char *msg) {
  while(1) {
    int c = receive_byte_from_cpu();
    if(TIMEOUT) break;    
    *msg++ = c;
    if(c==0) break;
  }  
}

// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_WRITE  *****************************************
// **************************************************************************************
// **************************************************************************************

int receive_word_from_cpu() {
  int lo = receive_byte_from_cpu();
  int hi = receive_byte_from_cpu();
  int data = lo | (hi << 8);  
  return data;
}

void comando_write() {
  Serial.println("command CMD_WRITE received from CPU");

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print("file to write: ");
  Serial.println(filename);
  
  if(SD.exists(filename)) {
    Serial.println("file already exist");
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu("?ALREADY EXISTS");
    return;    
  }

  // open the file 
  File myFile = SD.open(filename, FILE_WRITE);
  if(!myFile) {            
    Serial.println("error opening file for write");
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu("?CAN'T CREATE FILE");
    return;
  }  
  Serial.println("file opened for write on the SD card");

  // ok response
  send_byte_to_cpu(OK_RESPONSE);
  if(TIMEOUT) return;
  Serial.println("first ok response sent to CPU");
    
  // get file size low and high byte
  int size = receive_word_from_cpu();
  if(TIMEOUT) return;
  Serial.print("received file size: ");
  Serial.println(size);

  int error = 0;
  for(int t=0;t<size;t++) {
    int data = receive_byte_from_cpu();
    if(TIMEOUT) break;
    int n = myFile.write((unsigned char)data);
    if(n!=1) error = 1;
  }

  // close the file
  myFile.close();

  if(TIMEOUT) return;

  // report write issues
  if(error) {
    Serial.println("file write error");    
    send_byte_to_cpu(ERR_RESPONSE);
    if(TIMEOUT) return;
    send_string_to_cpu("?WRITE ERROR");
    return;
  }
      
  Serial.println("file read ok");
  send_byte_to_cpu(OK_RESPONSE);
}

// **************************************************************************************
// **************************************************************************************
// ********************************* LOOP  **********************************************
// **************************************************************************************
// **************************************************************************************

void loop() {  
  TIMEOUT = 0;

  int data = receive_byte_from_cpu();
  if(TIMEOUT) return;

  if(data == CMD_READ) {
    comando_read();
    if(TIMEOUT) Serial.println("TIMEOUT during CMD_READ");    
  }
  else if(data == CMD_WRITE) {
    comando_write();
    if(TIMEOUT) Serial.println("TIMEOUT during CMD_WRITE");
  }
  else if(data == CMD_DIR) {
    Serial.println("command DIR received from CPU");

    File root = SD.open("/");
    printDirectory(root, 0);
    root.close();

    // terminates
    send_byte_to_cpu(0);

    if(TIMEOUT) Serial.println("TIMEOUT during DIR");    

    Serial.println("command DIR ended");    
  }
  else {
    Serial.print("unknown command ");
    Serial.print(data);
    Serial.println(" received");
  }
}
