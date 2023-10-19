// 寝ている子どもの動きを確認するプログラム

#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h> //library manager 
#include <DallasTemperature.h>

const char* ssid = "AP01-01";
const char* password =  "1qaz2wsx";

const String endpoint = "https://maker.ifttt.com/trigger/";//定型url
const String eventName = "SchooMyIoT";//IFTTTのEvent Name
const String eventName2 = "SchooMyIoT2";//IFTTTのEvent Name
const String conn = "/with/key/";//定型url
const String Id = "SpluMUtU_DTXM5clG3iYt";//自分のIFTTTのYour Key

#define imp_pin 13 //imp OUTB:13
#define SoundSensor 32 //sound INA:33 INB:32
#define led_pin 26 //OUTA:26 

// 温度
#define ONE_WIRE_BUS 33 //温度　INA:33 INB:32
#define SENSER_BIT    9      // 精度の設定bit

#define red 26 
#define green 25
#define blue 27
  
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int val;
int SoundValue ;

void setup() {
  // 振動
  Serial.begin(115200);
  pinMode(imp_pin,INPUT);
  pinMode(led_pin,OUTPUT);

  // 音量
  pinMode (SoundSensor, INPUT);

  // wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // LED
  sensors.setResolution(SENSER_BIT);

  // LED 使用するタイマーのチャネル,周波数,デューティ比を設定
  ledcSetup(0, 12800, 8);
  ledcSetup(1, 12800, 8);
  ledcSetup(2, 12800, 8);
  // 各色を各チャネルへ接続
  ledcAttachPin(red, 0);
  ledcAttachPin(green, 1);
  ledcAttachPin(blue, 2);
}

// 振動
int SensAve(){
  int i;
  int sval=0;
  int ave=0;

  for (i = 0; i < 10000; i++){
    sval = sval + digitalRead(imp_pin);  // アナログ入力ピンのセンサから
  }

  ave = sval / 100;
  return ave;
}

void loop() {
  
  // 音量
  SoundValue=analogRead(SoundSensor);
  Serial.print("音量：");
  Serial.println(SoundValue);

  // 振動
  val = SensAve();
  Serial.print("振動：");
  Serial.println(val);

  // 温度
  Serial.print("温度：");
  sensors.requestTemperatures();              // 温度取得要求
  Serial.println(sensors.getTempCByIndex(0)); //温度の取得&シリアル送信
  int temp = sensors.getTempCByIndex(0);

  Serial.println("---------------------------");

  //if(30 <= temp){                 //30℃以上の場合(危険)
  if(32 <= temp){
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 0);
    delay(100);
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 255);
    delay(50);
  }

  //if(26 <= temp && temp < 30){   　//26～29℃の場合(高温)
  if(31 <= temp && temp < 32){    
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 0);
  }

  //if(18 <= temp && temp < 26){    //18～25℃の場合(適温)
  if(28 <= temp && temp < 31){   
    ledcWrite(0, 0);
    ledcWrite(1, 255);
    ledcWrite(2, 255);
  }

  //if(temp < 18){                  //17℃以下の場合(低温)
  if(temp < 28){    
    ledcWrite(0, 255);
    ledcWrite(1, 0);
    ledcWrite(2, 255);
  }


  if ((WiFi.status() == WL_CONNECTED)) {

    // 振動感知
    if(val>0 && val<90) //val<xxのxxが感度。0～100
    { 
      HTTPClient http;

      String value ="?value1=";   //値 value1=xxxx 
      value = value + String(temp);
 
      http.begin(endpoint + eventName + conn + Id + value); //URLを指定
      int httpCode = http.GET();  //GETリクエストを送信
 
      if (httpCode == 200) { //返答がある場合
        Serial.println("異常を検知 LINEを送信");
      }else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
    }

    // 音量感知
    if(1910 <= SoundValue) 
    { 
      HTTPClient http;

      String value2 ="?value1=";   //値 value1=xxxx 
      value2 = value2 + String(temp);
 
      http.begin(endpoint + eventName2 + conn + Id + value2); //URLを指定
      int httpCode = http.GET();  //GETリクエストを送信
 
      if (httpCode == 200) { //返答がある場合
        Serial.println("異常を検知 LINEを送信");
      }else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
    }
  }  
  delay(500);
}

// 30℃以上(危険)　　赤点滅
// 26～29℃(高温) 　 赤
// 18～25℃(適温) 　 緑
// 17℃以下(低温)　　青