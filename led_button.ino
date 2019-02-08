#define Control D0
#define Control2 D1
#define Buttonk1 D5
#define Buttonk2 D6
String ledCMD;
int tempD0,tempD1;
void setup() {
  Serial.begin(9600);
  pinMode(Control,OUTPUT);
  pinMode(Buttonk1,INPUT);
  pinMode(Control2,OUTPUT);
  pinMode(Buttonk2,INPUT);
  digitalWrite(Control,LOW);
  digitalWrite(Control2,LOW);
}

void loop() {
  tempD0 = digitalRead(Buttonk1);
  tempD1 = digitalRead(Buttonk2);
  if(tempD0 == 0){
      digitalWrite(Control,HIGH);
    }else{
      digitalWrite(Control,LOW);
   }
   if(tempD1 == 0){
      digitalWrite(Control2,HIGH);
    }else{
      digitalWrite(Control2,LOW);
   }
   
}
