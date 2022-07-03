#include "Arduino.h"
#include <NTPtimeESP.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define API_KEY "AIzaSyBVJPTTo4w-fDWstUfmn_2PmRBIAHOXBWA"
#define DATABASE_URL "autotree-minh-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "caohoangminh1801@gmail.com"
#define USER_PASSWORD "minhkk"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

//const char* ssid = "EGROUPS VIET NAM TANG 2";
//const char* password = "Egvn123!@#";
//const char* ssid = "EGGROUPS VIETNAM";
//const char* password = "egvn123!@#";
//const char* ssid = "MinhMB";
//const char* password = "12341234";

const char* ssid = "FetelNy";
const char* password = "123bon231";

#define den  27         //En8
#define quat  26        //En9
#define phunsuong 33    //En12
#define bom  32         //En13
#define t_sen 4         //I7
#define h_sen1 34        //I6
#define h_sen2 35        //I5

int ibom, iquat, iphunsuong, jphunsuong, actualMinute, actualHour, DKbom, DKquat, DKphunsuong, QQ, DD, MbMb, PsPs;
int chech;
float t, h, h1, h2;
String mystr, mystr2, myphut, mygio, thgian;
String Qon, Qoff, QtimeOn, QtimeOff, DtimeOn, DtimeOff, MBon, MBoff, MBtimeOn, MBtimeOff, PStimeOn, PStimeOff, PSTon, PSToff, PSHon, PSHoff;

NTPtime NTPch("pool.ntp.org");
strDateTime dateTime;
OneWire oneWire(t_sen);
DallasTemperature sensors(&oneWire);

////////////////////////////////////////////////////////////////////////////////////////////////

void initWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  pinMode(quat, OUTPUT);
  pinMode(den, OUTPUT);
  pinMode(bom, OUTPUT);
  pinMode(phunsuong, OUTPUT);
  
  initWiFi();
  delay(100);
  
  configF.api_key = API_KEY;
  configF.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);   
  sensors.begin();
  chech = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  if (!Firebase.ready()) 
  {
      Serial.println("Firebase Error");
      Firebase.reconnectWiFi(true);
  }
  sensors.requestTemperatures();
  t = sensors.getTempCByIndex(0);
  h1 = getSoilMoist1();
  h2 = getSoilMoist2();

  h = (h1+h2)/2;  
  if (t == -127)
  {
      return; 
  }
  
  if (isnan(t) || isnan(h1) || isnan(h2)) 
  {
    Serial.println("Mat ket noi sensor");
  } 
  else 
  {
    Firebase.RTDB.setFloat(&fbdo, "/DHT/data/nhietDo", t);
    Firebase.RTDB.setFloat(&fbdo, "/DHT/data/doAm", h);
    Serial.print("do am h1: ");
    Serial.println(h1);
    Serial.print("do am h2: ");
    Serial.println(h2);
    Serial.print("nhiet do t: ");
    Serial.println(t);
  }

  hamthoigian();

  if(chech == 1)
  {
    docDATA();
    Firebase.RTDB.setInt(&fbdo, "chech/chech", 0);
  }
  if (Firebase.RTDB.getInt(&fbdo, "chech/chech")) 
  {
    chech = fbdo.intData();
  }
  
  hamxulyQUAT();
  hamxulyDEN();
  hamxulyMB();
  hamxulyPS();
  
  if (Firebase.RTDB.getInt(&fbdo, "Quat/Quat")) 
  {
    QQ = fbdo.intData();
  }
  if (Firebase.RTDB.getInt(&fbdo, "Den/Den")) 
  {
    DD = fbdo.intData();
  }
  if (Firebase.RTDB.getInt(&fbdo, "MayBom/MayBom")) 
  {
    MbMb = fbdo.intData();
  }
  if (Firebase.RTDB.getInt(&fbdo, "PhunSuong/PhunSuong")) 
  {
    PsPs = fbdo.intData();
  }
  
  DKthietbi();
}

///////////////////////////////////////////////////////////////////////////////////////////

int getSoilMoist1()
{
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  
  {
    anaValue += analogRead(h_sen1);     //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 4095, 0, 0, 100);      //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue;
}

int getSoilMoist2()
{
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  
  {
    anaValue += analogRead(h_sen2);     //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 4095, 0, 0, 100);      //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue;
}

/////////////////////////////////////////////////////////////////////////

void hamthoigian()
{
  dateTime = NTPch.getNTPtime(7.0, 0);  
  if(dateTime.valid)
  {
    NTPch.printDateTime(dateTime);    
    actualHour = dateTime.hour;           
    actualMinute = dateTime.minute;       
  }
  delay(100);
  if(actualHour < 10)
  {
    mygio = String(actualHour);
    mystr = String("0" + mygio);
  }
  else
  {
    mystr = String(actualHour);
  }
  mystr2 = String(mystr + ":");
  delay(50);
  if(actualMinute < 10)
  {
    myphut = String(actualMinute);
    mystr = String("0" + myphut);
  }
  else  
  {
    mystr = String(actualMinute);
  }
  thgian = String(mystr2 + mystr);
}

///////////////////////////////////////////////////////////////////////////////////////////

void docDATA()
{
  if (Firebase.RTDB.getString(&fbdo, "Quat/NhietDo/NhietDoBat")) 
  {
    Qon = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "Quat/NhietDo/NhietDoTat")) 
  {
    Qoff = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "Quat/HenGio/HenGioBat")) 
  {
    QtimeOn = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "Quat/HenGio/HenGioTat")) 
  {
    QtimeOff = fbdo.stringData();
  }
///////////////////////////////////////////////////////////////////////////////////////////
  if (Firebase.RTDB.getString(&fbdo, "MayBom/NhietDo/NhietDoBat")) 
  {
    MBon = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "MayBom/NhietDo/NhietDoTat")) 
  {
    MBoff = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "MayBom/HenGio/HenGioBat")) 
  {
    MBtimeOn = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "MayBom/HenGio/HenGioTat")) 
  {
    String MBtimeOff = fbdo.stringData();
  }
///////////////////////////////////////////////////////////////////////////////////////////
  if (Firebase.RTDB.getString(&fbdo, "Den/HenGio/HenGioBat")) 
  {
    DtimeOn = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "Den/HenGio/HenGioTat")) 
  {
    DtimeOff = fbdo.stringData();
  }
///////////////////////////////////////////////////////////////////////////////////////////    
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/NhietDo/NhietDoBat")) 
  {
    PSTon = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/NhietDo/NhietDoTat")) 
  {
    PSToff = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/DoAm/DoAmBat")) 
  {
    PSHon = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/DoAm/DoAmTat")) 
  {
    PSHoff = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/HenGio/HenGioBat")) 
  {
    PStimeOn = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo, "PhunSuong/HenGio/HenGioTat")) 
  {
    PStimeOff = fbdo.stringData();    
  }
  Serial.println("Da doc xong yeu cau hoat dong moi");
}

///////////////////////////////////////////////////////////////////////////////////////////

void hamxulyQUAT()
{
  if(thgian == QtimeOn)
  {
    Firebase.RTDB.setInt(&fbdo, "Quat/Quat", 1);
    Firebase.RTDB.setInt(&fbdo, "Quat/DKQuat", 1);
  }
  if(thgian == QtimeOff)
  {
    Firebase.RTDB.setInt(&fbdo, "Quat/Quat", 0);
    Firebase.RTDB.setInt(&fbdo, "Quat/DKQuat", 0);
  }

  if (Firebase.RTDB.getInt(&fbdo, "Quat/DKQuat")) 
  {
    DKquat = fbdo.intData();
  }
  if(DKquat == 0)
  {
    if (Firebase.RTDB.getInt(&fbdo, "Quat/iquat")) 
    {
      iquat = fbdo.intData();
    }
    switch(iquat)
    {
      case 0:
        if(t>Qon.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "Quat/Quat", 1);
          Firebase.RTDB.setInt(&fbdo, "Quat/iquat", 1);
          break;
        }
        break;
      case 1:
        if(t<Qoff.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "Quat/Quat", 0);
          Firebase.RTDB.setInt(&fbdo, "Quat/iquat", 0);
          break;
        }
        break;
      default: 
        Serial.println("Loi dieu khien Quat");
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

void hamxulyDEN()
{
  if(thgian == DtimeOn)
  {
    Firebase.RTDB.setInt(&fbdo, "Den/Den", 0);
  }  
  if(thgian == DtimeOff)
  {
    Firebase.RTDB.setInt(&fbdo, "Den/Den", 1);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

void hamxulyMB()
{
  if(thgian == MBtimeOn)
  {
    Firebase.RTDB.setInt(&fbdo, "MayBom/MayBom", 1);
    Firebase.RTDB.setInt(&fbdo, "MayBom/DKBom", 1);
  }
  if(thgian == MBtimeOff)
  {
    Firebase.RTDB.setInt(&fbdo, "MayBom/MayBom", 0);
    Firebase.RTDB.setInt(&fbdo, "MayBom/DKBom", 0);
  }

  if (Firebase.RTDB.getInt(&fbdo, "MayBom/DKBom")) 
  {
    DKbom = fbdo.intData();
  }
  if(DKbom == 0)
  {
    if (Firebase.RTDB.getInt(&fbdo, "MayBom/ibom")) 
    {
      ibom = fbdo.intData();
    }
    switch(ibom)
    {
      case 0:
        if(t>MBon.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "MayBom/MayBom", 1);
          Firebase.RTDB.setInt(&fbdo, "MayBom/ibom", 1);
          break;
        }
        break;
      case 1:
        if(t<MBoff.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "MayBom/MayBom", 0);
          Firebase.RTDB.setInt(&fbdo, "MayBom/ibom", 0);
          break;
        }
        break;
      default: 
        Serial.println("Loi dieu khien May Bom");
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

void hamxulyPS()
{
  if(thgian == PStimeOn)
  {
    Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 1);
    Firebase.RTDB.setInt(&fbdo, "PhunSuong/DKPhunSuong", 1);
  }
  if(thgian == PStimeOff)
  {
    Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 0);
    Firebase.RTDB.setInt(&fbdo, "PhunSuong/DKPhunSuong", 0);
  }

  if (Firebase.RTDB.getInt(&fbdo, "PhunSuong/DKPhunSuong")) 
  {
    DKphunsuong = fbdo.intData();
  }
  if(DKphunsuong == 0)
  {
    if (Firebase.RTDB.getInt(&fbdo, "PhunSuong/iPhunSuong")) 
    {
      iphunsuong = fbdo.intData();
    }
    switch(iphunsuong)
    {
      case 0:
        if(t>PSTon.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 1);
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/iPhunSuong", 1);
          break;
        }
        if(h<PSHon.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 1);
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/iPhunSuong", 2);
          break;
        }
        break;
      case 1:
        if(t<PSToff.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 0);
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/iPhunSuong", 0);
          break;
        }
        break;
      case 2:
        if(h>PSHoff.toInt())
        {
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/PhunSuong", 0);
          Firebase.RTDB.setInt(&fbdo, "PhunSuong/iPhunSuong", 0);
          break;
        }
        break;
      default: 
        Serial.println("Loi dieu khien Phun Suong");
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////

void DKthietbi()
{
  switch(QQ)
  {
    case 0:
      Serial.println("Quat turned OFF");
      digitalWrite(quat, LOW);
      break;
    case 1:
      Serial.println("Quat turned ON");
      digitalWrite(quat, HIGH);
      break;
    default:
      break;
  }

  switch(DD)
  {
    case 0:
      Serial.println("Den turned OFF");
      digitalWrite(den, LOW);
      break;
    case 1:
      Serial.println("Den turned ON");
      digitalWrite(den, HIGH);
      break;
    default:
      break;
  }

  switch(MbMb)
  {
    case 0:
      Serial.println("May Bom turned OFF");
      digitalWrite(bom, LOW);
      break;
    case 1:
      Serial.println("May Bom turned ON");
      digitalWrite(bom, HIGH);
      break;
    default:
      break;
  }

  switch(PsPs)
  {
    case 0:
      Serial.println("Phun Suong turned OFF");
      digitalWrite(phunsuong, LOW);
      break;
    case 1:
      Serial.println("Phun Suong turned ON");
      digitalWrite(phunsuong, HIGH);
      break;
    default:
      break;
  }
}
