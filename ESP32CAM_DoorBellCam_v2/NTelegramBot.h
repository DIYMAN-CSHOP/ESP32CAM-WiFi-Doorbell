#ifndef NTELEGRAMBOT_H_
#define NTELEGRAMBOT_H_

#define BOT_REQUEST_DELAY 1000


#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "esp_camera.h"

class NTelegramBot
{
  using InputEvent = void (*)(telegramMessage &);
  
  public:
    NTelegramBot();
    void Init(const String& token);
    bool SendMessage(const String& chat_id, const String& text);
    String SendPhoto(String chat_id, camera_fb_t * fb);
    void HandleNewMessages();
    void RemoveDeafMSG();
    void RegisterCallback(InputEvent InEvent)
    {
        Event = InEvent;
    }
    
  private:
    WiFiClientSecure clientTCP;
    UniversalTelegramBot* bot;
    InputEvent Event;
    String BotToken;
    unsigned long lastTimeBotRan;
};

#endif /* NTELEGRAMBOT_H_ */
