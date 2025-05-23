// 2025.04.02
// Changed to R909-VFO-ESP  BAND A1->4, rx_tx A2->D5, AGC A0, TUNE-SW D1
//  INT,
// To reduce memory consumption, change code to array reference 20230729
//  freq_band[21], bargraph function, 
//  There are some bugs on startup display and serial message
/**********************************************************************************************************
  10kHz to 225MHz VFO / RF Generator with Si5351 and Arduino Nano, with Intermediate
  Frequency (IF) offset
  (+ or -), RX/TX Selector for QRP Transceivers, Band Presets
  and Bargraph S-Meter. See the schematics for
  wiring and README.txt for details.
  By J. CesarSound - ver 2.0 - Feb/2021.
***********************************************************************************************************/

//Libraries
#include <Wire.h>                 //IDE Standard
#include <Rotary.h>               //Ben  Buxton https://github.com/brianlow/Rotary
#include <si5351.h>               //Etherkit  https://github.com/etherkit/Si5351Arduino
#include <Adafruit_GFX.h>         //Adafruit  GFX https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>     //Adafruit SSD1306 https://github.com/adafruit/Adafruit_SSD1306

//User  preferences
//------------------------------------------------------------------------------------------------------------
#define IF         0         //Enter your IF frequency, ex: 455 = 455kHz, 10700 = 10.7MHz,  
                             // 0 = to direct convert receiver or RF generator, + will add and - will subtract IF  offfset.
#define BAND_INIT  8         //Enter your initial Band (1-21) at startup,  8:10MHz
                             //  ex: 1 = Freq Generator, 2 = 800kHz (MW), 7 = 7.2MHz (40m), 11 = 14.1MHz (20m). 
#define XT_CAL_F   147600    //Si5351 calibration factor, adjust to get exatcly 10MHz. Increasing
                             //  this value will decreases the frequency and vice versa.
#define S_GAIN     1010      //Adjust the sensitivity of Signal Meter A/D input: 101 = 500mv; 202 = 1v;
                             //  303 = 1.5v; 404 = 2v; 505 = 2.5v; 1010 = 5v (max).
#define TUNE_SW    1         //  ** The   pin used by tune step push button.
#define BAND_SW    4         //  ** The pin used   by band selector push button. CHANGE A1->A2
#define RXTX_SW    5         //  ** The pin used by   RX / TX selector switch, RX = switch open, A2->D4
                             //  TX = switch closed to GND. When in TX,  the IF value is not considered.
#define AGC        A0        //  ** The pin used by   Signal Meter A/D input.
#define REA        2         //
#define REB        3         //
#define LED_PORT   21
//------------------------------------------------------------------------------------------------------------

//    stp = 4;   // Frequency step 4:1kHz

Rotary r = Rotary(REA, REB);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
Si5351  si5351(0x60); //Si5351 I2C Address 0x60

unsigned long freq, freqold, fstep;
long   interfreq = IF, interfreqold = 0;
long cal5351 = XT_CAL_F;
unsigned int smval;
byte   encoder = 1;  // RE active or inactive
byte fstep_no, tu_ind = 1;     // Frequency step 4:1kHz, tu_ind: tune pointer position
unsigned int band_no=1;  // band_no: band number
unsigned int s_value, s_value_old;
bool rxtx_sts = 0;
unsigned int period = 100;
unsigned long time_now = 0;

static unsigned long freq_band[21] = {100000, 800000, 1800000, 3650000, 4985000, 
6180000, 7200000, 10000000, 11780000, 13630000, 14100000, 15000000, 17655000, 
21525000, 27015000, 28400000, 50000000, 100000000, 118000000, 144000000, 220000000} ;
static unsigned long fstep_list[6] = {1,1000,10000,100000,1000000,25000};
static unsigned char fstep_name[6][7] = { " 25kHz"," 1Hz "," 1kHz ", " 10kHz", "100kHz", " 1MHz "};
static unsigned char band_name[21][5] = { "GEN","MW","160m","80m","60m","49m","40m","31m","25m"
  ,"22m","20m","19m","16m","13m","11m","10m","6m","WFM","AIR","2m","1m"};

void rotary_encoder(){
  char re_result = r.process();
  if (re_result == DIR_CW) set_frequency(1);
  else if (re_result == DIR_CCW) set_frequency(-1);
}

void set_frequency(int dir) {
  if (encoder == 1) {                         //Up/Down frequency
    if (dir == 1) freq = freq + fstep;
    if (freq >= 225000000) freq = 225000000;
    if (dir == -1) freq = freq - fstep;
    if (fstep == 1000000 && freq <= 1000000) freq = 1000000;
    else if (freq < 10000) freq = 10000;
  }
  if (encoder  == 1) {                       //Up/Down graph tune pointer
    if (dir == 1)  tu_ind = tu_ind + 1;
    if (tu_ind > 42) tu_ind = 1;
    if (dir == -1) tu_ind = tu_ind - 1;
    if (tu_ind < 1) tu_ind = 42;
  }
}

void setup() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  pinMode(REA, INPUT_PULLUP);
  pinMode(REB, INPUT_PULLUP);
  pinMode(TUNE_SW, INPUT_PULLUP);
  pinMode(BAND_SW, INPUT_PULLUP);
  pinMode(RXTX_SW, INPUT_PULLUP);
  pinMode(LED_PORT, OUTPUT);
  digitalWrite(LED_PORT,LOW);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(cal5351, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 1);                  //1 - Enable / 0 - Disable CLK
  si5351.output_enable(SI5351_CLK1, 0);
  si5351.output_enable(SI5351_CLK2, 0);

  attachInterrupt(REA,rotary_encoder,CHANGE);
  attachInterrupt(REB,rotary_encoder,CHANGE);

  sei();

  band_no = BAND_INIT;
  bandpresets(band_no);
  fstep_no = 1;          // Frequency step 4:1kHz
  setstep();
  
  statup_text();         //If you hang on startup, comment
  delay(500); 

//  Serial.begin(9600); //If you hang on startup, comment
//  Serial.println(F("\nJA3KPA VFO V1.0")); //If you hang on startup, comment

  delay(500); 
     
}

void loop() {

  if (freqold != freq) {
    time_set();             // time_now = mullis()
    tunegen();              // Set 5351 CLK0 as freq
    freqold = freq;
  }

  if (interfreqold != interfreq) {
    time_set();
    tunegen();
    interfreqold = interfreq;
  }

  if (s_value_old != s_value) {
    time_set();
    s_value_old = s_value;
  }

  if (digitalRead(TUNE_SW) == LOW) {
    time_delay_set();             // time_now = mullis()+300
    setstep();
    delay(300);
  }

  if (digitalRead(BAND_SW) == LOW) {
    time_delay_set();
    inc_preset();
    delay(300);
  }
  if (digitalRead(RXTX_SW) == LOW){
    time_delay_set();
    rxtx_sts = 1;
  } else rxtx_sts = 0;


  if ((time_now + period) > millis()) {
    displayfreq();
    layout();              // Display edited buffer
  }
  sgnalread();
}

void time_set(void){
  time_now = millis();
}

void time_delay_set(void){
  time_now = ( millis()+ 300 );
}

void tunegen() {
  si5351.set_freq((freq + (interfreq * 1000ULL)) * 100ULL, SI5351_CLK0);
}

void displayfreq() {
  unsigned int m = freq / 1000000;
  unsigned int k = (freq % 1000000) / 1000;
  unsigned int h = (freq % 1000) / 1;

  display.clearDisplay();
  display.setTextSize(2);

  char buffer[15] = "";
  if (m < 1) {
    display.setCursor(41, 1); 
    sprintf(buffer, "%003d.%003d", k, h);
  }
  else if (m < 100) {
    display.setCursor(5, 1); 
    sprintf(buffer, "%2d.%003d.%003d", m, k, h);
  }
  else if (m >= 100)
  {
    unsigned int h = (freq % 1000) / 10;
    display.setCursor(5, 1); 
    sprintf(buffer, "%2d.%003d.%02d", m, k, h);
  }
  display.print(buffer);
}

void  setstep() {
//  fstep = fstep_list[fstep_no-1];
  switch (fstep_no) {
    case 1: fstep = fstep_list[0]; break;
    case 2: fstep = fstep_list[1]; break;
    case 3: fstep = fstep_list[2]; break;
    case 4: fstep = fstep_list[3]; break;
    case 5: fstep = fstep_list[4]; break;
    case 6: fstep = fstep_list[5]; break;
  }
  fstep_no++;
  if(fstep_no>=7) fstep_no=1;
}

void inc_preset() {
  band_no++;
  if (band_no > 21) band_no = 1;
  bandpresets(band_no);
  delay(50);
}

void  bandpresets(unsigned int band_count) {
//  freq = freq_band[band_count-1];  
  switch (band_count){
    case 1: freq = freq_band[0]; tunegen(); break;
    case 2: freq = freq_band[1]; break;
    case 3: freq = freq_band[2]; break;
    case 4: freq = freq_band[3]; break;
    case 5: freq = freq_band[4]; break;
    case 6: freq = freq_band[5]; break;
    case 7: freq = freq_band[6]; break;
    case 8: freq = freq_band[7]; break;
    case 9: freq = freq_band[8]; break;
    case 10: freq = freq_band[9]; break;
    case 11: freq = freq_band[10]; break;
    case 12: freq = freq_band[11]; break;
    case 13: freq = freq_band[12]; break;
    case 14: freq = freq_band[13]; break;
    case 15: freq = freq_band[14]; break;
    case 16: freq = freq_band[15]; break;
    case 17: freq = freq_band[16]; break;
    case 18: freq = freq_band[17]; break;
    case 19: freq = freq_band[18]; break;
    case 20: freq = freq_band[19]; break;
    case 21: freq = freq_band[20]; break;
    
  }
  si5351.pll_reset(SI5351_PLLA);
  fstep_no = 4;     // Frequency step 4:1kHz n: tune pointer position
  setstep();
}

void layout() {
  display.setTextColor(WHITE);
  display.drawLine(0, 20, 127, 20, WHITE);
  display.drawLine(0, 43, 127, 43, WHITE);
  display.drawLine(105, 24, 105, 39, WHITE);
  display.drawLine(87, 24, 87, 39, WHITE);
  display.drawLine(87, 48, 87, 63, WHITE);
  display.drawLine(15, 55, 82, 55, WHITE);
  display.setTextSize(1);
  display.setCursor(59, 23);
  display.print("STEP");
  display.setCursor(54, 33);
  //char disp_buff[7];
  //strcpy(disp_buff,fstep_name[fstep_no-1]);
  //display.print(disp_buff);
  // /*  
  if (fstep_no == 2) display.print(" 1Hz"); 
  if (fstep_no == 3) display.print(" 1kHz"); 
  if (fstep_no == 4) display.print(" 10kHz");
  if (fstep_no == 5) display.print("100kHz"); 
  if (fstep_no == 6) display.print(" 1MHz");
  if (fstep_no == 1) display.print("25kHz");
  // */
  display.setTextSize(1);
  display.setCursor(92, 48);
  display.print("IF:");
  display.setCursor(92, 57);
  display.print(interfreq);
  display.print("k");
  display.setTextSize(1);
  display.setCursor(110, 23);
  if (freq < 1000000) display.print("kHz");
  if (freq >= 1000000) display.print("MHz");
  display.setCursor(110, 33);
  if (interfreq == 0) display.print("VFO");
  if (interfreq != 0) display.print("L O"); 
  display.setCursor(91, 28);
  if (!rxtx_sts) display.print("RX"); 
  if (!rxtx_sts) interfreq = IF;
  if (rxtx_sts) display.print("TX"); 
  if (rxtx_sts) interfreq = 0;  
  bandlist(); 
  drawbargraph();

  display.display();
}

void bandlist() {
  display.setTextSize(2);
  display.setCursor(0, 25);
  
//  char disp_buff[4];
//  strcpy(disp_buff,band_name[band_no-1]);
//  display.print(disp_buff);
  
  if (band_no == 1) display.print("GEN"); 
  if (band_no == 2) display.print("MW"); 
  if (band_no == 3) display.print("160m"); 
  if (band_no == 4) display.print("80m");
  if (band_no == 5) display.print("60m"); 
  if (band_no == 6) display.print("49m"); 
  if (band_no == 7) display.print("40m"); 
  if (band_no == 8) display.print("31m");
  if (band_no == 9) display.print("25m"); 
  if (band_no == 10) display.print("22m"); 
  if (band_no == 11) display.print("20m"); 
  if (band_no == 12) display.print("19m");
  if (band_no == 13) display.print("16m"); 
  if (band_no == 14) display.print("13m"); 
  if (band_no == 15) display.print("11m"); 
  if (band_no == 16) display.print("10m");
  if (band_no == 17) display.print("6m");
  if (band_no == 18) display.print("WFM"); 
  if (band_no == 19) display.print("AIR");
  if (band_no == 20) display.print("2m");
  if (band_no == 21) display.print("1m");

  
  if (band_no == 1) interfreq = 0; 
  else if (!rxtx_sts) interfreq = IF;
}

void sgnalread() {
    smval = analogRead(AGC); 
    s_value = map(smval, 0, S_GAIN, 1, 14); 
    if (s_value > 14) s_value = 14;
  }

void drawbargraph() {
  byte tu_pos = map(tu_ind, 1, 42, 1, 14);
  display.setTextSize(1);

  //Tune indicator pointer
  display.setCursor(0, 48);
  display.print("TU");
  byte t_pos = 15+5*(tu_pos-1);
  display.fillRect(t_pos, 48, 2, 6, WHITE);
 
  // S-meter displaying 
  display.setCursor(0, 57);
  display.print("SM");
  
  for ( int xx = s_value; xx >= 0; xx--){ 
    byte s_pos = 15+5*(xx-1);
    display.fillRect(s_pos, 58, 2, 6, WHITE);
  }
}

void statup_text() {
  display.clearDisplay();
  display.setTextSize(1); 
  display.setCursor(13, 18);
  display.print(F("Si5351 VFO/RF GEN"));
  display.setCursor(13, 40);
  display.print(F("KPA lab. V1.1 @JCR"));
  display.display(); 
  delay(4000);
}
