
void set_lcd(char* line1, char* line2){
  slcd.setCursor(0, 0);
  slcd.print("                ");
  slcd.setCursor(0, 0);
  slcd.print(line1);
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

