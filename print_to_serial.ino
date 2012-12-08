void print_to_serial()
{    
  Serial.println(ptx.ph); 
  Serial.println(ptx.temp); 
  Serial.println(ptx.ec); 
  delay(100);
}
