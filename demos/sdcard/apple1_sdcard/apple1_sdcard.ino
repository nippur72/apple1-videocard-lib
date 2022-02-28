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

/*
PORT ASSIGNMENT FOR FAST READ/WRITE
===================================
BIT 0:      PORTD[2]
BIT 1:      PORTD[3]
BIT 2:      PORTD[4]
BIT 3:      PORTD[5]
BIT 4:      PORTD[6]
BIT 5:      PORTD[7]
BIT 6:      PORTB[0]
BIT 7:      PORTB[1]
MCU_STROBE: PORTC[0]
CPU_STROBE: PORTC[1]

in Verilog syntax: data = { PORTB[1:0], PORTD[7:2] };

*/

#define FASTWRITE 1

#ifdef FASTWRITE
#define get_cpu_strobe    ((PORTC >> 1) & 1)
#define set_mcu_strobe(c) PORTC = (PORTC & 0xFE) | (c)
#else
#define get_cpu_strobe digitalRead(CPU_STROBE)
#define set_mcu_strobe(c) digitalWrite(MCU_STROBE,(c))
#endif


#include <Regexp.h>
#include <SPI.h>
#include "SdFat.h"

SdFat SD;

#define SD_CS_PIN SS


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

// indicates that a timeout occurred during wait_cpu_strobe()
int TIMEOUT = 0;

void wait_cpu_strobe(int value) {
    
  unsigned long start_time = millis();  
  unsigned long elapsed;  

  if(TIMEOUT) return;
  
  while(get_cpu_strobe != value) {
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
  wait_cpu_strobe(HIGH);           
  
  // read the data byte
  #ifdef FASTWRITE  
  int data = ((PORTB & 0x03)<<6) | (PORTD >> 2);  // data = { PORTB[1:0], PORTD[7:2] }
  #else
  int data = 
    (digitalRead(BIT0) << 0) |
    (digitalRead(BIT1) << 1) |
    (digitalRead(BIT2) << 2) |
    (digitalRead(BIT3) << 3) |
    (digitalRead(BIT4) << 4) |
    (digitalRead(BIT5) << 5) |
    (digitalRead(BIT6) << 6) |
    (digitalRead(BIT7) << 7);
  #endif 

  // after reading the byte, MCU sets strobe high
  set_mcu_strobe(HIGH);

  // CPU now sets strobe low
  wait_cpu_strobe(LOW);

  // and MCU sets strobe low
  set_mcu_strobe(LOW);
    
  return data;
}

void send_byte_to_cpu(int data) {

  // set data port pins as OUTPUT pins
  set_data_port_direction(DIR_OUTPUT);

  // both strobes are 0
  
  // put byte on the data port
  #ifdef FASTWRITE  
  PORTB = (PORTB & 0xFC) | ((data >> 6) & 0x03);   // PORTB[1:0] = data[7:6];
  PORTD = (PORTD & 0x03) | (data << 2);            // PORTD[7:2] = data[5:0];  
  #else
  digitalWrite(BIT0, data &   1);
  digitalWrite(BIT1, data &   2);
  digitalWrite(BIT2, data &   4);
  digitalWrite(BIT3, data &   8);
  digitalWrite(BIT4, data &  16);
  digitalWrite(BIT5, data &  32);
  digitalWrite(BIT6, data &  64);
  digitalWrite(BIT7, data & 128);
  #endif
  
  // after depositing data byte, MCU sets strobe high
  set_mcu_strobe(HIGH);
  
  // wait for CPU to set strobe high
  wait_cpu_strobe(HIGH);  
  
  // tells CPU byte we are finished
  set_mcu_strobe(LOW); 

  // wait for CPU to set strobe low
  wait_cpu_strobe(LOW);  
}

// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************
// *********************************************************************************************

const int CMD_READ  =  0;
const int CMD_WRITE =  1;
const int CMD_DIR   =  2;
const int CMD_DEL   = 11;
const int CMD_LS    = 12;
const int CMD_CD    = 13;
const int CMD_MKDIR = 14;
const int CMD_PWD   = 19;
const int CMD_RMDIR = 15;
const int CMD_TEST  = 20;

const int ERR_RESPONSE = 255;
const int OK_RESPONSE  =   0;

char filename[64]; 
char tmp[64];
char cd_path[64]; 

// fixed messages
const char *FILE_NOT_FOUND = "?FILE NOT FOUND";
const char *CANT_OPEN_FILE = "?CAN'T OPEN FILE";
const char *ALREADY_EXISTS = "?FILE ALREADY EXISTS";
const char *CANT_CREATE_FILE = "?CAN'T CREATE FILE";
const char *WRITE_ERROR = "?WRITE ERROR";
const char *CANT_DELETE_FILE = "?CAN'T DELETE FILE";
const char *FILE_DELETED = " DELETED";
const char *DIR_NOT_FOUND = "?DIR NOT FOUND";
const char *CANT_REMOVE_DIR = "?CAN'T REMOVE DIR";
const char *DIR_REMOVED = " (DIR) REMOVED";
const char *DIR_ALREADY_EXISTS = "?DIR ALREADY EXISTS";
const char *CANT_MAKE_DIR = "?CAN'T MAKE DIR";
const char *DIR_CREATED = " (DIR) CREATED";
const char *CANT_CD_DIR = "?CAN'T CHANGE DIR";
const char *NOT_A_DIRECTORY = "?NOT A DIRECTORY";

void setup() {
  // debug on serial
  Serial.begin(9600);  
  Serial.println(F("SDCARD library: SDFat.h"));

  // initialize SD card
  if (!SD.begin(SD_CS_PIN)) Serial.println(F("SD card initialization failed"));    
  else                      Serial.println(F("SD card initialized"));
  
  // control pins setup
  pinMode(CPU_STROBE, INPUT);
  pinMode(MCU_STROBE, OUTPUT);  
  digitalWrite(MCU_STROBE, LOW);   

  // data pins setup
  last_dir = -1;  // no previous data direction  
  set_data_port_direction(DIR_INPUT);

  MatchState ms;
  char buf [100] = { "The quick " };
  ms.Target (buf);
  char result = ms.Match ("f.x");
  if(result >0) {
    Serial.println("match!");  
  }

  // set working directory to root
  strcpy(cd_path, "/");
}


// **************************************************************************************
// **************************************************************************************
// ********************************* @CMD_DIR  ******************************************
// **************************************************************************************
// **************************************************************************************

/*
// recursive print directory
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
    entry.getName(filename, 64);
    msg = filename;
    
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
     
      // file size
      //Serial.println(entry.size(), DEC);
      //msg = itoa(entry.size());
      //for(int t=0; t<strlen(msg); t++) {
      //  send_byte_to_cpu(msg[t]);
      //}     
      send_byte_to_cpu('\r');
      
    }
    entry.close();
  }
}

// versione ricorsiva (disabilitata)
void comando_dir_alternate() {
  Serial.println(F("command DIR received from CPU"));

  File root = SD.open("/");
  printDirectory(root, 0);
  root.close();

  // terminates
  send_byte_to_cpu(0);
  Serial.println(F("command DIR ended"));
}
*/

// versione tipo MS-DOS
void comando_dir(int command) {
  Serial.println(F("command DIR received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("dir to read: "));
  Serial.println(filename);

  File dir;  
  
  if(filename[0]==0) {
    // no argument given, use cd_path
    strcpy(filename, cd_path);    
  }

  dir = SD.open(filename);

  if(!dir) {
    Serial.println(F("dir not found"));    
    send_string_to_cpu(DIR_NOT_FOUND);
    return;    
  }

  // *** lists directories first  
  print_dir_entry(dir, 0, command);  
  if(TIMEOUT) return;
  
  // *** then files
  dir.rewind();
  print_dir_entry(dir, 1, command);
  dir.close();

  if(TIMEOUT) return;

  // terminates
  send_byte_to_cpu(0);
  Serial.println(F("command DIR ended"));
}

void print_dir_entry(File dir, int list_files, int command) {
  while (true) {    
    File entry = dir.openNextFile();

    if(!entry) {
      // no more files
      break;
    }

    // send file size or directory
    entry.getName(filename, 64);
    
    if((list_files == 0 && entry.isDirectory()) || (list_files == 1 && !entry.isDirectory())) {
      if(entry.isDirectory()) {
         if(command != CMD_DIR) {
           sprintf(tmp, "(DIR) %s\r", filename);
           print_string_to_cpu(tmp);
           Serial.println(tmp);     
         }  
         else {
           sprintf(tmp, "%-15s (DIR)\r", filename);
           print_string_to_cpu(tmp);
           Serial.println(tmp);     
         }                
      }
      else {
        if(command != CMD_DIR) {
           // BUG the following line does not work
           // sprintf(tmp, "%5d %s", entry.size(), filename);        
           // use this instead
           sprintf(tmp, "%5d ", entry.size());
           print_string_to_cpu(tmp);
           Serial.print(tmp);       

           print_string_to_cpu(filename);                
           Serial.println(filename);               

           send_byte_to_cpu('\r');           
        }
        else {
           // BUG the following line does not work
           // sprintf(tmp, "%5d %s", entry.size(), filename);        
           // use this instead
           sprintf(tmp, "%-15s", filename);
           print_string_to_cpu(tmp);                              
           Serial.print(filename);               

           sprintf(tmp, "%6d", entry.size());
           print_string_to_cpu(tmp);
           Serial.println(tmp);     

           send_byte_to_cpu('\r');
        }
      }      
      if(TIMEOUT) break;          
    }
    entry.close();
  }
  dir.close();
}


// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_READ  ******************************************
// **************************************************************************************
// **************************************************************************************

void comando_read() {
  Serial.println(F("command CMD_READ received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("file to read: "));
  Serial.println(filename);

  if(!SD.exists(filename)) {
    Serial.println(F("error opening file"));
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu(FILE_NOT_FOUND);
    return;    
  }

  // open the file 
  File myFile = SD.open(filename);
  if(!myFile) {            
    Serial.println(F("error opening file"));
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu(CANT_OPEN_FILE);
    return;
  }  
  Serial.println(F("file opened on the SD card"));

  // ok response
  send_byte_to_cpu(OK_RESPONSE);
  if(TIMEOUT) return;
  Serial.println(F("ok response sent to CPU"));
    
  // sends size as low and high byte
  int size = myFile.size();
  send_byte_to_cpu(size & 0xFF);
  send_byte_to_cpu((size >> 8) & 0xFF);
  if(TIMEOUT) return;
  Serial.println(F("file size sent to CPU"));

  int bytes_sent = 0;    
  while(myFile.available() && !TIMEOUT) {      
    send_byte_to_cpu(myFile.read());
    if(!TIMEOUT) bytes_sent++;
  }    
  myFile.close();

  if(TIMEOUT) {
    Serial.print(F("timeout, bytes sent: ")); 
    Serial.println(bytes_sent); 
    return;
  }
    
  Serial.println(F("file read ok"));
}

void send_string_to_cpu(char *msg) {
  while(1) {
    int c = *msg++;
    send_byte_to_cpu(c);
    if(TIMEOUT) break;
    if(c==0) break;
  }
}

// come send_string_to_cpu ma senza lo zero finale
void print_string_to_cpu(char *msg) {
  while(1) {
    int c = *msg++;
    if(c==0) break;
    send_byte_to_cpu(c);
    if(TIMEOUT) break;    
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
  Serial.println(F("command CMD_WRITE received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("file to write: "));
  Serial.println(filename);
  
  if(SD.exists(filename)) {
    Serial.println(F("file already exist"));
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu(ALREADY_EXISTS);
    return;    
  }

  // open the file 
  File myFile = SD.open(filename, FILE_WRITE);
  if(!myFile) {            
    Serial.println(F("error opening file for write"));
    send_byte_to_cpu(ERR_RESPONSE);
    send_string_to_cpu(CANT_CREATE_FILE);
    return;
  }  
  Serial.println(F("file opened for write on the SD card"));

  // ok response
  send_byte_to_cpu(OK_RESPONSE);
  if(TIMEOUT) return;
  Serial.println(F("first ok response sent to CPU"));
    
  // get file size low and high byte
  int size = receive_word_from_cpu();
  if(TIMEOUT) return;
  Serial.print(F("received file size: "));
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
    Serial.println(F("file write error"));    
    send_byte_to_cpu(ERR_RESPONSE);
    if(TIMEOUT) return;
    send_string_to_cpu(WRITE_ERROR);
    return;
  }
      
  Serial.println(F("file read ok"));
  send_byte_to_cpu(OK_RESPONSE);
}

// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_DEL ********************************************
// **************************************************************************************
// **************************************************************************************

void comando_del() {
  Serial.println(F("command DEL received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("file to delete: "));
  Serial.println(filename);
  
  if(!SD.exists(filename)) {
    Serial.println(F("file does not exist"));
    send_string_to_cpu(FILE_NOT_FOUND);
    return;    
  }

  if(TIMEOUT) return;

  // open the file for deletion
  File myFile = SD.open(filename, FILE_WRITE);
  if(!myFile) {            
    Serial.println(F("error opening file for deletion"));    
    send_string_to_cpu(CANT_DELETE_FILE);
    myFile.close();
    return;
  }  

  if(TIMEOUT) return;
    
  if(!myFile.remove()) {
    Serial.println(F("error removing file"));
    send_string_to_cpu(CANT_DELETE_FILE);      
    myFile.close();
    return;
  }

  // close the file
  myFile.close();

  print_string_to_cpu(filename);  
  send_string_to_cpu(FILE_DELETED); 

  Serial.println(F("file deleted"));       
}

// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_RMDIR ******************************************
// **************************************************************************************
// **************************************************************************************

void comando_rmdir() {
  Serial.println(F("command RMDIR received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("directory to delete: "));
  Serial.println(filename);

  if(TIMEOUT) return;

  // check if the directory exists
  if(!SD.exists(filename)) {
    Serial.println(F("dir does not exist"));
    send_string_to_cpu(DIR_NOT_FOUND);
    return;
  }

  // removes the directory
  if(!SD.rmdir(filename)) {
    Serial.println(F("error removing dir"));
    send_string_to_cpu(CANT_REMOVE_DIR);
    return;
  }

  print_string_to_cpu(filename);
  send_string_to_cpu(DIR_REMOVED);

  Serial.println(F("dir removed"));
}

// **************************************************************************************
// **************************************************************************************
// ********************************* CMD_MKDIR ******************************************
// **************************************************************************************
// **************************************************************************************

void comando_mkdir() {
  Serial.println(F("command MKDIR received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("directory to create: "));
  Serial.println(filename);

  if(TIMEOUT) return;

  // check if the directory exists
  if(SD.exists(filename)) {
    Serial.println(F("dir already exist"));
    send_string_to_cpu(DIR_ALREADY_EXISTS);
    return;
  }

  // removes the directory
  if(!SD.mkdir(filename)) {
    Serial.println(F("error creating"));
    send_string_to_cpu(CANT_MAKE_DIR);
    return;
  }

  print_string_to_cpu(filename);
  send_string_to_cpu(DIR_CREATED);

  Serial.println(F("dir created"));
}

// **************************************************************************************
// **************************************************************************************
// ********************************* @CMD_CD ********************************************
// **************************************************************************************
// **************************************************************************************

void comando_cd() {
  Serial.println(F("command CD received from CPU"));

  // reads filename as 0 terminated string
  receive_string_from_cpu(filename);
  if(TIMEOUT) return;
  Serial.print(F("directory to change to: "));
  Serial.println(filename);

  if(TIMEOUT) return;

  // CD without arguments
  if(filename[0] == 0) {
    // ok response
    send_byte_to_cpu(OK_RESPONSE);
    return;
  }

  // CD ..
  if(filename[0] == '.' && filename[1] == '.' && filename[2] == 0) {  
    strcpy(filename, cd_path);
    for(int t=strlen(filename)-1;t>=1;t--) {
      int c = filename[t];
      filename[t] = 0;
      if(c == '/') break;        
    }
  }

  // CD /
  if(filename[0] == '/' && filename[1] == 0) {
    // changes to the root directory
    if(SD.chdir()) {
      // set root working directory
      strcpy(cd_path, filename);      
      Serial.print(F("dir changed to:"));
      Serial.println(cd_path);
      send_byte_to_cpu(OK_RESPONSE);
      return;      
    }   
    else {
      Serial.println(F("error changing dir"));
      send_byte_to_cpu(ERR_RESPONSE);
      send_string_to_cpu(CANT_CD_DIR);
      return;
    }
  }

  // CD dirname  
  if(SD.chdir(filename)) {
    // update working directory
    if(filename[0] == '/') {       
      strcpy(cd_path, filename);     // path is absolute, replace cwd   
    }
    else {
      // path is relative, append to cwd
      if(cd_path[0] == '/' && cd_path[1] == 0) {        
        cd_path[0] = 0;  // avoid the double slash ("//DIR") when cwd is root 
      }
      sprintf(tmp, "%s/%s", cd_path, filename);
      strcpy(cd_path, tmp);
    }
    
    
    Serial.print(F("dir changed to:"));
    Serial.println(cd_path);    
    send_byte_to_cpu(OK_RESPONSE);
    return;
  } 

  // errors 

  send_byte_to_cpu(ERR_RESPONSE);
  
  // check if the directory exists
  if(!SD.exists(filename)) {
    Serial.println(F("dir not found"));    
    send_string_to_cpu(DIR_NOT_FOUND);
    return;
  }
  
  File myFile = SD.open(filename);
  bool isDir = myFile.isDirectory();
  myFile.close();
  
  if(!isDir) {            
    Serial.println(F("not a directory"));    
    send_string_to_cpu(NOT_A_DIRECTORY);    
  }
  else {
    Serial.println(F("error changing dir"));    
    send_string_to_cpu(CANT_CD_DIR);    
  }
}

// **************************************************************************************
// **************************************************************************************
// ********************************* @CMD_PWD *******************************************
// **************************************************************************************
// **************************************************************************************

void comando_pwd() {
  Serial.println(F("command PWD received from CPU"));
  send_string_to_cpu(cd_path);
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

  unsigned long start_time = millis();

       if(data == CMD_READ)  comando_read();      
  else if(data == CMD_WRITE) comando_write();      
  else if(data == CMD_DEL)   comando_del();    
  else if(data == CMD_RMDIR) comando_rmdir();    
  else if(data == CMD_MKDIR) comando_mkdir();     
  else if(data == CMD_CD)    comando_cd();      
  else if(data == CMD_PWD)   comando_pwd();      
  else if(data == CMD_DIR)   comando_dir(data);    
  else if(data == CMD_LS)    comando_dir(data);
  else if(data == CMD_TEST) {
    while(!TIMEOUT) {
      int data = receive_byte_from_cpu();
      send_byte_to_cpu(data ^ 0xFF);
    }
  }
  else {
    Serial.print(F("unknown command "));
    Serial.print(data);
    Serial.println(F(" received"));
  }

  Serial.print(F("command processing time: "));
  Serial.println(millis() - start_time);
  
  if(TIMEOUT) Serial.println(F("TIMEOUT during command"));    
}
