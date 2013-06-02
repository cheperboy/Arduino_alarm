

void set_lcd_0(char* data){
  slcd.setCursor(0, 0);
  slcd.print("                ");
  slcd.setCursor(0, 0);
  slcd.print(data);
}

void set_lcd_pin(){
  slcd.setCursor(0, 1);
  slcd.print(pwd.getPassword());
//  printPositionPIN +=1;
}
void clear_lcd_pin(){
  slcd.setCursor(0, 1);
  slcd.print("                ");
}

