
/*
 * An Arduino sketch to use the board as GPIO interface of Kadecot|JS
 * The default configuration is set as littleBits Arduino.
 * Kadecot|JS : https://github.com/SonyCSL/Kadecot-JS
 * Japanese instructions : http://qiita.com/sgrowd/items/9ef56370a49f4f10c96c
 *
 */

#include <Arduino.h>

// Constants. Do not modify.
const int DOFS = -100 , D0=0+DOFS,D1=1+DOFS,D2=2+DOFS,D3=3+DOFS,D4=4+DOFS,D5=5+DOFS,D6=6+DOFS,D7=7+DOFS,D8=8+DOFS,D9=9+DOFS,D10=10+DOFS,D11=11+DOFS,D12=12+DOFS,D13=13+DOFS,D14=14+DOFS,D15=15+DOFS ;

// If you connect two or more Arduinos, the following ID must be modified!!!
String UNIQUE_ID = "Arduino123456" ;

// If you prefer different I/O configurations, modify the following arrays.
const int inPorts[] = {D0,A0,A1} ;
const int outPorts[] = {D1,D5,D9} ;

// This threshold determines if input value change should be published or not.
// Should be less than 1.0f
const float PUBLISH_THR = 0.05f ;


// Please do not modify below.
const int inPortsLen = sizeof(inPorts)/sizeof(int) ;
const int outPortsLen = sizeof(outPorts)/sizeof(int) ;

float prevInVal[inPortsLen] ;

String recvStr = "" ;
void setup() {
  Serial.begin(9600); // initialize serial:
  recvStr.reserve(200); // reserve 200 bytes for the recvStr

  for( int i=0;i<inPortsLen;++i ){
    prevInVal[i] = -100 ;
    if( inPorts[i]>= 0 ) continue ;
    pinMode(inPorts[i]-DOFS, INPUT);
  }
  for( int i=0;i<outPortsLen;++i ){
    if( outPorts[i]>= 0 ) continue ;
    pinMode(outPorts[i]-DOFS, OUTPUT);
  }
}

void loop() {
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar == ';') {
        onSerial(recvStr) ;
        recvStr = "" ;
      } else recvStr += inChar;
    }

    float curInVal[inPortsLen] ;
    for( int i=0;i<inPortsLen;++i ){
      if( inPorts[i]>=0 )  curInVal[i] = analogRead(inPorts[i])/1023.0f ;
      else                 curInVal[i] = (digitalRead(inPorts[i]-DOFS)==HIGH?1:0) ;

      if( abs( prevInVal[i] - curInVal[i] ) >= PUBLISH_THR ){
        sendSerial( String("pub:")+i+":" + curInVal[i] ) ;
        prevInVal[i] = curInVal[i] ;
      }
    }
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
    int outPort = outPorts[cmd[1].toInt()] ;
    if( outPort < 0 ) digitalWrite( outPort - DOFS , cmd[2].toFloat() < 0.5f ? LOW : HIGH ) ;
    else              analogWrite( outPort , (int)( cmd[2].toFloat() * 1023.999) ) ;
  } else if( cmd[0].equals("get") ){
    sendSerial( String("rep:")+prevInVal[cmd[1].toInt()]+":"+cmd[2] ) ;
  } else if( cmd[0].equals("init") ){
    String sendstr = String("oninit:")+UNIQUE_ID ;
    if( inPortsLen > 0 ){
      sendstr += "/in" ;
      for( int i=0;i<inPortsLen;++i ) sendstr += String(i==0?":":",") + i ;
    }
    if( outPortsLen > 0 ){
      sendstr += "/out" ;
      for( int i=0;i<outPortsLen;++i ) sendstr += String(i==0?":":",") + i ;
    }
    sendstr += "/mode:gpio" ;

    sendSerial(sendstr) ;
  }
}

void sendSerial(String txt){ Serial.print(txt+";") ; }

