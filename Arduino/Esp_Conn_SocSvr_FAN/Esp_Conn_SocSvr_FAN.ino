#include <SoftwareSerial.h>

#define _rxpin      5
#define _txpin      6
#define FANpin      7

#define Ssid  "\"abc12345\""
#define Pasw  "\"qqqq1111\""
String ConnectWifi;
byte CommandClear_count=0 , ConnectServer_count=0;
SoftwareSerial esp8266(5,6); // RX 5, TX 6
void setup() {
  // put your setup code here, to run once:
  ConnectWifi = "AT+CWJAP_CUR=";
  ConnectWifi += Ssid;
  ConnectWifi += ",";
  ConnectWifi += Pasw;
  Serial.begin(9600);
  esp8266.begin(9600);
  delay(2000);
  pinMode(13, OUTPUT);
  pinMode(FANpin, OUTPUT);
  digitalWrite(FANpin , LOW);
  digitalWrite(13 , HIGH);
  InitSetting();
}

void loop() {
  // put your main code here, to run repeatedly:
   ConnectServer();
}

void Sendboth( String Str ){
  esp8266.println(Str);
  esp8266.flush();
  Serial.println("SEND:"+Str);
  Serial.flush();
}


String GetResponse( int dly ) {
  String response="";
  char c;
Wait:
dly-=1;
  while (esp8266.available()) {
    c=esp8266.read();
    response.concat(c);
    delayMicroseconds(900);
    }
  response.trim();
  delay(10);
if(dly>0) goto Wait;
  return response;
}


/*ConnectServerSetting------------------------------------------------------------------------------------------*/
bool ConnectServer(){
  switch(ConnectServer_count){
    case 0:
      Sendboth(ConnectWifi);
      if(ConnectServerDebug( "ConnectWIFI" ,"WIFI GOT IP" , "OK", 150 ))ConnectServer_count++;
    break;
    case 1:
      Sendboth("AT+CIPSTART=\"TCP\",\"106.104.139.182\",8881");
      if(ConnectServerDebug( "ConnectSocketServer" ,"OK" , "+IPD,5:*/*", 20 ))ConnectServer_count++;
    break;
    case 2:
      SendStatus();
    break;
    case 3:
      GetServerControl();
    break;
  }
}

void GetServerControl(){
  String Str = "";
  if(ConnectServerDebug( "GetControl" ,"TrunOn" , "TrunOff", 2 )){
    digitalWrite( FANpin,!digitalRead(FANpin) );
    if( digitalRead(FANpin) == HIGH ){
      Str = "OkyON";
      Sendboth( "AT+CIPSEND=" + ( (String) Str.length() ) );
      if(ConnectServerDebug( "SendTypeLength" ,"OK" , ">", 2 )){
        Sendboth( Str );
      }
    }
    else{
      Str = "OkyOFF";
      Sendboth( "AT+CIPSEND=" + ( (String) Str.length() ) );
      if(ConnectServerDebug( "SendTypeLength" ,"OK" , ">", 2 )){
        Sendboth( Str );
      }
    }
  }
}

void SendStatus(){
  String Type = "*/*fan1\r\n";
  String Type_length = ( "AT+CIPSEND=" + ( (String) Type.length() ) );
  bool b = digitalRead(FANpin);
  String Status = ( b ? "nowtrue\r\n" : "nowfalse\r\n" );
  String Status_length = ( "AT+CIPSEND=" + ( (String) Status.length() ) );
  
  Sendboth(Type_length);
  if(ConnectServerDebug( "SendTypeLength" ,"OK" , ">", 5 )){
    
    delay(100);
    Sendboth(Type);
    if(ConnectServerDebug( "SendType" ,"SEND OK" , "+IPD,8:Get it", 10 )){
      
      delay(50);
      Sendboth(Status_length);
      if(ConnectServerDebug( "SendStatusLength" ,"OK" , ">", 5 )){
        
        delay(100);
        Sendboth(Status);
        if(ConnectServerDebug( "SendStatus" ,"SEND OK" , "+IPD,8:Get it", 10 )){
          ConnectServer_count++;
        }
        else{
          Sendboth("AT+CIPCLOSE");
          ConnectServer_count = 1;
        }
        
      }
      else{
        Sendboth("AT+CIPCLOSE");
        ConnectServer_count = 1;
      }
      
    }
    else{
      Sendboth("AT+CIPCLOSE");
      ConnectServer_count = 1;
    }
    
  }
  else{
    Sendboth("AT+CIPCLOSE");
    ConnectServer_count = 1;
  }
  
}

bool ConnectServerDebug( String Str ,String chk1 , String chk2, byte dly ){
  String str = "";
  bool b;
  Serial.print("Check \'" + Str + "\' response ...");
  delay(dly);
  String res = GetResponse(dly*5);
  Serial.println("\n"+res);
  if(res.indexOf(chk1) != -1 | res.indexOf(chk2) != -1){
      Serial.println("OK");
      digitalWrite(13 , LOW);
      b = true;
  }
  else if(res.indexOf("no ip") != -1 | res.indexOf("WIFI DISCONNECT") != -1| res.indexOf("FAIL") != -1){
    ConnectServer_count=0;
    digitalWrite(13 , HIGH);
    b=false;
  }
  else if( res.indexOf("LREADY CONNECTED") != -1 ){
    b=true;
  }
  else if(res.indexOf("link is not valid") != -1){
    ConnectServer_count=1;
    digitalWrite(13 , HIGH);
    b=false;
  }
  else if(res.indexOf("CLOSED") != -1){
      ConnectServer_count=1;
      digitalWrite(13 , HIGH);
      b=false;
    }
  else{
    Serial.println("Error");
    b = false;
  }
  Serial.flush();
  return b;
}
/*ConnectServerSettingEnd------------------------------------------------------------------------------------------*/


/*InitSetting------------------------------------------------------------------------------------------*/
void InitSetting(){
    while(CommandClear());
}


bool CommandClear(){
  bool b = true;
  byte dly=50;
  switch(CommandClear_count){
    case 0:
      Sendboth("AT");
      if(Debug("AT","OK",dly))CommandClear_count++;
    break;
    case 1:
      Sendboth("AT+CWMODE=1");
      if(Debug("AT+CWMODE=1","OK",dly))CommandClear_count++;
    break;
    case 2:
      Sendboth("AT+CIPMODE=0");
      if(Debug("AT+CIPMODE=0","OK",dly))CommandClear_count++;
    break;
    case 3:
       Sendboth("AT+CIPCLOSE");
      if(Debug("AT+CIPCLOSE","ERROR",dly))CommandClear_count++;
    break;
    case 4:
      Sendboth("AT+CWQAP");
      if(Debug("AT+CIPMUX=0","OK",dly))CommandClear_count++;
    break;
    case 5:
      Sendboth("AT+CIPMUX=0");
      if(Debug("AT+CIPMUX=0","OK",dly)){
        CommandClear_count=0;
        b=false;
      }
    break;
    default:
      CommandClear_count=0;
    break;
  }
  return b;
}


bool Debug( String Str ,String chk, byte dly ){
  String str = "";
  bool b;
  Serial.print("Check \'" + Str + "\' response ...");
  delay(dly);
  String res = GetResponse(1);
//  Serial.println("\n"+res);
  if(res.indexOf(chk) != -1){
    Serial.println("OK");
    b = true;
  }
  else{
    Serial.println("Error");
    b = false;
  }
  Serial.flush();
  return b;
}
/*InitSettingEnd------------------------------------------------------------------------------------------*/




