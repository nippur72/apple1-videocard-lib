// pin definitions

// VIA 6522 connections

#define D0 1   /* I/O data bit 0, connects to PA0 on the VIA */
#define D1 2   /* I/O data bit 1, connects to PA1 on the VIA */
#define D2 3   /* I/O data bit 2, connects to PA2 on the VIA */
#define D3 4   /* I/O data bit 3, connects to PA3 on the VIA */
#define D4 5   /* I/O data bit 4, connects to PA4 on the VIA */
#define D5 6   /* I/O data bit 5, connects to PA5 on the VIA */
#define D6 7   /* I/O data bit 6, connects to PA6 on the VIA */
#define D7 8   /* I/O data bit 7, connects to PA7 on the VIA */

#define CPU_STROBE   9   /* PB0 => MCU  1=cpu byte is available on the data port; 0 cpu is waiting */  
#define MCU_STROBE  10   /* PB1 <= MCU  1=mcu byte is available on the data port; 0 mcu is waiting */

// indicates that a timeout occurred during wait()
int TIMEOUT = 0;

void setup() {
  // debug on serial
  Serial.begin(9600);  
    
  pinMode(CPU_STROBE, INPUT);
  pinMode(MCU_STROBE, OUTPUT);
  
  digitalWrite(MCU_STROBE, LOW);   
}

void loop() {
  // applicazione di esempio: manda un messaggio quando riceve il comando 42
  TIMEOUT = 0;

  int data = receive_byte_from_cpu();

  if(data == 42 && !TIMEOUT) {
    
    Serial.println("command 42 received from CPU");
    char *msg = "HELLO WORLD!\r\n";
    for(int t=0; t<strlen(msg); t++) {
      send_byte_to_cpu(msg[t]);  
    }      
    send_byte_to_cpu(0);  // terminatore stringa
    if(!TIMEOUT) Serial.println("message sent to CPU");
    else Serial.println("timeout during send to CPU");
  }  
}

void wait(int pin, int value) {
    
  unsigned long start_time = millis();  
  unsigned long elapsed;  

  if(TIMEOUT) break;
  
  while(digitalRead(pin) != value) {
    elapsed = millis() - start_time;
    if(elapsed > 500) {
      TIMEOUT = 1;
      break;
    }      
  }
}

int receive_byte_from_cpu() {
      
  // set data port as input
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);  

  // both strobes are 0   
  
  // CPU deposits data byte and sets strobe high
  wait(CPU_STROBE, HIGH);           
  
  // read the data byte
  int data = 
    (digitalRead(D0) << 0) |
    (digitalRead(D1) << 1) |
    (digitalRead(D2) << 2) |
    (digitalRead(D3) << 3) |
    (digitalRead(D4) << 4) |
    (digitalRead(D5) << 5) |
    (digitalRead(D6) << 6) |
    (digitalRead(D7) << 7);

  // after reading the byte, MCU sets strobe high
  digitalWrite(MCU_STROBE, HIGH);

  // CPU now sets strobe low
  wait(CPU_STROBE, LOW);

  // and MCU sets strobe low
  digitalWrite(MCU_STROBE, LOW);
    
  return data;
}

void send_byte_to_cpu(int data) {
  
  // set data port as output
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  // both strobes are 0
  
  // put byte on the data port
  digitalWrite(D0, data &   1);
  digitalWrite(D1, data &   2);
  digitalWrite(D2, data &   4);
  digitalWrite(D3, data &   8);
  digitalWrite(D4, data &  16);
  digitalWrite(D5, data &  32);
  digitalWrite(D6, data &  64);
  digitalWrite(D7, data & 128);
  
  // after depositing data byte, MCU sets strobe high
  digitalWrite(MCU_STROBE, HIGH);
  
  // wait for CPU to set strobe high
  wait(CPU_STROBE, HIGH);  
  
  // tells CPU byte we are finished
  digitalWrite(MCU_STROBE, LOW); 

  // wait for CPU to set strobe low
  wait(CPU_STROBE, LOW);  
}
