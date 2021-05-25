//Ventilotto - hacked Ventilator for better Air Quality
//GT+KE Hackathon WS 2020/2021
//by Ismael Rittmann, Jonas Golchini, Helena Kehl
//for educational purpose

//Bibliotheken

#include "Servo.h"
#include "Ticker.h"

//Anschlüsse, An welchen Pins die Komponenten angeschlossen sind

const int TASTER = 2;
const int LED_G = 3;
const int LED_R = 6;
const int LED_B = 5;
const int Luft_Mess_G = A0;

//Servos

Servo servo_1; //AN_AUS
Servo servo_2; //AUF_ZU

//Ticker

  //Programm, das die Sensorwerte ausliest und speichert
  void writeValues();
  Ticker getSensorValues(writeValues, 300);

  //Zeitmesser, misst und wartet eine Stunde
  void writeTime();
  Ticker missEineStunde(writeTime, 60000);   //for 1h write 60000 for 1 Sec write 1000

  //Knopf Abfrage
  void writeButtonStatus();
  Ticker readButtonValue(writeButtonStatus, 100);

  //Auslöser für Hauptprogramm, sobald eine der Beiden Auslöserbedingungen erfüllt ist, wird der Ventilotto aktiviert
  void checkValues();
  Ticker Ausloeser(checkValues, 100);

  //RGB-LED Steuerung
  void rgbLED();
  Ticker lightOutput(rgbLED, 100);

  //Debug, Ausgabe von Werten für den PC zur Überprüfung
  void valuesOutput();
  Ticker debugOutput(valuesOutput, 100);


//Variablen

int sensorValue = 100;
int smoothSensorValue = 100;
int green = 0;
int red = 0;
int blue = 0;
int minutesPassed = 0;
bool Idle = false;
bool buttonActivated = false;
int tasterstatus = 1;
int Checkpoint = 0;


void setup() {

  //Modi der jeweiligen Anschlüsse festlegen

  servo_1.attach(8);                //Servo_An_Aus
  servo_1.write(20);

  servo_2.attach(9);                //Servo_Auf_Zu
  servo_2.write(120);

  pinMode(LED_G, OUTPUT);           //grün
  pinMode(LED_R, OUTPUT);           //rot
  pinMode(LED_B, OUTPUT);           //blau
  pinMode(Luft_Mess_G, INPUT);      //Luftmessgeraet
  pinMode(TASTER, INPUT);           //Taster

  //die einzelnen Ticker-Programme starten

  getSensorValues.start();
  lightOutput.start();
  debugOutput.start();
  missEineStunde.start();
  readButtonValue.start();
  Ausloeser.start();

  //den Serial Port festlegen

  Serial.begin(9600);

}

//loop
  
void loop() {
  
  getSensorValues.update();         //zuerst werden die Werte des CO-Sensors gelesen
  missEineStunde.update();          //dann wird abgefragt, wieviel Zeit seit der letzten Aktivierung verstrichen ist
  readButtonValue.update();         //hier wird überprüft, ob der Knopf gedrückt wurde
  Ausloeser.update();               //hier wird geprüft, ob eine der Beiden Auslösebedingungen erfüllt sind.
                                    //Diese wären: Entweder ist eine Stunde seit der letzten Aktivierung vergangen oder der CO-Sensor misst einen kritischen Wert 
  lightOutput.update();             //hier wird berechnet, in welche Farbe - je nach Sensorwert - die RGB_LED leuchtet. Grün bedeutet gute Luft, Rot schlechte Luft.
  debugOutput.update();             //Zum Schluss hat man hier die Möglichkeit sich verschiedene Variablen am PC anzeigen zu lassen.


  if (Idle == true and Checkpoint == 0 and buttonActivated == false){
                                    //Jetzt wird geprüft, ob der Ventilotto im richtigen Zustand ist, um ausgelöst zu werden. D.h. es wird geprüft, ob er nicht gerade schon ausgelöst ist.
      servo_1.write(0);             //Wenn das nicht der Fall ist, wird eine gewisse Reihenfolge an Aktionen nacheinander ausgeführt und alle anderen Ticker-Programme sind inaktiv.
      delay(2000);
      servo_1.write(20);
      delay(5000);
      servo_2.write(60);
      Checkpoint = 1;
      
    }
    
   if (Idle == true and Checkpoint == 1 and buttonActivated == true){
                                     //Jetzt wird geprüft, ob der Knopf aktiviert wurde und ob der Ventilotto schon ausgelöst wurde.
      servo_2.write(120);            //Trifft beides zu, so schließt die Klappe wieder und einige Variablen werden zurückgesetzt.
      delay(1000);
      minutesPassed = 0;
      Idle = false;
      Checkpoint = 0;
      buttonActivated = false;
      blue = 0;
    
    }
  
}

//Sensor lesen                        //Dieses Programm liest den CO-Sensor aus und glättet die Werte, damit keine Messschwankungen oder
void writeValues (){                  //kurze Extremspitzen den Ventilotto auslösen. Erst wenn der Wert über längere Zeit (2 Sekunden)
                                      //zu hoch ist, wird der Ventilotto ausgelöst werden.
  sensorValue = analogRead(A0);
  smoothSensorValue = (smoothSensorValue * 10) + (analogRead(A0) * 6);
  smoothSensorValue = (smoothSensorValue + 8) >> 4;
                                      //Um Werte zu glätten, werden die neuen Messwerte in einen Durchschnitt mit dem vorherigen Messwert verrechnet.
  }                                   //Diese kurze Schreibweise funktioniert ohne Teilungen in Zahlenwerte mit Nachkommastellen, da der Arduino diese
                                      //langsamer verarbeiten würde.
                                      
//1h Warten                           //Die Variable wird jede Minute um 1 erhöht. Ist ihr Wert bei 60 angekommen, ist eine Stunde vergangen.
void writeTime () {                   //Falls der Ventilotto gerade ausgelöst wurde, werden manche Programme durch die Idle Variable in den Ruhemodus versetzt.
  if (Idle == false){
    minutesPassed = minutesPassed + 1 ;
    }
  else {
    minutesPassed = 0 ;               //Im Ruhemodus wird die Zeitzählung also nicht fortgeführt, sondern auf 0 zurückgesetzt.
    }
  
  }

//Knopf Abfrage                       //Hier wird der ausgelesene Wert schlicht und einfach einer Varaible zugeordnet.
void writeButtonStatus(){
  tasterstatus = digitalRead(TASTER); //0 heißt der Knopf ist gedrückt. 

  }

//Auslöser                            //In diesem Programm werden die Sensorwerte und Zeitmessung mit den Grenzwerten verglichen
void checkValues(){
  if (smoothSensorValue >= 200 or minutesPassed >= 60){
    Idle = true;
    }
                                      //Außerdem wird ausgelesen, ob der Knopf gedrückt ist.
  if (tasterstatus == 0) {
    buttonActivated = true;
    }
    
  }

//LED Farbe Programm                  //Hier wird mithilfe des Sensorwertes die Farbe der RGB-LED bestimmt
void rgbLED (){
      if (Idle == false){             //Die Sensorwerte sind direkt den Farbwerten zugewiesen, was eine simple Lösung für einen Weichen Übergang ohne Abstufungen ermöglicht. 
                                      //Überschreitet der Sensorwert jedoch 255 (RGB 8bit kann jedem der Farbkanäle einen Wert von 0 bis 255 zuordnen), muss er hier gekappt werden.
        if (smoothSensorValue >= 255){
            green = 0;
            red = 255;
        }
        else {
            green = 255-smoothSensorValue ;
            red = smoothSensorValue ;
        }
        
      }
      else { 
          green = 0;
          red = 180;
          blue = 60;
      }
                                      //Am Ende werden die Werte über Stromsignale an den richtigen Pins ausgegeben.
      analogWrite(LED_G,green);
      analogWrite(LED_R,red);
      analogWrite(LED_B,blue); 
  
  }

//Debug Programm
void valuesOutput (){                 //Beispelhafte Überprüfung einiger Werte, indem sie über den Serial Port an einen PC übermittelt werden.
                                      //Das "\t" erzeugt in der Serial-Konsole eine neue Spalte, damit alle Werte nebeneinander angezegt werden.
  Serial.println(smoothSensorValue);  
  Serial.print("\t");
  Serial.println(minutesPassed);  
  Serial.print("\t");
  Serial.println(Idle);  
  Serial.print("\t");
  Serial.println(tasterstatus);  
  
  }
