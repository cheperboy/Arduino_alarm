boolean checkPassword(void){
  if (pwd.evaluate()){
 		pwd.reset();
		return true;
	}
	return false;
}

boolean key_OK(void){
  if (pressed_OK){
		pressed_OK = false;
		return true;
	}
	return false;
}

void kpdEvent (KeypadEvent Key)
{
  switch (kpd.getState())
  {
    case PRESSED :
      Serial.println(Key); // mj
      switch (Key)
      {
        // 'A' -> touche OK
        case 'A' : 
          pressed_OK = true; 
        break;
        // 'B' -> touche CANCEL
        case 'B' : 
          pressed_CANCEL = true; 
          pwd.reset();
          clear_lcd_pin(); 
        break;
        // sinon on ajoute le chiffre à la combinaison
        case '0' : 
        case '1' : 
        case '2' : 
        case '3' : 
        case '4' : 
        case '5' : 
        case '6' : 
        case '7' : 
        case '8' : 
        case '9' : 
        	pwd.append(Key); 
				break;
				
				default  : 
				break;
      }
    set_lcd_pin();
    default : break;
  }
}

