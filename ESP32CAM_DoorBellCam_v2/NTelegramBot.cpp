#include <Arduino.h>
#include "NTelegramBot.h"

#define NTELEGRAMBOT_DEBUG

NTelegramBot::NTelegramBot(){
}

void NTelegramBot::Init(const String& token){
  BotToken = token;
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  bot = new UniversalTelegramBot(BotToken, clientTCP);
}

void NTelegramBot::RemoveDeafMSG(){
  int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
  while (numNewMessages)
  {
    numNewMessages = bot->getUpdates(bot->last_message_received + 1);
  }
}

bool NTelegramBot::SendMessage(const String& chat_id, const String& text){
  return bot->sendMessage(chat_id, text, "");
}

String NTelegramBot::SendPhoto(String chat_id, camera_fb_t * fb){
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";
  
#ifdef NTELEGRAMBOT_DEBUG
  Serial.println("Connect to " + String(myDomain));
#endif  
  if (clientTCP.connect(myDomain, 443)) {
#ifdef NTELEGRAMBOT_DEBUG
    Serial.println("Connection successful");
#endif    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chat_id + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BotToken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
#ifdef NTELEGRAMBOT_DEBUG
      Serial.print(".");
#endif      
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
#ifdef NTELEGRAMBOT_DEBUG
    Serial.println(getBody);
#endif    
  }
  else {
    getBody="Connected to api.telegram.org failed.";
#ifdef NTELEGRAMBOT_DEBUG
    Serial.println("Connected to api.telegram.org failed.");
#endif    
  }
  return getBody;
}

void NTelegramBot::HandleNewMessages(){
  if (millis() > lastTimeBotRan + BOT_REQUEST_DELAY)
  {
    int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
    while (numNewMessages)
    {
      for (int i = 0; i < numNewMessages; i++)
      {

#ifdef NTELEGRAMBOT_DEBUG
    Serial.print("Bot got messages: "); Serial.println(bot->messages[i].text);
    Serial.print("From ID: "); Serial.println(String(bot->messages[i].chat_id));
#endif

        if(Event)
          Event(bot->messages[i]);
      }
      numNewMessages = bot->getUpdates(bot->last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
