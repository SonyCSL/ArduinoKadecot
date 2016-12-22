
/*
 * Arduino sketch for Kadecot|JS (Default littleBits Arduino)
 * 
 *  参考：
 *  
 *
 */

#include <Arduino.h>

// This must be modified!!!
String UNIQUE_ID = "Arduino001234" ;
const int outPorts[] = {1,5,9} ;

String recvStr = "" ;
void setup() {
  Serial.begin(9600); // initialize serial:
  recvStr.reserve(200); // reserve 200 bytes for the recvStr

  sendSerial(String(UNIQUE_ID + "/in:0,1,2/out:0,1,2")) ;
  pinMode(0, INPUT);
  pinMode(outPorts[0], OUTPUT);
  pinMode(outPorts[1], OUTPUT);
  pinMode(outPorts[2], OUTPUT);
}

int prevD0 = -1 ;
int prevA0 = -1000 ;
int prevA1 = -1000 ;


void loop() {
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar == ';') {
        onSerial(recvStr) ;
        recvStr = "" ;
      } else recvStr += inChar;
    }
  
    int curD0 = digitalRead(0) ;
    int curA0 = analogRead(A0) ;
    int curA1 = analogRead(A1) ;

    if( curD0 != prevD0 ){
      sendSerial( String("pub:0:") + (curD0==HIGH ? 1 : 0) ) ;
      prevD0 = curD0 ;
    }

    if( abs( curA0 - prevA0 ) > 100 ){
      sendSerial( String("pub:1:")+(curA0/1023.0f) ) ;
      prevA0 = curA0 ;
    }

    if( abs( curA1 - prevA1 ) > 100 ){
      sendSerial( String("pub:2:")+(curA1/1023.0f) ) ;
      prevA1 = curA1 ;
    }
    
//    delay(1000);
}

void onSerial(String txt){
  // Split txt into command tokens

  String cmd[txt.length()] ;
  int cmd_len = 0 ;
  {  
    int idx=0 ;
    while(1) {
      int nidx = txt.indexOf(":",idx) ;
      cmd[cmd_len++] = txt.substring(idx,nidx) ;
      if( nidx == -1 ) break ;
      idx = nidx+1 ;
    }
  }

  if( cmd[0].equals("set") ){
    digitalWrite(outPorts[cmd[1].toInt()], cmd[2].toFloat() < 0.5f ? LOW : HIGH ) ;
  } else if( cmd[0].equals("get") ){
    float sensorValue = -1 ;
    switch( cmd[1].toInt() ){
      case 0 : sensorValue = (prevD0==HIGH?1:0) ; break ;
      case 1 : sensorValue = prevA0/1023.0f ; break ;
      case 2 : sensorValue = prevA1/1023.0f ; break ;
    }

    sendSerial( String("rep:")+sensorValue+":"+cmd[2] ) ;
  } else if( cmd[0].equals("init") ){
    sendSerial( String("oninit:")+UNIQUE_ID + "/in:0,1,2/out:0,1,2/mode:gpio/key:"+cmd[1]) ;
  }
}

void sendSerial(String txt){ Serial.print(txt+";") ; }

