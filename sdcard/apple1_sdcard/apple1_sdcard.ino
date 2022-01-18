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

#define CPU_CLOCK   9   /* cpu send data clock, VIA.PB0 => MCU */  
#define MCU_CLOCK  10   /* mcu send data clock, VIA.PB1 <= MCU */

// utility function for read a bit 
#define BIT(data,n) (((data) >> (n)) & 1)

void setup() {
  // debug on serial
  Serial.begin(9600);  
  
  pinMode(CPU_CLOCK, INPUT);
  pinMode(MCU_CLOCK, OUTPUT);
}

void loop() {
  // applicazione di esempio: manda un messaggio quando riceve il comando 42
  int data = receive_byte_from_cpu();

  if(data == 42) {
    Serial.println("command 42 received from CPU");
    char *msg = "HELLO WORLD!\r\n";
    for(int t=0; t<strlen(msg); t++) {
      send_byte_to_cpu(msg[t]);  
    }      
    send_byte_to_cpu(0);  // terminatore stringa
    Serial.println("message sento to CPU");
  }  
}

void wait(int pin, int value) {
  // TODO prevedere un meccanismo di timeout?
  while(digitalRead(pin) != value);  
}

void receive_byte_from_cpu() {
  
  // when here, both CPU_CLOCK and MCU_CLOCK are LOW
  
  // set data port as input
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  
  // tells CPU we are ready to receive
  digitalWrite(MCU_CLOCK, HIGH);   
  
  // wait data to be signaled as "ready" on the CPU  
  wait(CPU_CLOCK, HIGH);           

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

  // tells CPU data was received
  digitalWrite(MCU_CLOCK, HIGH);
  
  // wait the CPU to end operation
  wait(CPU_CLOCK, LOW);           
  
  // we also terminate
  digitalWrite(MCU_CLOCK, LOW);
  
  // when here, both CPU_CLOCK and MCU_CLOCK are LOW
  
  return byte;
}

void send_byte_to_cpu(int data) {

  // when here, both CPU_CLOCK and MCU_CLOCK are LOW
  
  // set data port as output
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  
  // wait until CPU is ready to receive (should be already)
  wait(CPU_CLOCK, LOW);

  // put byte on D0-D7
  digitalWrite(D0, BIT(data,0) );
  digitalWrite(D1, BIT(data,1) );
  digitalWrite(D2, BIT(data,2) );
  digitalWrite(D3, BIT(data,3) );
  digitalWrite(D4, BIT(data,4) );
  digitalWrite(D5, BIT(data,5) );
  digitalWrite(D6, BIT(data,6) );
  digitalWrite(D7, BIT(data,7) );
  
  // tells CPU byte is ready on the data port
  digitalWrite(MCU_CLOCK, HIGH);
  
  // wait for CPU to read the data
  wait(CPU_CLOCK, HIGH);
  
  // tells CPU byte we are finished
  digitalWrite(MCU_CLOCK, LOW);
  
  // wait for CPU end as well
  wait(CPU_CLOCK, LOW);
  
  // when here, both CPU_CLOCK and MCU_CLOCK are LOW    
}
