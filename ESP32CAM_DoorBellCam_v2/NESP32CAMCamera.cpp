#include <Arduino.h>
#include "NESP32CAMCamera.h"

  /*
   * UXGA   = 1600 x 1200 pixels
   * SXGA   = 1280 x 1024 pixels
   * XGA    = 1024 x 768  pixels
   * SVGA   = 800 x 600   pixels
   * VGA    = 640 x 480   pixels
   * CIF    = 352 x 288   pixels
   * QVGA   = 320 x 240   pixels
   * HQVGA  = 240 x 160   pixels
   * QQVGA  = 160 x 120   pixels
   */

#define FRAM_SIZE_C FRAMESIZE_SVGA
#define FRAM_SIZE_R FRAMESIZE_VGA

//#define FRAM_SIZE_C FRAMESIZE_VGA
//#define FRAM_SIZE_R FRAMESIZE_CIF

NESP32CAMCamera::NESP32CAMCamera()
{
}

bool NESP32CAMCamera::Config()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAM_SIZE_C;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAM_SIZE_C;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
#ifdef DEBUGON
    Serial.printf("Camera init failed with error 0x%x", err);
#endif    
    return false;
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAM_SIZE_R);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  return true;
}

camera_fb_t * NESP32CAMCamera::GetPhotoFB()
{
  /*
  camera_fb_t * fb;
  for(int i=0; i<2; i++)
  {
    fb = esp_camera_fb_get();
    if(fb) esp_camera_fb_return(fb);
  }
  */
  return esp_camera_fb_get();
}

void NESP32CAMCamera::FBReturn(camera_fb_t *fb)
{
  esp_camera_fb_return(fb);
}
