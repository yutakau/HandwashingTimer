// Hand washing timer 
// 
// for M5Stick-C
//

#include <M5StickC.h>
#include <driver/i2s.h>
#include "arduinoFFT.h"


#define PIN_CLK  0
#define PIN_DATA 34
#define READ_LEN (2 * 1024)
#define SAMPLING_FREQUENCY 44100

#define SCORE_LIMIT 1000000
#define DET_THRES 300000
#define DET_TCNT  5
#define DETCNT_MAX 200
#define DETCNT_MIN 0
#define MAX_SEC 30

uint8_t BUFFER[READ_LEN] = {0};
uint16_t *adcBuffer = NULL;
const uint16_t FFTsamples = 512;
double vReal[FFTsamples];
double vImag[FFTsamples];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY); 
unsigned int sampling_period_us;
bool ledflag = true;
int nsamples = FFTsamples/2;
int label;
int detcnt, detectflag;
int start, seccnt;
  
void i2sInit(){
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void mic_record_task (void* arg){     
  while(1){
    i2s_read_bytes(I2S_NUM_0,(char*)BUFFER,READ_LEN,(100/portTICK_RATE_MS));
    adcBuffer = (uint16_t *)BUFFER;
    fft();
    detect();
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Axp.ScreenBreath(9);  
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);
  i2sInit();
  xTaskCreatePinnedToCore(mic_record_task,"mic_record_task",2048,NULL,1,NULL,1);
  detcnt = 0;

  M5.Lcd.fillScreen(WHITE);
  delay(500);
  M5.Lcd.fillScreen(BLACK);
}

void fft(){
  for (int i = 0; i < FFTsamples; i++) {
    unsigned long t = micros();
    vReal[i] = adcBuffer[i];
    vImag[i] = 0;
    while ((micros() - t) < sampling_period_us) ;
  }

  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); 
  FFT.Compute(FFT_FORWARD); 
  FFT.ComplexToMagnitude(); 
  //M5.Lcd.fillScreen(BLACK);  

#ifdef _SAMPLEMODE
  for (int band = 2; band < nsamples; band++) {
    float d = vReal[band] ;
//    Serial.print(band);
//    Serial.print(" : ");
//    Serial.print((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000);
//    Serial.print("kHz : ");
//    Serial.println(d);
    label = (ledflag) ? 0:1;    
    Serial.print(label);
    Serial.print(",");    
    Serial.print(band);
    Serial.print(",");
    Serial.println(d);
  }
  
  if(digitalRead(M5_BUTTON_RST) == LOW){
    ledflag = !ledflag;
    while(digitalRead(M5_BUTTON_RST) == LOW);
  }
  digitalWrite(M5_LED, ledflag);
#endif
}

void detect() {
  int res;
  res = predict( &vReal[0], nsamples );
  if (res > SCORE_LIMIT ) res = SCORE_LIMIT;  
  if (res > DET_THRES) {
    detcnt++;
    if (detcnt > DETCNT_MAX) detcnt=DETCNT_MAX;
  } else {
    detcnt--;
    if (detcnt < DETCNT_MIN) detcnt=DETCNT_MIN;
  }

  //Serial.println(detcnt);
  if (detcnt > DET_TCNT) {
    detectflag=1;    
  }else{
    detectflag=0;
  }
}

void finish_logo()
{
  M5.Lcd.setTextColor(0x7B1F);
  M5.Lcd.drawString( "OK!!", 0, 0, 8);
  delay(3000);
}

void loop(){
  char s[8];
  unsigned long t;
  //digitalWrite(M5_LED, !detectflag);  
  
  if ((start == 0) && (detectflag == 1)) {
    start=1;
    seccnt=0;
    Serial.println("start");
  }

  //if ((start == 1) && (detectflag == 0)) {
  //  M5.Lcd.fillScreen(BLACK);
  //  start = 0;
  //}
  if (digitalRead(M5_BUTTON_HOME) == LOW){
    M5.Lcd.fillScreen(BLACK);
    start = 0;
  }

  if (start==1) {
    t = micros();
    sprintf(s, "%2d", seccnt);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);    
    M5.Lcd.drawString( s, 0, 0, 8);
    
    while ((micros() - t) < 1*1000*1000 ) ;  //1s=1000*1000us
    seccnt++;
    if (seccnt > MAX_SEC) {
      finish_logo();
      while (digitalRead(M5_BUTTON_HOME) == LOW) ;
      
      start=0;
      M5.Lcd.fillScreen(BLACK);
    }
  }
  
}
