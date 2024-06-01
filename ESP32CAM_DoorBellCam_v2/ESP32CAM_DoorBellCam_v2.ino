//Please don't detele this lines
//This code from TAD3DPrinted
//Youtube channel: https://www.youtube.com/channel/UC-n38zIe3MdZ_U63ipnHFxA
//Facebook: https://www.facebook.com/NTechCheTao/
//email: buncalockg@gmail.com


#define FLASH_LED_PIN 4
#define PIN_BELL_BUTTON 15

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

#include "Config.h"
#include "NESP32CAMCamera.h"
#include "NTelegramBot.h"

//#define LANG_EN
#define LANG_VN

//const char* ssid = "ABC"; //Name your wifi
//const char* password = "AZ123456^"; //Pass your wifi

// Initialize Telegram BOT
String BOTtoken = ""; //"5537275447:AAFSicOD2rpUNT2XlMZYGcJ3Q9uimCbkZQE";  // your Bot Token (Get from Botfather)
//String CHAT_ID = "5342644132";
String CHAT_ID1 = "";
String CHAT_ID2 = "";
String CHAT_ID3 = "";
// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
bool ID1_RequestPhoto = false;
bool ID2_RequestPhoto = false;
bool ID3_RequestPhoto = false;


NESP32CAMCamera myCam;
NTelegramBot nBot;

bool sendPhoto = false;
bool IsFlashOn = true;
bool IsAP;

#ifdef LANG_EN
String welcome_str = R"rawliteral(
Hello, <name>
You can execute the following commands
/photo : Take a photo
/flash : Flash on/off when taking pictures

=== Copyright by A2TEC ===
https://www.youtube.com/channel/UC9-l_lXvByF7wMRT-MxJiQg
---
https://www.facebook.com/linhkienchetaoA2Tec
)rawliteral";
String msg1_str = "<repstr> flash mode when taking pictures";
#endif

#ifdef LANG_VN
String welcome_str = R"rawliteral(
Xin chào, <name>
Bạn có thể thực hiện các lệnh sau
/photo : Chụp ảnh
/flash : Bật/tắt đèn flash khi chụp ảnh

=== Bản quyền của A2TEC ===
https://www.youtube.com/channel/UC9-l_lXvByF7wMRT-MxJiQg
---
https://www.facebook.com/linhkienchetaoA2Tec
)rawliteral";
String msg1_str = "<repstr> chế độ flash khi chụp ảnh";
#endif

String index_html_str = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Wifi: <input type="text" name="wifiname" value = "<wifiname>"> <br>
    Pass: <input type="text" name="wifipass" value = "<wifipass>"> <br>
    Bot Token: <input type="text" name="bottoken" value = "<bottoken>"> <br>
    Chat ID1: <input type="text" name="chatid1" value = "<chatid1>"> <br>
    Chat ID2: <input type="text" name="chatid2" value = "<chatid2>"> <br>
    Chat ID3: <input type="text" name="chatid3" value = "<chatid3>"> <br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void processMessage(telegramMessage &msg){

    String msg_chat_id = String(msg.chat_id);
    if(msg_chat_id != CHAT_ID1 && msg_chat_id != CHAT_ID2 && msg_chat_id != CHAT_ID3){
      nBot.SendMessage(msg_chat_id, "Unauthorized user");      
    }
    else{
      // Print the received message
      String text = msg.text;
      Serial.println(text);
      String from_name = msg.from_name;
  
      if (text == "/start" || text=="/huongdan" || text=="huongdan" || text=="/hd" || text=="hd"){
        String _welcome_str = String(welcome_str);
        _welcome_str.replace("<name>", from_name);
        nBot.SendMessage(CHAT_ID1, _welcome_str);
      }
      if (text == "/flash"){
        IsFlashOn = !IsFlashOn;
        //digitalWrite(FLASH_LED_PIN, IsFlashOn);
        Serial.println("Change flash LED state");

        String _msg1_str = String(msg1_str);

#ifdef LANG_EN
        if(IsFlashOn)
          _msg1_str.replace("<repstr>", "Turn on");
        else
          _msg1_str.replace("<repstr>", "Turn off");
#endif
#ifdef LANG_VN
        if(IsFlashOn)
          _msg1_str.replace("<repstr>", "Mở");
        else
          _msg1_str.replace("<repstr>", "Tắt");
#endif
          
        nBot.SendMessage(CHAT_ID1, _msg1_str);
      }
      if (text == "/photo" || text == "/chuphinh" || text =="photo" || text =="chuphinh"){
        //sendPhoto = true;

        if(msg_chat_id ==  CHAT_ID1)
          ID1_RequestPhoto = true;
        if(msg_chat_id ==  CHAT_ID2)
          ID2_RequestPhoto = true;
        if(msg_chat_id ==  CHAT_ID3)
          ID3_RequestPhoto = true;
      }
    }
}

static void IRAM_ATTR detectsMovement(void * arg) {
  Serial.println("Button Press");
  sendPhoto = true;
}


#define AP_SSID "DoorBellCam"
#define AP_PASS "12345678"

struct AppSetting
{
    boolean OK;
    char wifiname[30];
    char wifipass[30];
    char bottoken[60];
    char chatid1[15];
    char chatid2[15];
    char chatid3[15];
    bool auto_chatid1;
    bool auto_chatid2;
    bool auto_chatid3;
};

AppSetting settingData;

bool IsChatID(String ChatID)
{
  int length = ChatID.length();
  
  if( length < 10)
    return false;

    for(int i=0; i<length; i++)
    {
      if(ChatID[i] - '0' < 0 || ChatID[i] - '0' > 9)
        return false;
    }
  return true;
}

void LoadSetting()
{
  EEPROM.get(0, settingData);

  if(!isprint(settingData.wifiname[0]))
    settingData.wifiname[0] = 0;
  if(!isprint(settingData.wifipass[0]))
    settingData.wifipass[0] = 0;
  if(!isprint(settingData.bottoken[0]))
    settingData.bottoken[0] = 0;

  if(!isprint(settingData.chatid1[0]))
    settingData.chatid1[0] = 0;
  if(!isprint(settingData.chatid2[0]))
    settingData.chatid2[0] = 0;
  if(!isprint(settingData.chatid3[0]))
    settingData.chatid3[0] = 0;


  BOTtoken = String(settingData.bottoken);
  CHAT_ID1 = String(settingData.chatid1);
  CHAT_ID2 = String(settingData.chatid2);
  CHAT_ID3 = String(settingData.chatid3);

/*
  if(settingData.auto_chatid1!=false && settingData.auto_chatid1!=true) settingData.auto_chatid1 = true;
  if(settingData.auto_chatid2!=false && settingData.auto_chatid2!=true) settingData.auto_chatid2 = true;
  if(settingData.auto_chatid3!=false && settingData.auto_chatid3!=true) settingData.auto_chatid3 = true;
*/

  settingData.auto_chatid1 = true;
  settingData.auto_chatid2 = true;
  settingData.auto_chatid3 = true;
  
  Serial.println("Loading ...");
  Serial.print("Bottoken: ");
  Serial.println(settingData.bottoken);
  Serial.print("ChatID1: ");
  Serial.println(settingData.chatid1);
  Serial.print("ChatID2: ");
  Serial.println(settingData.chatid2);
  Serial.print("ChatID3: ");
  Serial.println(settingData.chatid3);
}

void SaveSetting()
{
  EEPROM.put(0, settingData);
  EEPROM.commit();
}


AsyncWebServer server(80);


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


// Set IP addresses
IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  EEPROM.begin(512);
  Serial.begin(115200);
  LoadSetting();

  myCam.Config();

  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);


  esp_err_t err = gpio_isr_handler_add(GPIO_NUM_15, &detectsMovement, (void *) 1);
    if (err != ESP_OK) {
    Serial.printf("handler add failed with error 0x%x \r\n", err);
  }
  err = gpio_set_intr_type(GPIO_NUM_15, GPIO_INTR_NEGEDGE);//GPIO_INTR_NEGEDGE, GPIO_INTR_POSEDGE, GPIO_INTR_ANYEDGE
    if (err != ESP_OK) {
    Serial.printf("set intr type failed with error 0x%x \r\n", err);
  }

  if(settingData.wifiname[0] == 0 || settingData.bottoken[0] == 0 || settingData.chatid1[0] == 0)
    IsAP = true;
    else IsAP = false;

  if(!IsAP){
      int count = 0;
      while(sendPhoto == false && count < 3){
        digitalWrite(FLASH_LED_PIN, HIGH);
        delay(500);
        digitalWrite(FLASH_LED_PIN, LOW);
        delay(500);
        count++;
      }
      if(sendPhoto) IsAP = true;
      else  IsAP = false;
  }

  if(IsAP){
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASS);

    index_html_str.replace("<wifiname>", String(settingData.wifiname));
    index_html_str.replace("<wifipass>", String(settingData.wifipass));
    index_html_str.replace("<bottoken>", String(settingData.bottoken));
    index_html_str.replace("<chatid1>", String(settingData.chatid1));
    index_html_str.replace("<chatid2>", String(settingData.chatid2));
    index_html_str.replace("<chatid3>", String(settingData.chatid3));


      // Send web page with input fields to client
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        int str_len = index_html_str.length() + 1; 
        char char_array[str_len];
        index_html_str.toCharArray(char_array, str_len);
        request->send_P(200, "text/html", char_array );
      });
    
      // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
      server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request)
      {

        String wifiname = request->getParam("wifiname")->value();
        String wifipass = request->getParam("wifipass")->value();
        String bottoken = request->getParam("bottoken")->value();
        String chatid1 = request->getParam("chatid1")->value();
        String chatid2 = request->getParam("chatid2")->value();
        String chatid3 = request->getParam("chatid3")->value();

        wifiname.toCharArray(settingData.wifiname, wifiname.length() + 1);
        wifipass.toCharArray(settingData.wifipass, wifipass.length() + 1);
        bottoken.toCharArray(settingData.bottoken, bottoken.length() + 1);
        chatid1.toCharArray(settingData.chatid1, chatid1.length() + 1);
        chatid2.toCharArray(settingData.chatid2, chatid1.length() + 1);
        chatid3.toCharArray(settingData.chatid3, chatid1.length() + 1);

        SaveSetting();
        
        Serial.println("wifiname: " + String(settingData.wifiname));
        Serial.println("wifipass: " + String(settingData.wifipass));
        Serial.println("bottoken: " + String(settingData.bottoken));
        Serial.println("chatid1: " + String(settingData.chatid1));
        Serial.println("chatid2: " + String(settingData.chatid2));
        Serial.println("chatid3: " + String(settingData.chatid3));

        request->send(200, "text/html", "HTTP GET request sent to your DoorBellCAM <br><a href=\"/\">Return to Home Page</a>");
        delay(2000);
        ESP.restart();
      });
      server.onNotFound(notFound);
      server.begin();
   
  } else{
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(settingData.wifiname);
    WiFi.begin(settingData.wifiname, settingData.wifipass);
  
    unsigned long t1 =  millis();
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      digitalWrite(FLASH_LED_PIN, HIGH);
      delay(50);
      digitalWrite(FLASH_LED_PIN, LOW);
      delay(1000);
      if(millis() - t1 > 10000)
        ESP.restart();
    }
    Serial.println();
    Serial.print("ESP32-CAM IP Address: ");
    Serial.println(WiFi.localIP()); 
  
    nBot.Init(BOTtoken);
    nBot.RegisterCallback(processMessage);
    nBot.RemoveDeafMSG();
  }
}

void loop(){
  if(IsAP){
    
  } else {
    nBot.HandleNewMessages();
    if (sendPhoto || ID1_RequestPhoto || ID2_RequestPhoto || ID3_RequestPhoto) {
      Serial.println("Preparing photo");
  
      if(IsFlashOn) digitalWrite(FLASH_LED_PIN, HIGH);
      delay(1000);
      camera_fb_t *fb = myCam.GetPhotoFB();
      fb = myCam.GetPhotoFB();
      digitalWrite(FLASH_LED_PIN, LOW);
      
      if(fb){
        if(IsChatID(CHAT_ID1) && (settingData.auto_chatid1 || ID1_RequestPhoto))
        {
          nBot.SendPhoto(CHAT_ID1, fb);
          ID1_RequestPhoto = false;
        }
        if(IsChatID(CHAT_ID2) && (settingData.auto_chatid2 || ID2_RequestPhoto))
        {
          nBot.SendPhoto(CHAT_ID2, fb);
          ID2_RequestPhoto = false;
        }
        if(IsChatID(CHAT_ID3) && (settingData.auto_chatid3 || ID3_RequestPhoto))
        {
          nBot.SendPhoto(CHAT_ID3, fb);
          ID3_RequestPhoto = false;
        }
        myCam.FBReturn(fb);
        sendPhoto = false;
      }
    }
  }
  delay(50);
}
