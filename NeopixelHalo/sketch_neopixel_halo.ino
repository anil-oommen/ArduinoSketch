#include <Adafruit_NeoPixel.h>


#define PIN 6
#define STRIPSIZE 16
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(64);  // Lower brightness and save eyeballs!
  strip.show(); // Initialize all pixels to 'off'
  //just debug only   
  Serial.begin(9600); 
  setMode("cyclic_overlap");
}

String mode = "none";
void loop(){
  //Start Triggers
  main_start_if();
  
  //End Triggers
  main_end_if();
}
void setMode(String i_mode ){
  mode = i_mode;
  main_init();
}




/********************************************************************
* Main Controlled For Timesliced Processing
*/
long main_set_switch_delay  = 1000; //Speed For Animation Change Check
long main_last_switch_mills  = 0; 

void main_init(){
  main_last_switch_mills = millis();
  
  cyco_setSpeed(1);
  if(mode=="processing"){ proc_init(); proc_setSpeed(5); }
  if(mode=="cyclic_overlap"){ cyco_init(); cyco_setSpeed(5); }
}
void main_start_if(){
  if(mode=="processing"){ proc_start_if(); }
  if(mode=="cyclic_overlap"){ cyco_start_if(); }
  
  
  if(main_isTimeUp(main_set_switch_delay)){
    //proc_ctlSpeed(1); // Any Special Processing done
  }
}
void main_end_if(){
  if(mode=="processing"){ proc_end_if(); }
  if(mode=="cyclic_overlap"){ cyco_end_if(); }
}

boolean main_isTimeUp(long interval){
  if((millis() - main_last_switch_mills ) > interval){
    //times up reset
    main_last_switch_mills = millis();
    return true;
  }
  return false;
}





/**************************************************************************************
* Feature Virtual LED , Reverse Flow and displacement adjust 
*/
boolean vled_go_backwards = true;
int vled_displacement = 0; // Displacement can be 0-15 , use to adjust starting point
int vled(int vled_pos){    //Accepted values 0-15 Just as supported by neopixel
  int pled_pos = vled_pos%STRIPSIZE; // 
  if(vled_go_backwards){
    pled_pos = STRIPSIZE - pled_pos -1;
  }
  
  pled_pos = (pled_pos+vled_displacement) % STRIPSIZE;
  return pled_pos;
}




/**************************************************************************************
* Feature 'cyclic overlap' circle round with on and off , n overlap
* Usage :  cyco_init() 
*          cyco_setColor() 
*          cyco_setOverlayColor() 
*          cyco_setSpeed()  value 1-10 , 
*          cyco_ctlSpeed() Control speed increase (+ve) or decrease (-ve)
*          
*/
long cyco_set_switch_delay_MIN = 2;   // The Minimum Delay , So Max Speed 
long cyco_set_switch_delay_MAX = 200; // The Maximum Delay , So Minimum Speed 
long cyco_set_switch_delay  = (cyco_set_switch_delay_MIN + cyco_set_switch_delay_MAX)/2; 
      // Speed of Animation , Default is mid way
long cyco_last_switch_mills  = 0; 
uint32_t cyco_black = strip.Color(0,0,0);
uint32_t cyco_color = strip.Color(10,0,10);
uint32_t cyco_overlay_color = strip.Color(200,0,200); 
uint32_t cyco_keypoint_color = strip.Color(0,0,200); 

void cyco_init(){
  cyco_last_switch_mills = millis();
}

void cyco_setColor(int r, int g, int b){
  cyco_color = strip.Color(r,g,b);
}

void cyco_setOverlayColor(int r, int g, int b){
  cyco_overlay_color = strip.Color(r,g,b);
}

int cyco_currentSpeed =-1;  //Speed is 1-10 , Default value no relevence, have to setSpeed()
void cyco_setSpeed(int pSpeed){
  if(pSpeed>10) {cyco_currentSpeed=10;} //max val
  else if(pSpeed<1) { cyco_currentSpeed=1; }   //min val
  else {cyco_currentSpeed = pSpeed;}

  //  Find the delay to set, also reverse so 10-x
  cyco_set_switch_delay = cyco_set_switch_delay_MIN + 
   (  (cyco_set_switch_delay_MAX - cyco_set_switch_delay_MIN) *  (11-cyco_currentSpeed) * 0.1); 
  
  Serial.println(cyco_set_switch_delay);
  Serial.println();
}

void cyco_ctlSpeed(int adjustSpeed){
  Serial.println(cyco_currentSpeed);
  Serial.println(adjustSpeed);
  proc_setSpeed(cyco_currentSpeed+adjustSpeed);
}


void cyco_end_if() {
  //none here
}
int cyco_index=1;
int cyco_key_point=1;
boolean toggle_ONOFF = false;
void cyco_start_if() {
  
  
  
  if(cyco_isTimeUp(cyco_set_switch_delay)){
  
    int cyco_index_mod = cyco_index%STRIPSIZE;  
    Serial.println(cyco_index);
    if(cyco_index_mod==cyco_key_point){ 
      //Starting Point
      if(toggle_ONOFF){  
        Serial.println("KeyReached");
        strip.setPixelColor(vled(cyco_index_mod), cyco_keypoint_color); 
      }else{
        strip.setPixelColor(vled(cyco_index_mod), cyco_keypoint_color); 
      }
      toggle_ONOFF = !toggle_ONOFF;
      cyco_key_point++;
      cyco_index++;
    }else if((cyco_index_mod-cyco_key_point)>0 && (cyco_index_mod-cyco_key_point)<4){
     if(toggle_ONOFF){  
       Serial.println("Key-near");
       strip.setPixelColor(vled(cyco_index_mod), cyco_overlay_color); 
      }else{
        strip.setPixelColor(vled(cyco_index_mod), cyco_color); 
      }
    }else{
      //Other Leds 
      if(toggle_ONOFF){  
        Serial.println("SlavesReached");
        strip.setPixelColor(vled(cyco_index_mod), cyco_color); 
      }else{
        strip.setPixelColor(vled(cyco_index_mod), cyco_black); 
      }
    }
    strip.show();
    cyco_index++;
    
    //cyco_index=cyco_index%16;
  }
  
  
  //cyco_index=(cyco_index%STRIPSIZE); //Minus one since we are doing ++
  /*
  if(cyco_isTimeUp(cyco_set_switch_delay)){
     strip.setPixelColor(vled(cyco_index+0), cyco_color); //lead
  }*/
}

boolean cyco_isTimeUp(long interval){
  if((millis() - cyco_last_switch_mills ) > interval){
    //times up reset
    cyco_last_switch_mills = millis();
    return true;
  }
  return false;
}






/**************************************************************************************
* Feature 'processing/ refreshing' circle round with 3 trails leds 
* Usage :  proc_init() 
*          proc_setColor() 
*          proc_setSpeed()  value 1-10 , 
*          proc_ctlSpeed() Control speed increase (+ve) or decrease (-ve)
*          
*/
int proc_index=1;
long proc_set_switch_delay_MIN = 2;   // The Minimum Delay , So Max Speed 
long proc_set_switch_delay_MAX = 200; // The Maximum Delay , So Minimum Speed 
long proc_set_switch_delay  = (proc_set_switch_delay_MIN + proc_set_switch_delay_MAX)/2; 
      // Speed of Animation , Default is mid way

long proc_last_switch_mills  = 0; 
uint32_t proc_black = strip.Color(0,0,0);
uint32_t proc_lead = strip.Color(200,0,200);        //Default Colors
uint32_t proc_trail_low1 = strip.Color(100,0,100);  //Default Colors
uint32_t proc_trail_low2 = strip.Color(10,0,10);    //Default Colors
uint32_t proc_trail_low3 = strip.Color(5,0,5);      //Default Colors

void proc_init(){
  proc_last_switch_mills = millis();
  proc_setColor(76,186,54); //Green
  proc_setColor(222,122,9); //Orange
  //proc_setColor(9,176,222); //Blue
}

void proc_setColor(int r, int g, int b){
  proc_lead = strip.Color(r,g,b);
  proc_trail_low1 = strip.Color(r*0.5,g*0.5,b*0.5);
  proc_trail_low2 = strip.Color(r*0.05,g*0.05,b*0.05);
  proc_trail_low3 = strip.Color(r*0.03,g*0.03,b*0.03);
}

int proc_currentSpeed =-1;  //Speed is 1-10 , Default value no relevence, have to setSpeed()
void proc_setSpeed(int pSpeed){
  
  if(pSpeed>10) {proc_currentSpeed=10;} //max val
  else if(pSpeed<1) { proc_currentSpeed=1; }   //min val
  else {proc_currentSpeed = pSpeed;}

  //  Find the delay to set, also reverse so 10-x
  proc_set_switch_delay = proc_set_switch_delay_MIN + 
   (  (proc_set_switch_delay_MAX - proc_set_switch_delay_MIN) *  (11-proc_currentSpeed) * 0.1); 
  
  Serial.println(proc_set_switch_delay);
  Serial.println();
}

void proc_ctlSpeed(int adjustSpeed){
  Serial.println(proc_currentSpeed);
  Serial.println(adjustSpeed);
  proc_setSpeed(proc_currentSpeed+adjustSpeed);
}


void proc_end_if() {
  //none here
}
void proc_start_if() {
  
  if(proc_isTimeUp(proc_set_switch_delay)){
      
    //Reset old counters
    strip.setPixelColor(vled(proc_index+0), proc_black); //lead
    strip.setPixelColor(vled(proc_index+1), proc_black);  //trail 1
    strip.setPixelColor(vled(proc_index+2), proc_black);  //Trail 2
    strip.setPixelColor(vled(proc_index+3), proc_black);  //Trail 2
    
    proc_index++;
    
    //Setup Leds and show
    strip.setPixelColor(vled(proc_index+0), proc_trail_low3); //lead
    strip.setPixelColor(vled(proc_index+1), proc_trail_low2); //lead
    strip.setPixelColor(vled(proc_index+2), proc_trail_low1);  //trail 1
    strip.setPixelColor(vled(proc_index+3), proc_lead);  //Trail 2
    strip.show(); 
  }
}

boolean proc_isTimeUp(long interval){
  if((millis() - proc_last_switch_mills ) > interval){
    //times up reset
    proc_last_switch_mills = millis();
    return true;
  }
  return false;
}

