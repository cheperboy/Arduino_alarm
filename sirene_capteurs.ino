boolean get_sensors(){
	if (HIGH == digitalRead(pirPin)) {
		return true;
	}
	return false;
}

void set_sirene(boolean value){
	if (value == true)	{
		digitalWrite(alarmPin, HIGH);
		// digitalWrite(relayPin, HIGH);
	}
	if (value == false)	{
		digitalWrite(alarmPin, LOW);
		// digitalWrite(relayPin, LOW);
	}
}

void send_sms(){}

