// 2025.03.30 i2c scan, RE
//  Report the connected i2c address. 
//  If sw turning on, reporting SW name and ADC value or RE count.
// For R909-VFO ESP32-C3 mini
// To test the OLED display and SW1 responce
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include<Rotary.h>                      // Mr Ben buxton's INT 
#define ENC_A 2                         // R909-VFO-ESP Rotary encoder A port
#define ENC_B 3
Rotary r = Rotary(ENC_A,ENC_B);

#define RE_SW 4                         // R909-VFO-ESP RE-SW port
#define PIN_SDA 8                       // ESP32-C3 mini SDA
#define PIN_SCL 9                       // ESP32-C3 mini SCL
#define ADC_port 1                      // R909-VFO-ESP SW port
#define led_port 21                     // R909-VFO-ESP LED port

  int onoff = 0;
  byte error, address;
  int nDevices;
  boolean led_status;
  char charbuf[6];
  int re_cnt = 0;                       // RE +/- counter
  int cnt = 0;

#define SCREEN_WIDTH 128                // SSD1306 resolution
#define SCREEN_HEIGHT 64                // SSD1306 resolution
#define OLED_RESET     -1               // Set -1 for no use
#define SCREEN_ADDRESS 0x3C             // 0x3C

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);     //
                                        
void setup()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);                            // If no responce , wait for ever
  }

  r.begin();

  attachInterrupt(ENC_A,rotary_encoder,CHANGE);
  attachInterrupt(ENC_B,rotary_encoder,CHANGE);
  Serial.begin(9600);

  display.clearDisplay();               // Display clear at first
  display.display();                    // To kick displaying
  pinMode(led_port, OUTPUT);
  pinMode(RE_SW, INPUT);

  display.setTextSize(1);               // select second size of font
  display.setTextColor(SSD1306_WHITE);  // default
  display.setCursor(5, 0);              // the starting point of character of bit position
  display.print("R909-VFO");            // the banner
  display.print(" i2c TEST");
 
  delay(2000);
}

void loop()
{
  Serial.println("i2c Scanning start");
  digitalWrite(led_port, led_status);  
  led_status = !led_status;
  delay(1000);   

  display.setCursor(0, 15);
  display.print("i2c scan");  
  display.display();
  
  display.setCursor(0, 32);
  display.print(" AD:"); 
  display.display();
    
  nDevices = 0;
  cnt = 0;
  for(address = 1; address < 127; address++ )
  {
    // Check WIRE function return back result
    // The value of Write.endTransmisstion
    // Is there ACK response (yes:0） or no
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)         // Got response
    {
      Serial.print("i2c device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      sprintf(charbuf, "%02X", address );
 
      cnt++;
      display.setCursor(cnt*36, 32);
      display.print(charbuf); 
      display.print(",");
      display.display(); 
      
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  
  if (nDevices == 0)
  {
    Serial.println("No i2c devices found\n");   
    display.setCursor(0, 49);
    display.print("No devices"); 
    display.display();
  }
  else{
    Serial.println("done\n");    
    display.setCursor(cnt*15, 49);
    display.print("Done"); 
    display.display();
  } 

  delay(2000);                  // Wait 2 sec
  display.clearDisplay(); 
 
  display.display();                    // To kick displaying

  digitalWrite(led_port, onoff++%2);
  delay(500);
  int sw_adc;
  if ( (sw_adc = analogRead( ADC_port)) < 4000) {
    display.clearDisplay();
    display.setCursor(0, 0);            // the starting point of character of bit position
    if(sw_adc < 50) display.print("SW1 on");  
    else display.print("SW2 on");       // SW2   
    display.print(sw_adc); 
    display.display();                  // To display
    delay(1000);
  }
  if( digitalRead(RE_SW) == 0){
    display.clearDisplay();
    display.setCursor(0, 0);            // the starting point of character of bit position
    display.print("RE SW on  ");  
    display.print(re_cnt); 
    display.display();                  // To display
    delay(1000);
  }
}

void rotary_encoder(){
  unsigned char result = r.process();
  if(result){
    if(result == DIR_CW){
      re_cnt++;
      Serial.println("Right");
    }
    else{
      re_cnt--;
      Serial.println("Left");
    }
  }
}
