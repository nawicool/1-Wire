/* 1-Wire DS18B20 program*/
/*Pin 2 of Arduino*/
#define PIN_ATTACHED 2 
//PIN on Arduino for the communication
byte num_devices=1, alarm_devices, CRC_Error;
//num of devices is the number of devices found.  A value of 10 means 10 found, and they start at 1
float tempC_decimal=0.0000, tempF_decimal=0.0000;
//These temps will hold the last value read from the sensor
byte LSB, MSB, TH, TL, Config;
//These are the RAW bytes read out of the sensor, and are last read
byte TempSensor1[8]={0x28,0xBD,0xA2,0xA0,0x5,0x0,0x0,0x7F};
//if you want to assign your addresses, and not use the search function



void setup(){
//setup  start   setup  start   setup  start   setup  start   setup  start   setup  start
  Serial.begin(115200);//note! you will always need to include serial communication even if you don't use it
  //go through the code and remove serial.print commands if you don't need it
}

void loop(){
  
  // LOOP START    LOOP START    LOOP START    LOOP START    LOOP START    LOOP START
  DSB_get_temp_one_device(PIN_ATTACHED,1);//pin, print
}


void DSB_get_temp_one_device(byte pin, byte printer){//   GET TEMP ONE START START
  int i;
//this function gets the temperature of a device if only one device is on the bus

  LSB=0;
  MSB=0;
  tempC_decimal =0;
  DSB_Reset(pin);//reset-----
  DSB_byte_write(0xCC, pin);//skip the ROM address, since there is only one device
  DSB_byte_write(0x44, pin);//convert the temp
  delay(750);//hang out for 750ms to let the conversion complete
  DSB_Reset(pin);//reset again
  DSB_byte_write(0xCC, pin);//skip again
  DSB_scratch_read(pin);//then read out the scratch pad, which includes the temp

if(printer==1){//will print out here if desired
Serial.print(tempC_decimal, 4);
Serial.print("C ");
Serial.print(tempF_decimal, 4);
Serial.println("F ");
}

}//GET TEMP ONE END END



void DSB_Reset(byte pin){
/*
During the initialization sequence the bus master transmits (T X ) the reset pulse by pulling the 1-Wire bus
low  for a  minimum of 480us. The  bus  master then releases the  bus and goes  into receive  mode (R X ).
When the bus is released, the 5kohm pullup resistor pulls the 1-Wire bus high. When the DS18B20 detects
this rising edge, it waits 15us to 60us and then transmits a presence pulse by pulling the 1-Wire bus low
for 60us to 240us.
*/
// RESET  START START
//this is needed before every transaction with the DS18B20

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(500);//drive the line low for 500us
  pinMode(pin, INPUT);//then let it pull-up for 500us
  delayMicroseconds(500);//PRESENCE pulse occurs here, but we don't care about it
}//                       RESET  END END






void DSB_byte_write(byte command, byte pin){
/*
Read/Write Time Slot Timing Diagram */    
    
    //    BYTE WRITE START START
int i;// don't hate on my use of i everywhere!  I know, I know bad programming Kevin!

//We need to write a BYTE "command" to the digital pin "pin"

for(i=0; i<8; i++){//8 bits in a byte, do this 8 times, least significant bit
   if(bitRead(command, i)){//if the first bit is a '1'
    pinMode(pin, OUTPUT);//get ready to drive LOW
    digitalWrite(pin, LOW);//go low for a microsecond, just to let the divice know something is coming
    delayMicroseconds(1); //might get rid of this, probably not needed
    pinMode(pin, INPUT);//since this is a '1', change into an input, and let the pull-up resistor yank the line HIGH
    delayMicroseconds(60);//chill out for 60micro secs, to let the DS18B20 get a read of the high signal
   }//true
   else{//nope it was a '0'
    pinMode(pin, OUTPUT);//get ready to write
    digitalWrite(8, LOW);//go LOW for the entire 60ish microseconds
    delayMicroseconds(59);//the DS18B20 will come in and read the low line as a 0 here
    pinMode(pin, INPUT);//back to input, line gets yanked high, and ready for the next bit
    delayMicroseconds(1);
   }//else

  }//for i=0
}////    BYTE WRITE END END




byte DSB_byte_read(byte pin){//                  BYTE READ START START
  int i;
  byte byte_in;

  //reads a byte from the DS18B20, this is not a 'void' function so it returns the byte read in

 for(i=0; i<8; i++){//8 bits in a byte
    pinMode(pin, OUTPUT);//get ready
    digitalWrite(pin, LOW);//drive low to tell the DS18B20 we're ready
    //used to have a delay of 1us here, turned out it was giving corrupt data
   pinMode(pin, INPUT);// change to an input to allow line to yank HIGH
   delayMicroseconds(10);//The DS18B20 now has control over the line
   //if pulled low, then we read in a '0', if high, then the bit is a '1'
   if(digitalRead(pin))//grab the line status
   bitSet(byte_in,i);//set it or...
   else
   bitClear(byte_in,i);//clear it
   delayMicroseconds(49);//wait out the rest of the timing window
 }//for

  return(byte_in);

}////    BYTE READ END END




void DSB_scratch_read(byte pin){//                    SCRATCH READ START START
  int i, j;
  byte data_in[9], CRC=0;
  boolean CRCxor;
  tempC_decimal=0;

  //things are about to get a little crazy here
  //this is where the scratchpad is read out and checked against the CRC
  //the LSB MSB of the temperature is converted into a float value


  DSB_byte_write(0xBE, pin);//read scratch
for(i=0; i<9; i++){//9 bytes are read in
  data_in[i]=DSB_byte_read(pin);//keep it all in data_in[0..8]
  }

 for(i=0; i<8; i++){//CRC checker, 8 bytes wide, 9th byte is the CRC
   for(j=0; j<8; j++){//8 bits each
   CRCxor = bitRead(data_in[i], j) ^ bitRead(CRC, 0);//XOR CRC bit 0 with the incoming bit
   bitWrite(CRC, 3, (CRCxor^bitRead(CRC, 3))); //take that XOR result and XOR it with the 3rd bit in the CRC
   bitWrite(CRC, 4, (CRCxor^bitRead(CRC, 4)));//do the same with the 4th bit
   CRC = CRC>>1;//shift the whole CRC byte over to the right once
   bitWrite(CRC, 7, CRCxor);//now store the XOR bit into the last position of the CRC byte
   }//this goes on for the entire 8 bytes
  }
 //Serial.println(CRC, HEX);//print stuff if you want to see the results
 //Serial.println(data_in[8], HEX);
  if(CRC==data_in[8]){//okay, now check the CRC you made with the 9th byte that came in
  //if we're good, go ahead and set everybody up
    LSB=data_in[0];
    MSB=data_in[1];
    TH=data_in[2];
    TL=data_in[3];
    Config=data_in[4];
   CRC_Error=0;
  }
  else//if not, set everybody to 0
  {
    CRC_Error=1;//ALSO - set a flag, you can use this to see if there was a fail
      LSB=0;
    MSB=0;
    TH=0;
    TL=0;
    Config=0;
  }


  if(bitRead(MSB, 7)==0){//check to see if the temp is negative
  //positive temperature

  for(i=0; i<4; i++){//first set the fractional part of the number
  if(bitRead(LSB,i)==0)//only the bottom 4 bits represent 0.5, 0.25, 0.125, 0.0625
   tempC_decimal = tempC_decimal+pow(2.0000, -4.0000+i);//yes, this is weird, but it makes sense,
  // 2^-1 = 0.5, 2^-2=0.25, 2^-3=0.125, and 2^-4=0.0625
  }//for


//tempC is the fractional part right now, so we add that to the integer part
//the top 4 of LSB are 2^0, 2^1... so we shift that to the right 4 times to get it lined up
//then the MSB only contains 3 contributing bits, so we AND off everything else
//then shift that to the left 4 times to line it up with the LSB we just shifted over
tempC_decimal = tempC_decimal + (LSB>>4) + ((B00000111 & MSB)<<4);
//convert to F, for those of us who can't use Celsius :)
tempF_decimal = (tempC_decimal)*9.0000/5.0000+32.00000;}
else
{//or..... if the temp is NEGATIVE

//the negative temp comes in as a 2's comliment number, so we need to convert it back

//I did this kinda weird, but I combined the LSB with MSB into a word, then inverted everything,
// by using a ^ against 0xFFFF, then added '1'.  This converts back to what we were dealing with
//if the number was positive
word full16 = ((((B00000111 & MSB)<<8) + LSB) ^ 0xFFFF)+1;
MSB = full16>>8;//but, I already had my code ready to work with MSB and LSB, so I pulled them back out
LSB=  full16 & 0x00FF;

  //now we're back to normal, so I pulled the same trick as before
  for(i=0; i<4; i++){
  if(bitRead(LSB,i))
   tempC_decimal = tempC_decimal+pow(2.0000, -4.0000+i);
  }//for

//and this is the same as before, now with a '-1 multiplier'
tempC_decimal = (tempC_decimal + (LSB>>4) + ((B00000111 & MSB)<<4))*-1;
tempF_decimal = (tempC_decimal)*9.0000/5.0000+32.00000;

}//else neg

}//                    SCRATCH READ END END













