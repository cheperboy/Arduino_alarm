

//#include "../libraries/Metro/Metro.h"
//#include "../libraries/Password/Password.h"
//#include "../libraries/Keypad/Keypad.h"
//#include "../libraries/SerialLCD/SerialLCD.h"
//#include "/Applications/Arduino.app/Contents/Resources/Java/libraries/SoftwareSerial/SoftwareSerial.h"

 #include <Password.h> // http://www.arduino.cc/playground/uploads/Code/Password.zip
 #include <Keypad.h>   // http://www.arduino.cc/playground/uploads/Code/keypad.zip
 #include <Metro.h>
 #include <SerialLCD.h>
 #include <SoftwareSerial.h>

// --- define ---
#define wait_on_delay 	    4*1000
#define before_sirene_delay 4*1000
#define ring_sirene_delay   10*1000
#define define_pwd					1

#define ROWS 4 // clavier
#define COLS 4 // clavier

// --- setup serialLCD --- 
#define lcdTX 10
#define lcdRX 11

// --- setup PIR & Alarm --- 
#define pirPin 12
#define alarmPin 17
// #define relayPin 14

// --- types ---
enum type_alarmState  {
	off,				// system idle
	wait_on,		// delay before running (delai de sortie maison)
	on, 				// system is running
	detection,	// movement detected
	before_sirene,// delay before sirene (delai d'entrée dans maison) 
	ring_sirene // sirene is ringing
	};

// --- variables ---

// --- programme ---
type_alarmState alarmState      = off;
type_alarmState next_alarmState = alarmState;
Metro wait_on_timer       = Metro(wait_on_delay);
Metro before_sirene_timer = Metro(before_sirene_delay);
Metro ring_sirene_timer   = Metro(ring_sirene_delay);
Password pwd = Password("123");   // définition du mot de passe
boolean passCorrect             = false;
boolean sensorDetection         = false;

// --- keypad ---
uint8_t printPositionPIN = 0;
boolean pressed_OK       = false;
boolean pressed_CANCEL   = false;


// --- setup Keypad --- 
char keys[ROWS][COLS] =                                              //
{                                                                    //
  {'1','2','3','A'},                                                 //
  {'4','5','6','B'},                                                 // configuration
  {'7','8','9','C'},                                                 // du clavier
  {'*','0','#','D'}                                                  //
};                                                                   //
byte rowPins[ROWS] = {9, 8, 7, 6};                                   //
byte colPins[COLS] = {5, 4, 3, 2};                                   //
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); //


// --- setup LCD --- 
//LiquidCrystal lcd(8, 9, 10, 11, 12, 13); ancien lcd sans shield serie
SerialLCD slcd(lcdTX,lcdRX);


// --- fonctions ---

// --- setup ---
void setup()
{
	pinMode(alarmPin, OUTPUT);
	pinMode(pirPin, INPUT);
// 	pinMode(relayPin, OUTPUT);
  kpd.addEventListener(kpdEvent); //keypad event listener
  Serial.begin(9600); // serial debug
  slcd.begin();
  set_lcd_0("demarrage");
  delay(1000);
}

void goto_off_if_password(){
	if (key_OK()){
		if(checkPassword()) {
			next_alarmState = off;
			set_lcd_0("alarm off");
			clear_lcd_pin();
		}
	}
}

void goto_on_if_password(){
	if (key_OK()){
		if(checkPassword()) {
			next_alarmState = wait_on;
			set_lcd_0("wait_on");
			clear_lcd_pin();
			wait_on_timer.reset();
		}	
	}
}

// --- loop ---
void loop(){
	char Key = kpd.getKey();
	switch (alarmState) {
		// off - system idle
	  case off:
			set_sirene(false);
			goto_on_if_password();
    	break;

		// wait_on - delay before on (delai de sortie maison)
	  case wait_on:
			set_sirene(false);
			goto_off_if_password();
			if(wait_on_timer.check() == 1){
				set_lcd_0("alarm on");
				next_alarmState = on;
			}
    	break;

		// on - system is running
	  case on:
			set_sirene(false);
			if(get_sensors()) {
				next_alarmState = detection;
				set_lcd_0("detection");
			}
			goto_off_if_password();
    	break;

		// detection - movement detected
	  case detection:
			set_sirene(false);
			send_sms();
			goto_off_if_password();
			before_sirene_timer.reset();
			next_alarmState = before_sirene;
	    break;
	
		// before_sirene - delay before sirene (delai d'entrée dans maison) 
	  case before_sirene:
			set_sirene(false);
			if(before_sirene_timer.check() == 1){
				ring_sirene_timer.reset();
				next_alarmState = ring_sirene;
				set_lcd_0("sirene");
			}
			goto_off_if_password();
	    break;

		// sirene - sirene is crying
	  case ring_sirene:
			set_sirene(true);
			goto_off_if_password();
			if(ring_sirene_timer.check() == 1){
				wait_on_timer.reset();
				next_alarmState = wait_on;
				set_lcd_0("wait_on");	
			}
	    break;
	}
	alarmState = next_alarmState;
}




