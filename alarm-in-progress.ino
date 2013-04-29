// #include "../libraries/Metro/Metro.h"
// #include "../libraries/Password/Password.h"
// #include "../libraries/Keypad/Keypad.h"
// #include "/Applications/Arduino.app/Contents/Resources/Java/libraries/LiquidCrystal/LiquidCrystal.h"

#include <Password.h> // http://www.arduino.cc/playground/uploads/Code/Password.zip
#include <Keypad.h>   // http://www.arduino.cc/playground/uploads/Code/keypad.zip
#include <Metro.h>
#include <LiquidCrystal.h>

const static uint8_t A0 = 14;
const static uint8_t A1 = 15;
const static uint8_t A2 = 16;
const static uint8_t A3 = 17;
const static uint8_t A4 = 18;
const static uint8_t A5 = 19;

// --- define ---
#define wait_on_delay 			5*1000
#define before_sirene_delay 5*1000
#define ring_sirene_delay 	5*1000
#define define_pwd					1

#define ROWS 4 // clavier
#define COLS 4 // clavier

// --- setup PIR & Alarm --- 
#define pirPin 11 
#define alarmPin 12
#define relayPin A0

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
type_alarmState alarmState      = off;
type_alarmState next_alarmState = alarmState;
boolean passCorrect             = false;
boolean sensorDetection         = false;
Metro wait_on_timer       = Metro(wait_on_delay);
Metro before_sirene_timer = Metro(before_sirene_delay);
Metro ring_sirene_timer   = Metro(ring_sirene_delay);
Password pwd = Password("123");   // définition du mot de passe

struct AlarmSettings
{
  byte magic;
  char alarmPassword1[PasswordLength_Max];
//  char alarmPassword2[PasswordLength_Max];
//  char alarmPassword3[PasswordLength_Max];
//  char alarmPassword4[PasswordLength_Max];
  char menuPassword[PasswordLength_Max];
  unsigned int keypadDebounceTime;
  unsigned int lcdBacklightTime;
  unsigned int beforeTurnOnDelay;
  unsigned int beforeAlarmDelay;
  unsigned int alarmStatOutCount;
  unsigned int alarmStatInCount;
  unsigned int alarmStatTamperCount;
  unsigned int alarmStatFireCount;
}
settings =
{
  SettingsMagic, // magic
  "1111", // alarmPassword1,
  "0000", // menuPassword
  80,     // keypadDebounceTime
  60000,  // lcdBacklightTime
  30000,   // beforeTurnOnDelay
  15000,   // beforeAlarmDelay
  0xFFFFFFFF, // alarmOutMask - all outputs are enabled
  0xFFFFFFFF, // alarmInMask - all inputs are enabled
  0xFFFFFFFF, // alarmTamperMask - all tampers are enabled
  0xFFFFFFFF, // alarmFireMask - all fire inputs are enabled
  0, // alarmStatOutCount
  0, // alarmStatInCount
  0, // alarmStatTamperCount
  0, // alarmFireCount
};



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
//LiquidCrystal lcd(8, 9, 10, 11, 12, 13); 

// --- fonctions ---
boolean get_sensors(){
	if (HIGH == digitalRead(pirPin)) {
		return true;
	}
	return false;
}

void set_sirene(boolean value){
	if (value == true)	{
		digitalWrite(alarmPin, HIGH);
		digitalWrite(relayPin, HIGH);
	}
	if (value == false)	{
		digitalWrite(alarmPin, LOW);
		digitalWrite(relayPin, LOW);
	}
}

void send_sms(){}

void set_lcd(String line1, String line2){
	Serial.println(line1);
}

boolean checkPassword(void){
  if (pwd.evaluate()){
 		pwd.reset();
		return true;
	}
	return false;
}

void kpdEvent (KeypadEvent Key)
{
  switch (kpd.getState())
  {
    case PRESSED :
//      Serial.println(Key); // mj
      switch (Key)
      {
        // appui sur '*' -> vérification de la saisie en cours
        case '*' : checkPassword(); break;
        // appui sur '#' -> réinitialisation de la saisie en cours
        case '#' : pwd.reset(); break;
        // sinon on ajoute le chiffre à la combinaison
        default  : pwd.append(Key); break;
      }
    default : break;
  }
}


// --- setup ---
void setup()
{
	pinMode(alarmPin, OUTPUT);
	pinMode(pirPin, INPUT);
	pinMode(relayPin, OUTPUT);
  kpd.addEventListener(kpdEvent); //keypad event listener
  Serial.begin(9600); // serial debug
  // lcd.begin(16, 2);	// set up the LCD's number of columns and rows: 
	set_lcd("alarm off", "");
}

void goto_off_if_password(){
	if(checkPassword()) {
		next_alarmState = off;
		set_lcd("alarm off", "");
	}
}
void goto_on_if_password(){
	if(checkPassword()) {
		next_alarmState = wait_on;
		set_lcd("wait_on", "");
		wait_on_timer.reset();
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
				set_lcd("alarm on", "");
				next_alarmState = on;
			}
    	break;

		// on - system is running
	  case on:
			set_sirene(false);
			if(get_sensors()) {
				next_alarmState = detection;
				set_lcd("detection", "");
			}
			goto_off_if_password();
    	break;

		// detection - movement detected
	  case detection:
			set_sirene(false);
			send_sms();
			goto_off_if_password();
			before_sirene_timer.reset();
			next_alarmState = 	before_sirene;
	    break;
	
		// before_sirene - delay before sirene (delai d'entrée dans maison) 
	  case before_sirene:
			set_sirene(false);
			if(before_sirene_timer.check() == 1){
				ring_sirene_timer.reset();
				next_alarmState = ring_sirene;
				set_lcd("sirene", "");
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
				set_lcd("wait_on", "");			
			}
	    break;
	}
	alarmState = next_alarmState;
}

// enum type_alarmState  {
// 	off,				// system idle
// 	wait_on,		// delay before running (delai de sortie maison)
// 	on, 				// system is running
// 	detection,	// movement detected
// 	before_sirene,// delay before sirene (delai d'entrée dans maison) 
// 	ring_sirene // sirene is ringing
// 	};

void setState(const type_alarmState _newState)
{
	switch(currState)
  {
    case off:
      leave_off();
      break;  
    case on:
      leave_on();
      break;
	
}
case off:
  leave_off();
  break;


