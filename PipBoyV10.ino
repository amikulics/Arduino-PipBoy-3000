#include "ModifiedSDFatV2.h"
#include "SoftwareSerial.h"
#include "mp3.h"
#include <Servo.h>

#define SD_CS     53
#define PALETTEDEPTH   8     // support 256-colour Palette

//#include <Adafruit_GFX.h> //Already included in ModifiedSDFatV2.h
//#include <MCUFRIEND_kbv.h> //Already included in ModifiedSDFatV2.h

#include <Fonts/Monospaced_bold_12.h>
#include <Fonts/Monospaced_bold_20.h>
#include <Fonts/Monospaced_plain_12.h>

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F

#define PIP_COLOR 0x05A8    //originally 0x07EF, 0x05A8 is closer to the images
#define PIP_TEXT 1

#define RE_OutPutA 14 //main rotary encoder CLK pin
#define RE_OutPutB 15 //main rotary encoder DT pin

boolean isPlaying = false;

Servo myservo;
#define batAnalogPin A10
#define ServoPin 12

int RE_Counter=0;
int RE_aState,RE_aLastState=0;
int selection=0;
int currentListItem=0;

byte Primary_ResPin= A9; //variables for resistor pack on Primary switch (glove)
byte Secondary_ResPin= A5; //variables for resistor pack on secondary switct
byte Rot_Pos;

byte PrimarySwitch=0;
byte SecondarySwitch=0;
byte CurrentScreenMain=0;
byte CurrentScreenSecond=0;

byte STATS_LED=A6;
byte ITEMS_LED=A7;
byte DATA_LED=A8;
byte batt_LED=9;

//stat screen stats
byte SPECIAL[7]={10,10,10,10,10,10,10};
byte Skills[13]={100,100,100,100,100,100,100,100,100,100,100,100,100};
int Level=50,HP=500,AP=200;
char XP[]={"MAX"}, Name[]={"Adam"};

//item screen stats
int weight=290;
double DT=15.0;
int caps=15000;

void setup() 
{
  
    Serial.begin(9600);
    uint32_t when = millis();
    
    if (!Serial) delay(5000);           //allow some time for Leonardo
    Serial.println("Serial took " + String((millis() - when)) + "ms to start");
    uint16_t ID = tft.readID(); 
    Serial.print("ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9481; // write-only shield

    tft.reset();
    tft.begin(ID);
    tft.fillScreen(BLACK);
    
    tft.setRotation(1);
    tft.setCursor(10, 10),tft.setTextColor(PIP_COLOR),tft.setTextSize(PIP_TEXT);
    tft.setFont(&Monospaced_bold_20),tft.print("ROBCO INDUSTRIES");
    tft.setFont(&Monospaced_bold_12);

    pinMode(RE_OutPutA,INPUT);//Rotary encoder pins
    pinMode(RE_OutPutB,INPUT);
    RE_aLastState=digitalRead(RE_OutPutA);

    pinMode(STATS_LED, OUTPUT);
    pinMode(ITEMS_LED, OUTPUT);
    pinMode(DATA_LED, OUTPUT);
     pinMode(batt_LED, OUTPUT);

    bool good = SD.begin(SD_CS);
    if (!good) 
    {
        Serial.print(F("cannot start SD"));
        while (1);
    }
    else
    {
      Serial.print("SD Card Found!");
      SD.open("/");
    }
    
    pinMode(buttonPause, INPUT);
    //digitalWrite(buttonPause,HIGH);//dfplayer
    
    mySerial.begin (9600);
    delay(1000);
    playFirst();
    pause();
    isPlaying = 0;

    myservo.attach(ServoPin);
    digitalWrite(batt_LED,HIGH);
}

void loop() 
{
  PrimarySwitch=checkRes(Primary_ResPin);
  //Serial.print("Primary: ");
  //Serial.println(PrimarySwitch);
  
  SecondarySwitch=checkRes(Secondary_ResPin);
  //Serial.print("Secondary: ");
  //Serial.println(SecondarySwitch);
  
  selection=RE_Check();
  Serial.println(selection);
  
  if(PrimarySwitch!=CurrentScreenMain || SecondarySwitch!=CurrentScreenSecond)
  {
    selection=1;
    switch(PrimarySwitch)
    {
      case 1:
        analogWrite(STATS_LED,255);
        analogWrite(ITEMS_LED,0);
        analogWrite(DATA_LED,0);
        drawStatScreen();
        CurrentScreenMain=1;
        break;
        
      case 2:
        analogWrite(STATS_LED,0);
        analogWrite(ITEMS_LED,255);
        analogWrite(DATA_LED,0);
        drawItemsScreen();
        CurrentScreenMain=2;
        break;
        
      case 3:
        analogWrite(STATS_LED,0);
        analogWrite(ITEMS_LED,0);
        analogWrite(DATA_LED,255);
        drawDataScreen();
        CurrentScreenMain=3;
        break;
    }
  }
  //Serial.println(selection);
  if(selection!=currentListItem)
  {
    switch(PrimarySwitch)
    {
      case 1:
        switch(SecondarySwitch){
          case 1:
          drawStatus();
          break;
          case 2:
          drawSPECIAL();
          break;
          case 3:
          drawSkills();
          break;
          case 4:
          drawPerks();
          break;
          case 5:
          drawGeneral();
          break;
        }
      break;
      
      case 2:
        switch(SecondarySwitch){
          case 1:
          drawWeapons();
          break;
          case 2:
          drawApparel();
          break;
          case 3:
          drawAid();
          break;
          case 4:
          drawItemsMisc();
          break;
          case 5:
          drawAmmo();
          break;
        }
      break;
       
      case 3:
        switch(SecondarySwitch){
          case 1:
          drawMap();
          break;
          case 2:
          drawMap();
          break;
          case 3:
          break;
          case 4:
          break;
          case 5:
          drawRadio();
          break;
        }
      break; 
    } 
  }

  if(CurrentScreenMain==3&&CurrentScreenSecond==5)
  {
    drawRadio();
  }
  gaugeBatt();
}

void drawFrame()
{
  tft.setRotation(1);
  
  tft.setTextColor(PIP_COLOR);
  tft.setTextSize(PIP_TEXT);
  tft.setFont(&Monospaced_bold_12);

  tft.drawLine(10,224,25,224, PIP_COLOR),tft.drawLine(10,224,10,210, PIP_COLOR);//bottom left corner

  tft.drawLine(82,224,95,224, PIP_COLOR);
  tft.drawLine(155,224,175,224, PIP_COLOR);
  tft.drawLine(230,224,250,224, PIP_COLOR);
  tft.drawLine(295,224,320,224, PIP_COLOR);

  tft.drawLine(375,224,390,224, PIP_COLOR),tft.drawLine(390,224,390,210, PIP_COLOR);//bottom right corner

  tft.drawLine(10,10,30,10, PIP_COLOR),tft.drawLine(10,10,10,24, PIP_COLOR);//top-left corner
 
  tft.drawLine(105,10,150,10, PIP_COLOR),tft.drawLine(150,10,150,24, PIP_COLOR);
  
  tft.drawLine(160,10,240,10, PIP_COLOR),tft.drawLine(240,10,240,24, PIP_COLOR);//HP readout (on stat screen)
  tft.drawLine(250,10,315,10, PIP_COLOR),tft.drawLine(315,10,315,24, PIP_COLOR);//AP readout (on stat screen)
  tft.drawLine(325,10,390,10, PIP_COLOR),tft.drawLine(390,10,390,24, PIP_COLOR);//XP readout (on stat screen)

 
}

void drawStatScreen()
{
  
  tft.fillScreen(BLACK);
  drawFrame();

  tft.setTextColor(PIP_COLOR);
  tft.setTextSize(PIP_TEXT);

  tft.setCursor(30,229),tft.print("Status");
  tft.setCursor(98,229),tft.print("SPECIAL");
  tft.setCursor(178,229),tft.print("Skills");
  tft.setCursor(253,229),tft.print("Perks");
  tft.setCursor(321,229),tft.print("General");
  tft.setCursor(35,18),tft.setTextSize(PIP_TEXT),tft.setFont(&Monospaced_bold_20),tft.print("STATS");

  tft.setFont(),tft.setCursor(110,15),tft.print("LVL "),tft.print(Level);
  tft.setCursor(175,15),tft.print("HP "),tft.print(HP),tft.print('/'),tft.print(HP);
  tft.setCursor(253,15),tft.print("AP "),tft.print(AP),tft.print('/'),tft.print(AP);
  tft.setCursor(340,15),tft.print("XP "),tft.print(XP);
  
  
    switch(SecondarySwitch)
    {
    case 1: {//status
      tft.drawLine(25, 215,82,215,PIP_COLOR),tft.drawLine(25, 233,82,233,PIP_COLOR);//first box (left)
      tft.drawLine(25, 215,25,233,PIP_COLOR),tft.drawLine(82, 215,82,233,PIP_COLOR);
      drawStatus();
      CurrentScreenSecond=1;
      break;
     }
    case 2: {//special
      tft.drawLine(95, 215,155,215,PIP_COLOR),tft.drawLine(95, 233,155,233,PIP_COLOR);//second box
      tft.drawLine(95, 215,95,233,PIP_COLOR),tft.drawLine(155, 215,155,233,PIP_COLOR);
      drawSPECIAL();
      CurrentScreenSecond=2;
      break;
     }
    case 3: {//skills
      tft.drawLine(175, 215,230,215,PIP_COLOR),tft.drawLine(175, 233,230,233,PIP_COLOR);//third box
      tft.drawLine(175, 215,175,233,PIP_COLOR),tft.drawLine(230, 215,230,233,PIP_COLOR);
      drawSkills();
      CurrentScreenSecond=3;
      break;
     }
    case 4: {//perks
      tft.drawLine(250, 215,295,215,PIP_COLOR),tft.drawLine(250, 233,295,233,PIP_COLOR);//fourth box
      tft.drawLine(250, 215,250,233,PIP_COLOR),tft.drawLine(295, 215,295,233,PIP_COLOR);
      drawPerks();
      CurrentScreenSecond=4;
      break;
     }
    case 5: {//general
      tft.drawLine(320, 215,375,215,PIP_COLOR),tft.drawLine(320, 233,375,233,PIP_COLOR);//fifth box (right)
      tft.drawLine(320, 215,320,233,PIP_COLOR),tft.drawLine(375, 215,375,233,PIP_COLOR);
      drawGeneral();
      CurrentScreenSecond=5;
      break;
     }
    }
}

void drawItemsScreen()
{
  tft.fillScreen(BLACK);
  drawFrame();

  tft.setTextColor(PIP_COLOR);
  tft.setTextSize(PIP_TEXT);

  tft.setCursor(26,229),tft.print("Weapons");
  tft.setCursor(98,229),tft.print("Apparel");
  tft.setCursor(190,229),tft.print("Aid");
  tft.setCursor(256,229),tft.print("Misc");
  tft.setCursor(330,229),tft.print("Ammo");
  tft.setCursor(35,18),tft.setTextSize(PIP_TEXT),tft.setFont(&Monospaced_bold_20),tft.print("ITEMS");

  tft.setFont(),tft.setCursor(110,15),tft.print("WG "),tft.print(weight);
  tft.setCursor(175,15),tft.print("HP "),tft.print(HP),tft.print('/'),tft.print(HP);
  tft.setCursor(253,15),tft.print("DT "),tft.print(DT);
  tft.setCursor(330,15),tft.print("Caps "),tft.print(caps);
  
  switch(SecondarySwitch)
    {
    case 1: {//weapons
      tft.drawLine(25, 215,82,215,PIP_COLOR),tft.drawLine(25, 233,82,233,PIP_COLOR);//first box (left)
      tft.drawLine(25, 215,25,233,PIP_COLOR),tft.drawLine(82, 215,82,233,PIP_COLOR);
      drawWeapons();
      CurrentScreenSecond=1;
      break;
     }
    case 2: {//apparel
      tft.drawLine(95, 215,155,215,PIP_COLOR),tft.drawLine(95, 233,155,233,PIP_COLOR);//second box
      tft.drawLine(95, 215,95,233,PIP_COLOR),tft.drawLine(155, 215,155,233,PIP_COLOR);
      drawApparel();
      CurrentScreenSecond=2;
      break;
     }
    case 3: {//aid
      tft.drawLine(175, 215,230,215,PIP_COLOR),tft.drawLine(175, 233,230,233,PIP_COLOR);//third box
      tft.drawLine(175, 215,175,233,PIP_COLOR),tft.drawLine(230, 215,230,233,PIP_COLOR);
      drawAid();
      CurrentScreenSecond=3;
      break;
     }
    case 4: {//misc
      tft.drawLine(250, 215,295,215,PIP_COLOR),tft.drawLine(250, 233,295,233,PIP_COLOR);//fourth box
      tft.drawLine(250, 215,250,233,PIP_COLOR),tft.drawLine(295, 215,295,233,PIP_COLOR);
      drawItemsMisc();
      CurrentScreenSecond=4;
      break;
     }
    case 5: {//ammo
      tft.drawLine(320, 215,375,215,PIP_COLOR),tft.drawLine(320, 233,375,233,PIP_COLOR);//fifth box (right)
      tft.drawLine(320, 215,320,233,PIP_COLOR),tft.drawLine(375, 215,375,233,PIP_COLOR);
      drawAmmo();
      CurrentScreenSecond=5;
      break;
     }
    }
}

void drawDataScreen()
{
  tft.fillScreen(BLACK);
  drawFrame();

  tft.setTextColor(PIP_COLOR);
  tft.setTextSize(PIP_TEXT);

  tft.setCursor(34,229),tft.print("Local");
  tft.setCursor(105,229),tft.print("World");
  tft.setCursor(180,229),tft.print("Quests");
  tft.setCursor(256,229),tft.print("Misc");
  tft.setCursor(328,229),tft.print("Radio");
  tft.setCursor(40,18),tft.setTextSize(PIP_TEXT),tft.setFont(&Monospaced_bold_20),tft.print("DATA");
  

  switch(SecondarySwitch)
    {
    case 1: {//local map
      tft.drawLine(25, 215,82,215,PIP_COLOR),tft.drawLine(25, 233,82,233,PIP_COLOR);//first box (left)
      tft.drawLine(25, 215,25,233,PIP_COLOR),tft.drawLine(82, 215,82,233,PIP_COLOR);
      drawMap();
      CurrentScreenSecond=1;
      break;
     }
    case 2: {//world map
      tft.drawLine(95, 215,155,215,PIP_COLOR),tft.drawLine(95, 233,155,233,PIP_COLOR);//second box
      tft.drawLine(95, 215,95,233,PIP_COLOR),tft.drawLine(155, 215,155,233,PIP_COLOR);
      drawMap();
      CurrentScreenSecond=2;
      break;
     }
    case 3: {//quests
      tft.drawLine(175, 215,230,215,PIP_COLOR),tft.drawLine(175, 233,230,233,PIP_COLOR);//third box
      tft.drawLine(175, 215,175,233,PIP_COLOR),tft.drawLine(230, 215,230,233,PIP_COLOR);
      drawQuests();
      CurrentScreenSecond=3;
      break;
     }
    case 4: {//misc
      tft.drawLine(250, 215,295,215,PIP_COLOR),tft.drawLine(250, 233,295,233,PIP_COLOR);//fourth box
      tft.drawLine(250, 215,250,233,PIP_COLOR),tft.drawLine(295, 215,295,233,PIP_COLOR);
      drawDataMisc();
      CurrentScreenSecond=4;
      break;
     }
    case 5: {//radio
      tft.drawLine(320, 215,375,215,PIP_COLOR),tft.drawLine(320, 233,375,233,PIP_COLOR);//fifth box (right)
      tft.drawLine(320, 215,320,233,PIP_COLOR),tft.drawLine(375, 215,375,233,PIP_COLOR);
      drawRadio();
      CurrentScreenSecond=5;
      break;
     }
    }
}

void drawStatus()
{
  if(selection<1)
    selection=1;
    
  if(selection>3)
    selection=3;
    
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(20,50),tft.print("CND");
  tft.setCursor(20,70),tft.print("RAD");
  tft.setCursor(20,90),tft.print("EFF");

  switch(selection)
  {
    case 1:
    tft.drawRect(18, 58, 28, 15, BLACK);
    tft.drawRect(18, 78, 28, 15, BLACK);
    tft.drawRect(18, 38, 28, 15, PIP_COLOR);
    tft.fillRect(55,30,336,180,BLACK),tft.fillRect(10,100,100,110,BLACK);

    tft.setCursor(327, 50),tft.print("Stimpack"),tft.setCursor(295, 65),tft.print("Doctor's Bag");
    tft.setCursor(198, 205),tft.print("-");
    tft.setCursor(155,205),tft.print(Name),tft.setCursor(220,205),tft.print("Level "),tft.print(Level);
    showBMP("/stats/status/StatScreenGuy.bmp",120,30);
    currentListItem=1;
    break;
    
    case 2:
    tft.drawRect(18, 38, 28, 15, BLACK);
    tft.drawRect(18, 78, 28, 15, BLACK);
    tft.drawRect(18, 58, 28, 15, PIP_COLOR);
    tft.fillRect(55,30,336,180,BLACK),tft.fillRect(10,100,100,110,BLACK);

    tft.setCursor(328, 50),tft.print("RadAway"),tft.setCursor(330, 65),tft.print("Rad-X");
    tft.drawLine(10,150,140,150,PIP_COLOR),tft.drawLine(140,150,140,170,PIP_COLOR);//rad resist box
    tft.drawLine(150,150,390,150,PIP_COLOR),tft.drawLine(390,150,390,170,PIP_COLOR);//graph box
    tft.setCursor(10,165),tft.print("RAD RESIST"),tft.setCursor(120,165),tft.print("8%");
    currentListItem=2;
    break;
    
    case 3:
    tft.drawRect(18, 38, 28, 15, BLACK);
    tft.drawRect(18, 58, 28, 15, BLACK);
    tft.drawRect(18, 78, 28, 15, PIP_COLOR);
    tft.fillRect(55,30,336,180,BLACK),tft.fillRect(10,100,100,110,BLACK);
    
    currentListItem=3;
    break;
  }
}

void drawSPECIAL()
{
  if(selection<1)
    selection=1;
    
  if(selection>8)
    selection=8;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print("Strength"),tft.setCursor(150,40),tft.print(SPECIAL[0]);
  tft.setCursor(30,60),tft.print("Perception"),tft.setCursor(150,60),tft.print(SPECIAL[1]);
  tft.setCursor(30,80),tft.print("Endurance"),tft.setCursor(150,80),tft.print(SPECIAL[2]);
  tft.setCursor(30,100),tft.print("Charisma"),tft.setCursor(150,100),tft.print(SPECIAL[3]);
  tft.setCursor(30,120),tft.print("Intelligence"),tft.setCursor(150,120),tft.print(SPECIAL[4]);
  tft.setCursor(30,140),tft.print("Agility"),tft.setCursor(150,140),tft.print(SPECIAL[5]);
  tft.setCursor(30,160),tft.print("Luck"),tft.setCursor(150,160),tft.print(SPECIAL[6]);
  tft.drawLine(190,136, 390, 136, PIP_COLOR),tft.drawLine(390,136, 390, 150, PIP_COLOR);//line between image and text
  
  switch(selection)
  {
    case 1://strength
    clearList(1);
    tft.drawLine(28,44,180,44, PIP_COLOR),tft.drawLine(28,44,28,28, PIP_COLOR);//box
    tft.drawLine(28,28,180,28, PIP_COLOR),tft.drawLine(180,44,180,28, PIP_COLOR);//box
    
    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Strength is a measure of your raw"));
    tft.setCursor(190, 150),tft.print(F("power. It affects how much you"));
    tft.setCursor(190, 160),tft.print(F("can carry, the power of all melee"));
    tft.setCursor(190, 170),tft.print(F("attacks, and your effectiveness"));
    tft.setCursor(190, 180),tft.print(F("with many heavy weapons."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/Strength.bmp",250,30);
    currentListItem=1;
    break;
    
    case 2://perception
    clearList(1);
    tft.drawLine(28,64, 180,64, PIP_COLOR),tft.drawLine(28,64,28,48, PIP_COLOR);//box
    tft.drawLine(28,48, 180,48, PIP_COLOR),tft.drawLine(180,64,180,48, PIP_COLOR);//box

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("A high Perception grants a bonus"));
    tft.setCursor(190, 150),tft.print(F("to the Explosives, Lockpick, and"));
    tft.setCursor(190, 160),tft.print(F("Energy Weapons skills, and "));
    tft.setCursor(190, 170),tft.print(F("determines when red compass marks"));
    tft.setCursor(190, 180),tft.print(F("appear. (indicating threats)"));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/Perception.bmp",250,30);
    currentListItem=2;
    break;
    
    case 3://endurance
    clearList(1);
    tft.drawLine(28,84, 180,84, PIP_COLOR),tft.drawLine(28,84,28,68, PIP_COLOR);
    tft.drawLine(28,68, 180,68, PIP_COLOR),tft.drawLine(180,84,180,68, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Endurance is a measure of your"));
    tft.setCursor(190, 150),tft.print(F("overall physical fitness. A high"));
    tft.setCursor(190, 160),tft.print(F("Endurance gives bonuses to health"));
    tft.setCursor(190, 170),tft.print(F(",environmental resistances, and "));
    tft.setCursor(190, 180),tft.print(F("the Survival and Unarmed skills."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/endurance.bmp",250,30);
    currentListItem=3;
    break;
    
    case 4://charisma
    clearList(1);
    tft.drawLine(28,104, 180,104, PIP_COLOR),tft.drawLine(28,104,28,88, PIP_COLOR);
    tft.drawLine(28,88, 180,88, PIP_COLOR),tft.drawLine(180,104,180,88, PIP_COLOR);
    
    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Having a high Charisma will"));
    tft.setCursor(190, 150),tft.print(F("imporve people's disposition"));
    tft.setCursor(190, 160),tft.print(F("toward you, and give bonuses to"));
    tft.setCursor(190, 170),tft.print(F("both the Barter and Speech skills"));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/charisma.bmp",250,30);
    currentListItem=4;
    break;
    
    case 5://intelligence
    clearList(1);
    tft.drawLine(28,124, 180,124, PIP_COLOR),tft.drawLine(28,124,28,108, PIP_COLOR);
    tft.drawLine(28,108, 180,108, PIP_COLOR),tft.drawLine(180,124,180,108, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Intelligence affects the Science,"));
    tft.setCursor(190, 150),tft.print(F("Repair and Medicine skills. The"));
    tft.setCursor(190, 160),tft.print(F("higher your Intelligence, the"));
    tft.setCursor(190, 170),tft.print(F("more Skill points you'll be able"));
    tft.setCursor(190, 180),tft.print(F("to distribute when you level up."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/intelligence.bmp",250,30);
    currentListItem=5;
    break;
    
    case 6://agility
    clearList(1);
    tft.drawLine(28,144, 180,144, PIP_COLOR),tft.drawLine(28,144,28,128, PIP_COLOR);
    tft.drawLine(28,128, 180,128, PIP_COLOR),tft.drawLine(180,144,180,128, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Agility affects your Guns and"));
    tft.setCursor(190, 150),tft.print(F("Sneak skills, and the number of"));
    tft.setCursor(190, 160),tft.print(F("Action Points available for "));
    tft.setCursor(190, 170),tft.print(F("V.A.T.S."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/Agility.bmp",250,30);
    currentListItem=6;
    break;
    
    case 7://luck
    clearList(1);
    tft.drawLine(28,164, 180,164, PIP_COLOR),tft.drawLine(28,164,28,148, PIP_COLOR);
    tft.drawLine(28,148, 180,148, PIP_COLOR),tft.drawLine(180,164,180,148, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Raising your Luck will raise all"));
    tft.setCursor(190, 150),tft.print(F("of your skills a little. Having"));
    tft.setCursor(190, 160),tft.print(F("a high Luck will also improve"));
    tft.setCursor(190, 170),tft.print(F("your critical chance with all"));
    tft.setCursor(190, 180),tft.print(F("weapons."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/SPECIAL/luck.bmp",250,30);
    currentListItem=7;
    break;
  }
}

void drawSkills()
{
  if(selection<1)
    selection=1;
    
  if(selection>14)
    selection=14;
    
  tft.drawLine(190,136, 390, 136, PIP_COLOR),tft.drawLine(390,136, 390, 150, PIP_COLOR);//line between image and text

  if(selection==9||selection==1)
  { 
    tft.fillRect(30,30,152,170,BLACK);
    tft.setFont(&Monospaced_bold_12);
    tft.setCursor(30,40),tft.print("Barter"),tft.setCursor(150,40),tft.print(Skills[0]);
    tft.setCursor(30,60),tft.print("Energy Weapons"),tft.setCursor(150,60),tft.print(Skills[1]);
    tft.setCursor(30,80),tft.print("Explosives"),tft.setCursor(150,80),tft.print(Skills[2]);
    tft.setCursor(30,100),tft.print("Guns"),tft.setCursor(150,100),tft.print(Skills[3]);
    tft.setCursor(30,120),tft.print("Lockpick"),tft.setCursor(150,120),tft.print(Skills[4]);
    tft.setCursor(30,140),tft.print("Medicine"),tft.setCursor(150,140),tft.print(Skills[5]);
    tft.setCursor(30,160),tft.print("Melee Weapons"),tft.setCursor(150,160),tft.print(Skills[6]);
    tft.setCursor(30,180),tft.print("Repair"),tft.setCursor(150,180),tft.print(Skills[7]);
    tft.setCursor(30,200),tft.print("Science"),tft.setCursor(150,200),tft.print(Skills[8]);
  }
  if(selection==10)
  { 
    tft.fillRect(30,30,152,170,BLACK);
    tft.setFont(&Monospaced_bold_12);
    tft.setCursor(30,40),tft.print("Sneak"),tft.setCursor(150,40),tft.print(Skills[9]);//10
    tft.setCursor(30,60),tft.print("Speech"),tft.setCursor(150,60),tft.print(Skills[10]);//11
    tft.setCursor(30,80),tft.print("Survival"),tft.setCursor(150,80),tft.print(Skills[11]);//12
    tft.setCursor(30,100),tft.print("Unarmed"),tft.setCursor(150,100),tft.print(Skills[12]);//13
  }
  
  switch(selection)
  {
    case 1://barter
    clearList(1);
    tft.drawLine(28,44,180,44, PIP_COLOR),tft.drawLine(28,44,28,28, PIP_COLOR);
    tft.drawLine(28,28,180,28, PIP_COLOR),tft.drawLine(180,44,180,28, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Barter skill affects the"));
    tft.setCursor(190, 150),tft.print(F("prices you get from buying and"));
    tft.setCursor(190, 160),tft.print(F("selling. In general, the higher"));
    tft.setCursor(190, 170),tft.print(F("your Barter, the lower your"));
    tft.setCursor(190, 180),tft.print(F("prices on purchased items."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/barter.bmp",250,30);
    currentListItem=1;
    break;
    
    case 2://energy weapons
    clearList(1);
    tft.drawLine(28,64, 180,64, PIP_COLOR),tft.drawLine(28,64,28,48, PIP_COLOR);
    tft.drawLine(28,48, 180,48, PIP_COLOR),tft.drawLine(180,64,180,48, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Energy Weapons skill"));
    tft.setCursor(190, 150),tft.print(F("determines your effectiveness "));
    tft.setCursor(190, 160),tft.print(F("with any weapon that uses Small"));
    tft.setCursor(190, 170),tft.print(F("energy cells, MicroFusion cells,"));
    tft.setCursor(190, 180),tft.print(F("EC Packs, or Flamer fuel."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/energyweapons.bmp",250,30);
    currentListItem=2;
    break;
    
    case 3://explosives
    clearList(1);
    tft.drawLine(28,84, 180,84, PIP_COLOR),tft.drawLine(28,84,28,68, PIP_COLOR);
    tft.drawLine(28,68, 180,68, PIP_COLOR),tft.drawLine(180,84,180,68, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Explosives Skill determines"));
    tft.setCursor(190, 150),tft.print(F("the ease of disarming any hostile"));
    tft.setCursor(190, 160),tft.print(F("mines and the effectiveness of"));
    tft.setCursor(190, 170),tft.print(F("any explosive weapon."));
    
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/explosives.bmp",250,30);
    currentListItem=3;
    break;
    
    case 4://guns
    clearList(1);
    tft.drawLine(28,104, 180,104, PIP_COLOR),tft.drawLine(28,104,28,88, PIP_COLOR);
    tft.drawLine(28,88, 180,88, PIP_COLOR),tft.drawLine(180,104,180,88, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Guns determines your effective-"));
    tft.setCursor(190, 150),tft.print(F("ness with any weapon that uses "));
    tft.setCursor(190, 160),tft.print(F("conventional ammo. "));
    tft.setCursor(190, 170),tft.print(F("(.22 LR, .357 Mag, 5mm, 10mm,"));
    tft.setCursor(190, 180),tft.print(F("5.56mm, .308, .45-70, ETC."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/guns.bmp",230,30);
    currentListItem=4;
    break;
    
    case 5://lockpick
    clearList(1);
    tft.drawLine(28,124, 180,124, PIP_COLOR),tft.drawLine(28,124,28,108, PIP_COLOR);
    tft.drawLine(28,108, 180,108, PIP_COLOR),tft.drawLine(180,124,180,108, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Lockpick Skill is used to "));
    tft.setCursor(190, 150),tft.print(F("open locked doors and containers"));
    
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/lockpick.bmp",250,30);
    currentListItem=5;
    break;
    
    case 6://medicine
    clearList(1);
    tft.drawLine(28,144, 180,144, PIP_COLOR),tft.drawLine(28,144,28,128, PIP_COLOR);
    tft.drawLine(28,128, 180,128, PIP_COLOR),tft.drawLine(180,144,180,128, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Medicine Skill determines how"));
    tft.setCursor(190, 150),tft.print(F("many HitPoints you'll replenish"));
    tft.setCursor(190, 160),tft.print(F("using a stimpack, and the "));
    tft.setCursor(190, 170),tft.print(F("effectiveness of Rad-X and"));
    tft.setCursor(190, 180),tft.print(F("RadAway."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/medicine.bmp",250,30);
    currentListItem=6;
    break;
    
    case 7://melee weapons
    clearList(1);
    tft.drawLine(28,164, 180,164, PIP_COLOR),tft.drawLine(28,164,28,148, PIP_COLOR);
    tft.drawLine(28,148, 180,148, PIP_COLOR),tft.drawLine(180,164,180,148, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Melee Weapons Skill deter-"));
    tft.setCursor(190, 150),tft.print(F("mines your effectivenes with any"));
    tft.setCursor(190, 160),tft.print(F("melee weapon, from the simple "));
    tft.setCursor(190, 170),tft.print(F("lead pipe all the way up to the"));
    tft.setCursor(190, 180),tft.print(F("high-tech SuperSledge."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/meleeweapons.bmp",250,30);
    currentListItem=7;
    break;
    
    case 8://reapir
    clearList(1);
    tft.drawLine(28,184, 180,184, PIP_COLOR),tft.drawLine(28,184,28,168, PIP_COLOR);
    tft.drawLine(28,168, 180,168, PIP_COLOR),tft.drawLine(180,184,180,168, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Repair Skill allows you to"));
    tft.setCursor(190, 150),tft.print(F("maintain any weapons and apparel"));
    tft.setCursor(190, 160),tft.print(F("In addition, Repair allows you"));
    tft.setCursor(190, 170),tft.print(F("to create items and ammunition"));
    tft.setCursor(190, 180),tft.print(F("at reloading benches."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/repair.bmp",250,30);
    currentListItem=8;
    break;
    
    case 9://science
    clearList(1);
    tft.drawLine(28,204, 180,204, PIP_COLOR),tft.drawLine(28,204,28,188, PIP_COLOR);
    tft.drawLine(28,188, 180,188, PIP_COLOR),tft.drawLine(180,204,180,188, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Science Skill represents your"));
    tft.setCursor(190, 150),tft.print(F("comined scientfic knowledge, and"));
    tft.setCursor(190, 160),tft.print(F("is primarily used to hack"));
    tft.setCursor(190, 170),tft.print(F("restricted computer terminals. "));
   
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/science.bmp",250,30);
    currentListItem=9;
    break;


    case 10://sneak
    clearList(1);
    tft.drawLine(28,44,180,44, PIP_COLOR),tft.drawLine(28,44,28,28, PIP_COLOR);
    tft.drawLine(28,28,180,28, PIP_COLOR),tft.drawLine(180,44,180,28, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The higher your Sneak Skill, the"));
    tft.setCursor(190, 150),tft.print(F("easier it is to remain un-"));
    tft.setCursor(190, 160),tft.print(F("detected, steal an item, or pick"));
    tft.setCursor(190, 170),tft.print(F("someone's pocket. As well as"));
    tft.setCursor(190, 180),tft.print(F("bonuses to Sneak attacks."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/sneak.bmp",250,30);
    currentListItem=10;
    break;
    
    case 11://speech
    clearList(1);
    tft.drawLine(28,64, 180,64, PIP_COLOR),tft.drawLine(28,64,28,48, PIP_COLOR);
    tft.drawLine(28,48, 180,48, PIP_COLOR),tft.drawLine(180,64,180,48, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Speech Skill governs how"));
    tft.setCursor(190, 150),tft.print(F("much you can influence someone"));
    tft.setCursor(190, 160),tft.print(F("through dialogue, and gain access"));
    tft.setCursor(190, 170),tft.print(F("to info they might not otherwise"));
    tft.setCursor(190, 180),tft.print(F("want to share."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/speech.bmp",250,30);
    currentListItem=11;
    break;
    
    case 12://survival
    clearList(1);
    tft.drawLine(28,84, 180,84, PIP_COLOR),tft.drawLine(28,84,28,68, PIP_COLOR);
    tft.drawLine(28,68, 180,68, PIP_COLOR),tft.drawLine(180,84,180,68, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Survival Skill increases the"));
    tft.setCursor(190, 150),tft.print(F("HitPoints you recive from food"));
    tft.setCursor(190, 160),tft.print(F("and drink. It also helps you"));
    tft.setCursor(190, 170),tft.print(F("create consumable items at camp-"));
    tft.setCursor(190, 180),tft.print(F("fires."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/survival.bmp",250,30);
    currentListItem=12;
    break;
    
    case 13://unarmed
    clearList(1);
    tft.drawLine(28,104, 180,104, PIP_COLOR),tft.drawLine(28,104,28,88, PIP_COLOR);
    tft.drawLine(28,88, 180,88, PIP_COLOR),tft.drawLine(180,104,180,88, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("The Unarmed Skill is used for"));
    tft.setCursor(190, 150),tft.print(F("fighting without a weapon, or "));
    tft.setCursor(190, 160),tft.print(F("with weapons designed for hand-"));
    tft.setCursor(190, 170),tft.print(F("to-hand combat, like Brass"));
    tft.setCursor(190, 180),tft.print(F("Knuckles or Power Fists."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/skills/unarmed.bmp",250,30);
    currentListItem=13;
    break;
    }
    
}

void drawPerks()
{
  if(selection<1)
    selection=1;
    
  if(selection>8)
    selection=8;
    
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F("Adamantium Skeleton"));
  tft.setCursor(30,60),tft.print("Bloody Mess");
  tft.setCursor(30,80),tft.print("Explorer");
  tft.setCursor(30,100),tft.print("Mysterious Stranger");
  tft.setCursor(30,120),tft.print("Swift Learner");
  tft.setCursor(30,140),tft.print("Trigger Discipline");
  tft.setCursor(30,160),tft.print("Wild Wasteland");
  tft.drawLine(190,136, 390, 136, PIP_COLOR),tft.drawLine(390,136, 390, 150, PIP_COLOR);//line between image and text

  switch(selection)
  {
    case 1://adamantium skeleton
    clearList(1);
    tft.drawLine(28,44,180,44, PIP_COLOR),tft.drawLine(28,44,28,28, PIP_COLOR);//box
    tft.drawLine(28,28,180,28, PIP_COLOR),tft.drawLine(180,44,180,28, PIP_COLOR);//box
    
    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140), tft.print(F("With Adamantium skeleton perk,"));
    tft.setCursor(190, 150),tft.print(F("your limbs only recieve 50% of "));
    tft.setCursor(190, 160),tft.print(F("the damage they normally would."));
    
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/adamantiumskeleton.bmp",250,30);
    currentListItem=1;
    break;
    
    case 2://bloodymess
    clearList(1);
    tft.drawLine(28,64, 180,64, PIP_COLOR),tft.drawLine(28,64,28,48, PIP_COLOR);//box
    tft.drawLine(28,48, 180,48, PIP_COLOR),tft.drawLine(180,64,180,48, PIP_COLOR);//box

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("With the Bloody Mess perk, char-"));
    tft.setCursor(190, 150),tft.print(F("acters and creatures you kill"));
    tft.setCursor(190, 160),tft.print(F("will often explode into a red,"));
    tft.setCursor(190, 170),tft.print(F("gut-ridden, eyeball-strewn paste."));
    tft.setCursor(190, 180),tft.print(F("Fun! Oh, and 5% extra damage."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/bloodymess.bmp",250,30);
    currentListItem=2;
    break;
    
    case 3://explorer
    clearList(1);
    tft.drawLine(28,84, 180,84, PIP_COLOR),tft.drawLine(28,84,28,68, PIP_COLOR);
    tft.drawLine(28,68, 180,68, PIP_COLOR),tft.drawLine(180,84,180,68, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("When you choose the Explorer perk"));
    tft.setCursor(190, 150),tft.print(F("every location in the world is"));
    tft.setCursor(190, 160),tft.print(F("revealed on your map. So get out"));
    tft.setCursor(190, 170),tft.print(F("there and explore!"));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/explorer.bmp",250,30);
    currentListItem=3;
    break;
    
    case 4://mysterious stranger
    clearList(1);
    tft.drawLine(28,104, 180,104, PIP_COLOR),tft.drawLine(28,104,28,88, PIP_COLOR);
    tft.drawLine(28,88, 180,88, PIP_COLOR),tft.drawLine(180,104,180,88, PIP_COLOR);
    
    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("You've gained your own personal"));
    tft.setCursor(190, 150),tft.print(F("guardian angel... armed with a"));
    tft.setCursor(190, 160),tft.print(F(".44 Magnum. With this perk, the"));
    tft.setCursor(190, 170),tft.print(F("Mysterious Stranger will appear"));
    tft.setCursor(190, 180),tft.print(F("occasionally in combat."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/mysteriousstranger.bmp",250,30);
    currentListItem=4;
    break;
    
    case 5://swiftlearner
    clearList(1);
    tft.drawLine(28,124, 180,124, PIP_COLOR),tft.drawLine(28,124,28,108, PIP_COLOR);
    tft.drawLine(28,108, 180,108, PIP_COLOR),tft.drawLine(180,124,180,108, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("With each rank in the Swift Learn-"));
    tft.setCursor(190, 150),tft.print(F("er perk, you gain an additional"));
    tft.setCursor(190, 160),tft.print(F("10% to total Experience Points"));
    tft.setCursor(190, 170),tft.print(F("whenever Experience Points are"));
    tft.setCursor(190, 180),tft.print(F("earned."));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/swiftlearner.bmp",250,30);
    currentListItem=5;
    break;
    
    case 6://trigger discipline
    clearList(1);
    tft.drawLine(28,144, 180,144, PIP_COLOR),tft.drawLine(28,144,28,128, PIP_COLOR);
    tft.drawLine(28,128, 180,128, PIP_COLOR),tft.drawLine(180,144,180,128, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("While using Guns and Energy"));
    tft.setCursor(190, 150),tft.print(F("Weapons, you fire 20% more slowly"));
    tft.setCursor(190, 160),tft.print(F("but are 20% mor accurate."));
    
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/triggerdiscipline.bmp",250,30);
    currentListItem=6;
    break;
    
    case 7://wildwasteland
    clearList(1);
    tft.drawLine(28,164, 180,164, PIP_COLOR),tft.drawLine(28,164,28,148, PIP_COLOR);
    tft.drawLine(28,148, 180,148, PIP_COLOR),tft.drawLine(180,164,180,148, PIP_COLOR);

    tft.setFont();
    tft.fillRect(190,140, 200,70,BLACK);
    tft.setCursor(190, 140),tft.print(F("Wild Wasteland unleashes the"));
    tft.setCursor(190, 150),tft.print(F("most bizarre and silly elements"));
    tft.setCursor(190, 160),tft.print(F("of post-apocalyptic America."));
    tft.setCursor(190, 170),tft.print(F("not for the faint of heart or"));
    tft.setCursor(190, 180),tft.print(F("the serious of temperament"));
    tft.fillRect(190,28, 200,108,BLACK);
    showBMP("/stats/perks/wildwasteland.bmp",250,30);
    currentListItem=7;
    break;
  }
  
}

void drawGeneral()
{
  if(selection<1)
    selection=1;
    
  if(selection>6)
    selection=6;
    
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(20,50),tft.print(F("Brotherhood of Steel"));
  tft.setCursor(20,70),tft.print(F("Caesar's Legion"));
  tft.setCursor(20,90),tft.print("NCR");
  tft.setCursor(20,110),tft.print("Novac");
  tft.setCursor(20,130),tft.print("The Strip");

  switch(selection)
  {
    case 1://bos
    clearList(2);
    tft.fillRect(200,30,190,170,BLACK);
    tft.drawRect(18,37,165,18,PIP_COLOR);
    tft.setCursor(200,190),tft.print(F("Brotherhood of Steel"));
    tft.setCursor(240,175),tft.print(F("Nuetral"));
    showBMP("/stats/general/bos.bmp",205,30);
    currentListItem=1;
    break;
    case 2:
    clearList(2);
    tft.fillRect(200,30,190,170,BLACK);
    tft.drawRect(18,57,165,18,PIP_COLOR);
    tft.setCursor(220,190),tft.print(F("Caesar's Legion"));
    tft.setCursor(245,175),tft.print(F("Nuetral"));
    showBMP("/stats/general/caesarslegion.bmp",225,30);
    currentListItem=2;
    break;
    case 3:
    clearList(2);
    tft.fillRect(200,30,190,170,BLACK);
    tft.drawRect(18,77,165,18,PIP_COLOR);
    tft.setCursor(205,190),tft.print(F("New California Republic"));
    tft.setCursor(255,175),tft.print(F("Nuetral"));
    showBMP("/stats/general/ncr.bmp",225,30);
    currentListItem=3;
    break;
    case 4:
    clearList(2);
    tft.fillRect(200,30,190,170,BLACK);
    tft.drawRect(18,97,165,18,PIP_COLOR);
    tft.setCursor(270,190),tft.print(F("Novac"));
    tft.setCursor(265,175),tft.print(F("Nuetral"));
    showBMP("/stats/general/novac.bmp",225,30);
    currentListItem=4;
    break;
    case 5:
    clearList(2);
    tft.fillRect(200,30,190,170,BLACK);
    tft.drawRect(18,117,165,18,PIP_COLOR);
    tft.setCursor(245,190),tft.print(F("The Strip"));
    tft.setCursor(255,175),tft.print(F("Nuetral"));
    showBMP("/stats/general/thestrip.bmp",225,30);
    currentListItem=5;
    break;
  }
}

void drawWeapons()
{
  if(selection<1)
    selection=1;
    
  if(selection>8)
    selection=8;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F(".45 Auto Pistol"));
  tft.setCursor(30,60),tft.print(F("All American"));
  tft.setCursor(30,80),tft.print(F("Anti-Materiel Rifle"));
  tft.setCursor(30,100),tft.print(F("Big Mountain"));
  tft.setCursor(30,117),tft.print(F("Transportalponder"));
  tft.setCursor(30,137),tft.print(F("Fat Man"));
  tft.setCursor(30,157),tft.print(F("That Gun"));
  tft.setCursor(30,177),tft.print(F("This Machine"));

  tft.drawLine(190,150,270,150,PIP_COLOR),tft.drawLine(270,150,270,164, PIP_COLOR);
  tft.drawLine(275,150, 330, 150, PIP_COLOR),tft.drawLine(330,150,330,164,PIP_COLOR);//upper three boxes under picture
  tft.drawLine(335,150,390,150,PIP_COLOR),tft.drawLine(390,150, 390, 164, PIP_COLOR);
  
  tft.drawLine(190,170,270,170,PIP_COLOR),tft.drawLine(270,170,270,184,PIP_COLOR);
  tft.drawLine(275,170,390,170,PIP_COLOR),tft.drawLine(390, 170, 390, 184, PIP_COLOR);//lower two boxes below picture
  
  switch(selection)
  {
    case 1://1911 :)
    clearList(3);
    tft.drawRect(28,28,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM       39"));
    tft.setCursor(277, 155),tft.print(F("WG  1.50"));
    tft.setCursor(337, 155),tft.print(F("VAL 1200"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F(".45 Auto (7/500)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/45autopistol.bmp",230,30);
    currentListItem=1;
    break;
    
    case 2://all aemrican
    clearList(3);
    tft.drawRect(28,48,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM       35"));
    tft.setCursor(277, 155),tft.print(F("WG  6.00"));
    tft.setCursor(337, 155),tft.print(F("VAL 4760"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("5.56 (24/500)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/allamerican.bmp",230,30);
    currentListItem=2;
    break;
    
    case 3://antimat
    clearList(3);
    tft.drawRect(28,68,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM      118"));
    tft.setCursor(277, 155),tft.print(F("WG 20.00"));
    tft.setCursor(337, 155),tft.print(F("VAL 5596"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F(".50MG (8/100)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/antimatterielrifle.bmp",220,30);
    currentListItem=3;
    break;
    
    case 4://big mtn
    clearList(3);
    tft.drawRect(28,88,160,35,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM        0"));
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL   --"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("--"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/bigmountaintransportalponder.bmp",230,30);
    currentListItem=4;
    break;
    
    case 5://christian runnals
    clearList(3);
    tft.drawRect(28,125,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM      854"));
    tft.setCursor(277, 155),tft.print(F("WG 30.00"));
    tft.setCursor(337, 155),tft.print(F("VAL 3511"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("Mini Nuke (1/5)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/fatman.bmp",250,30);
    currentListItem=5;
    break;

    case 6://that gun
    clearList(3);
    tft.drawRect(28,145,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM       32"));
    tft.setCursor(277, 155),tft.print(F("WG  5.00"));
    tft.setCursor(337, 155),tft.print(F("VAL 1534"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("5.56 (5/500)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/thatgun.bmp",225,30);
    currentListItem=6;
    break;
    
    case 7://this machine
    clearList(3);
    tft.drawRect(28,165,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DAM       59"));
    tft.setCursor(277, 155),tft.print(F("WG  9.50"));
    tft.setCursor(337, 155),tft.print(F("VAL 2374"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F(".308 (8/65)"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/weapons/thismachine.bmp",250,30);
    currentListItem=7;
    break;
    
  }
}

void drawApparel()
{
  if(selection>3)
    selection=3;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F("Armored Vault Suit"));
  tft.setCursor(30,60),tft.print(F("Radiation Suit"));

  tft.drawLine(190,150,270,150,PIP_COLOR),tft.drawLine(270,150,270,164, PIP_COLOR);
  tft.drawLine(275,150, 330, 150, PIP_COLOR),tft.drawLine(330,150,330,164,PIP_COLOR);//upper three boxes under picture
  tft.drawLine(335,150,390,150,PIP_COLOR),tft.drawLine(390,150, 390, 164, PIP_COLOR);
  
  tft.drawLine(190,170,270,170,PIP_COLOR),tft.drawLine(270,170,270,184,PIP_COLOR);
  tft.drawLine(275,170,390,170,PIP_COLOR),tft.drawLine(390, 170, 390, 184, PIP_COLOR);//lower two boxes below picture
  
  switch(selection)
  {
    case 1:
    clearList(3);
    tft.drawRect(28,28,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DT         8"));
    tft.setCursor(277, 155),tft.print(F("WG 15.00"));
    tft.setCursor(337, 155),tft.print(F("VAL   70"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("Light"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/apparel/armoredvault13jumpsuit.bmp",250,30);
    currentListItem=1;
    break;

    case 2:
    clearList(3);
    tft.drawRect(28,48,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(192, 155),tft.print(F("DT         4"));
    tft.setCursor(277, 155),tft.print(F("WG  5.00"));
    tft.setCursor(337, 155),tft.print(F("VAL   33"));
    tft.setCursor(192, 175),tft.print(F("CND")),tft.fillRect(235,175,30,7,PIP_COLOR);
    tft.setCursor(277, 175),tft.print(F("Light  Rad.Res.+30"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/apparel/radiationsuit.bmp",250,30);
    currentListItem=2;
    break;
  }
}

void drawAid()
{
  if(selection<1)
    selection=1;
    
  if(selection>7)
    selection=7;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F("Antivenom (6)"));
  tft.setCursor(30,60),tft.print(F("Buffout (1)"));
  tft.setCursor(30,80),tft.print(F("Doctor's Bag(2)"));
  tft.setCursor(30,100),tft.print(F("RadAway(4)"));
  tft.setCursor(30,120),tft.print(F("Rad-x (5)"));
  tft.setCursor(30,140),tft.print(F("Stimpak (8)"));

  tft.drawLine(275,150, 330, 150, PIP_COLOR),tft.drawLine(330,150,330,164,PIP_COLOR);//upper 2 boxes under picture
  tft.drawLine(335,150,390,150,PIP_COLOR),tft.drawLine(390,150, 390, 164, PIP_COLOR);
  
  tft.drawLine(190,170,390,170,PIP_COLOR),tft.drawLine(390, 170, 390, 184, PIP_COLOR);//lower box below picture
  
  switch(selection)
  {
    case 1://antivenom
    clearList(4);
    tft.drawRect(28,28,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  150"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS       Cure Animal Poison"));
    
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/antivenom.bmp",280,40);
    currentListItem=1;
    break;
    
    case 2://buffout
    clearList(4);
    tft.drawRect(28,48,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL   20"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS       HP+6-, END+3, STR+2"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/buffout.bmp",250,45);
    currentListItem=2;
    break;
    
    case 3://doctors bag
    clearList(4);
    tft.drawRect(28,68,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG  2.00"));
    tft.setCursor(337, 155),tft.print(F("VAL  110"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS    Restore All Body Parts"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/doctorsbag.bmp",270,45);
    currentListItem=3;
    break;
    
    case 4://radaway
    clearList(4);
    tft.drawRect(28,88,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL   80"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS                Rads -174"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/radaway.bmp",270,30);
    currentListItem=4;
    break;
    
    case 5://radx
    clearList(4);
    tft.drawRect(28,108,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  100"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS              Rad.Res.+72"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/radx.bmp",260,50);
    currentListItem=5;
    break;

    case 6://stimpak
    clearList(4);
    tft.drawRect(28,128,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  600"));
    tft.setCursor(192, 175),tft.print(F("EFFECTS                   HP +104"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/aid/stimpack.bmp",235,45);
    currentListItem=6;
    break;
    
    
  }
}

void drawItemsMisc()
{
  if(selection<1)
    selection=1;
    
  if(selection>9)
    selection=9;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F("Bobby Pin (50)"));
  tft.setCursor(30,60),tft.print(F("Deathclaw Hand (3)"));
  tft.setCursor(30,80),tft.print(F("Dinky the T-Rex"));
  tft.setCursor(30,100),tft.print(F("Mark of Caesar"));
  tft.setCursor(30,120),tft.print(F("NCR Dogtag (5)"));
  tft.setCursor(30,140),tft.print(F("Sierra Madre Chip"));
  tft.setCursor(30,160),tft.print(F("The Platinum Chip"));
  tft.setCursor(30,180),tft.print(F("Vault 13 Canteen"));

  tft.drawLine(275,150, 330, 150, PIP_COLOR),tft.drawLine(330,150,330,164,PIP_COLOR);//upper 2 boxes under picture
  tft.drawLine(335,150,390,150,PIP_COLOR),tft.drawLine(390,150, 390, 164, PIP_COLOR);
  
  switch(selection)
  {
    case 1://bobbypin
    clearList(4);
    tft.drawRect(28,28,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL   50"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/bobbypin.bmp",250,60);
    currentListItem=1;
    break;
    
    case 2://dclaw hand
    clearList(4);
    tft.drawRect(28,48,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG  3.00"));
    tft.setCursor(337, 155),tft.print(F("VAL  225"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/deathclawhand.bmp",260,30);
    currentListItem=2;
    break;
    
    case 3://dinky
    clearList(4);
    tft.drawRect(28,68,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG   .10"));
    tft.setCursor(337, 155),tft.print(F("VAL    1"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/dinkythetrex.bmp",270,35);
    currentListItem=3;
    break;
    
    case 4://mark of caesar
    clearList(4);
    tft.drawRect(28,88,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL    1"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/markofcaesar.bmp",250,30);
    currentListItem=4;
    break;
    
    case 5://ncr
    clearList(4);
    tft.drawRect(28,108,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL    5"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/ncrdogtag.bmp",250,35);
    currentListItem=5;
    break;

    case 6://sierra madre chip
    clearList(4);
    tft.drawRect(28,128,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL   --"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/sierramadrechip.bmp",245,35);
    currentListItem=6;
    break;

    case 7://plat chip
    clearList(4);
    tft.drawRect(28,148,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL    3"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/theplatinumchip.bmp",250,30);
    currentListItem=7;
    break;

    case 8://canteen
    clearList(4);
    tft.drawRect(28,168,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG  1.00"));
    tft.setCursor(337, 155),tft.print(F("VAL    2"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/misc/vault13canteen.bmp",250,40);
    currentListItem=8;
    break;
  }
}

void drawAmmo()
{
  if(selection<1)
    selection=1;
    
  if(selection>7)
    selection=7;
  
  tft.setFont(&Monospaced_bold_12);
  tft.setCursor(30,40),tft.print(F(".308 Rounds (65)"));
  tft.setCursor(30,60),tft.print(F(".45 Auto (500)"));
  tft.setCursor(30,80),tft.print(F(".50 MG (100)"));
  tft.setCursor(30,100),tft.print(F("5.56mm ROund (500)"));
  tft.setCursor(30,120),tft.print(F("Microfusion Cell(75)"));
  tft.setCursor(30,140),tft.print(F("Mini Nuke (5)"));

  tft.drawLine(275,150, 330, 150, PIP_COLOR),tft.drawLine(330,150,330,164,PIP_COLOR);//upper 2 boxes under picture
  tft.drawLine(335,150,390,150,PIP_COLOR),tft.drawLine(390,150, 390, 164, PIP_COLOR);

  switch(selection)
  {
    case 1://308
    clearList(4);
    tft.drawRect(28,28,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  455"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/308round.bmp",250,30);
    currentListItem=1;
    break;
    
    case 2://45
    clearList(4);
    tft.drawRect(28,48,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  225"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/45auto.bmp",250,60);
    currentListItem=2;
    break;
    
    case 3://50
    clearList(4);
    tft.drawRect(28,68,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL 6000"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/50bmg.bmp",260,35);
    currentListItem=3;
    break;
    
    case 4://556
    clearList(4);
    tft.drawRect(28,88,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL 2000"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/5.56round.bmp",260,30);
    currentListItem=4;
    break;
    
    case 5://mf
    clearList(4);
    tft.drawRect(28,108,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  375"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/microfusioncell.bmp",250,35);
    currentListItem=5;
    break;

    case 6://nuke
    clearList(4);
    tft.drawRect(28,128,160,15,PIP_COLOR);
    tft.setFont();
    tft.setCursor(277, 155),tft.print(F("WG    --"));
    tft.setCursor(337, 155),tft.print(F("VAL  500"));
    tft.fillRect(190,28, 200,118,BLACK);
    showBMP("/items/ammo/mininuke.bmp",260,45);
    currentListItem=6;
    break;
  }
}

void drawMap()
{
  tft.setFont(&Monospaced_bold_20),tft.setCursor(30,50);
  tft.fillRect(30,30,350,175,BLACK);
  tft.print(F("Loading Map"));
  for(int i=0;i<5;i++)
    tft.print(F(".")),delay(1500);
  tft.fillRect(30,30,350,175,BLACK);
  tft.setCursor(30,50);
  tft.print(F("Connection Error."));
  tft.setFont(&Monospaced_bold_12),tft.setCursor(30,70);
  tft.print(F("No connection to Vault-Tec!"));
  tft.setCursor(30,82);
  tft.print(F("Last connection: 999+ day(s) ago."));
  currentListItem=selection;
}

void drawQuests()
{
  
}

void drawDataMisc()
{
  
}

void drawRadio()
{
  digitalWrite(buttonPause,HIGH);
  //currentListItem=selection;
  tft.drawLine(250,170,390,170,PIP_COLOR),tft.drawLine(390,170,390,30,PIP_COLOR);
  for(int i=250;i<390;(i+=5))
  {
    tft.drawLine(i,170,i,165,PIP_COLOR);
  }
  for(int i=30;i<170;(i+=5))
  {
    tft.drawLine(385,i,390,i,PIP_COLOR);
  }
  
  tft.setCursor(30,50),tft.setFont(&Monospaced_bold_12),tft.print("Radio New Vegas");
  tft.drawRect(20,38,160,15,PIP_COLOR);
  
  if(isPlaying==true)
  {
    tft.fillRect(23,43,5,5,PIP_COLOR);
  }

  if(isPlaying==true)
  {
    if (digitalRead(buttonPause) == LOW)
    {
      pause();
      isPlaying = false;
      tft.fillRect(250,30,135,135,BLACK);
      tft.fillRect(23,43,5,5,BLACK);
    }
  }
  else if(isPlaying==false)
  {
    if (digitalRead(buttonPause) == LOW)
    {
      play();
      isPlaying = true;
    }
  }

  if(isPlaying==true)
  {
    tft.fillRect(250,30,135,135,BLACK);
      for(double x=250;x<385;(x+=.3))
        {
          double y=50*(sin(.25*x))+100;
          tft.drawPixel(x,y,PIP_COLOR);
        }
  }
  //Serial.println(isPlaying);
}

void clearList(int x)
{
  //x decides which list will be cleared
  switch(x)
  {
    case 1:
    tft.drawLine(28,44,180,44, BLACK),tft.drawLine(28,44,28,28, BLACK);
    tft.drawLine(28,28,180,28, BLACK),tft.drawLine(180,44,180,28, BLACK);
    tft.drawLine(28,64, 180,64, BLACK),tft.drawLine(28,64,28,48, BLACK);
    tft.drawLine(28,48, 180,48, BLACK),tft.drawLine(180,64,180,48, BLACK);   
    tft.drawLine(28,84, 180,84, BLACK),tft.drawLine(28,84,28,68, BLACK);
    tft.drawLine(28,68, 180,68, BLACK),tft.drawLine(180,84,180,68, BLACK);    
    tft.drawLine(28,104, 180,104, BLACK),tft.drawLine(28,104,28,88, BLACK);
    tft.drawLine(28,88, 180,88, BLACK),tft.drawLine(180,104,180,88, BLACK);    
    tft.drawLine(28,124, 180,124, BLACK),tft.drawLine(28,124,28,108, BLACK);
    tft.drawLine(28,108, 180,108, BLACK),tft.drawLine(180,124,180,108, BLACK);    
    tft.drawLine(28,144, 180,144, BLACK),tft.drawLine(28,144,28,128, BLACK);
    tft.drawLine(28,128, 180,128, BLACK),tft.drawLine(180,144,180,128, BLACK);    
    tft.drawLine(28,164, 180,164, BLACK),tft.drawLine(28,164,28,148, BLACK);
    tft.drawLine(28,148, 180,148, BLACK),tft.drawLine(180,164,180,148, BLACK);
    tft.drawLine(28,184, 180,184, BLACK),tft.drawLine(28,184,28,168, BLACK);
    tft.drawLine(28,168, 180,168, BLACK),tft.drawLine(180,184,180,168, BLACK);
    tft.drawLine(28,204, 180,204, BLACK),tft.drawLine(28,204,28,148, BLACK);
    tft.drawLine(28,188, 180,188, BLACK),tft.drawLine(180,204,180,148, BLACK);
    break;
    case 2://for general in stats
    tft.drawRect(18,37,165,18,BLACK);
    tft.drawRect(18,57,165,18,BLACK);
    tft.drawRect(18,77,165,18,BLACK);
    tft.drawRect(18,97,165,18,BLACK);
    tft.drawRect(18,117,165,18,BLACK);
    break;
    case 3://weapons in items
    tft.drawRect(28,28,160,15,BLACK);
    tft.drawRect(28,48,160,15,BLACK);
    tft.drawRect(28,68,160,15,BLACK);
    tft.drawRect(28,88,160,35,BLACK);
    tft.drawRect(28,125,160,15,BLACK);
    tft.drawRect(28,145,160,15,BLACK);
    tft.drawRect(28,165,160,15,BLACK);
    
    tft.fillRect(190,153,79,13,BLACK);//dam (these comments are for weapons)
    tft.fillRect(190,173,79,13,BLACK);//cnd
    tft.fillRect(275,153,54,13,BLACK);//wg
    tft.fillRect(275,173,110,13,BLACK);//ammo
    tft.fillRect(335,153,54,13,BLACK);//val
    break;
    
    case 4://aid, misc, and ammo in items
    tft.drawRect(28,28,160,15,BLACK);
    tft.drawRect(28,48,160,15,BLACK);
    tft.drawRect(28,68,160,15,BLACK);
    tft.drawRect(28,88,160,15,BLACK);
    tft.drawRect(28,108,160,15,BLACK);
    tft.drawRect(28,128,160,15,BLACK);
    tft.drawRect(28,148,160,15,BLACK);
    tft.drawRect(28,168,160,15,BLACK);
    
    tft.fillRect(190,173,199,13,BLACK);//cnd
    tft.fillRect(275,153,54,13,BLACK);//wg
    tft.fillRect(335,153,54,13,BLACK);//val
    break;

    
  }
}

int RE_Check()
{
  
  RE_aState=digitalRead(RE_OutPutA);
  if(RE_aState!=RE_aLastState)
  {
    if(digitalRead(RE_OutPutB)!=RE_aState)
    {
      RE_Counter--; //originally ++
    }  
    else 
    {
      RE_Counter++;//originally --
    }
    //Serial.print("position: ");
    //Serial.println(RE_Counter);
  }
  RE_aLastState=RE_aState;

  if(RE_Counter<=0)
    RE_Counter=1;

  return RE_Counter;
}

int checkRes(int Switch_Pin)
{
  
  int raw=0;
  
  int Vin= 5;
  float Vout= 0;
  float KnownRes= 1000; //should be 1000
  float R2= 0;
  float buffer= 0;
  
  raw= analogRead(Switch_Pin);
  if(raw) 
  {
  buffer= raw * Vin;
  Vout= (buffer)/1024.0;
  buffer= (Vin/Vout) -1;
  R2= KnownRes * buffer;

//these resistance values are pretty wide windows 
//but if they werent, testing on a breadboard would be impossible
  if(R2<40&&R2>1)
    Rot_Pos=1;
  else if(R2>170&&R2<255)
    Rot_Pos=2;
  else if(R2>270&&R2<350)
    Rot_Pos=3;
  else if(R2>80&&R2<130)
    Rot_Pos=4;
  else if(R2>700&&R2<1060)
    Rot_Pos=5;  
  else
    Rot_Pos=Rot_Pos;
    
  //Serial.print("R2: ");
  if(Switch_Pin==Secondary_ResPin)
    //Serial.println(R2);
  //Serial.print("Rot_Pos: ");
  //Serial.println(Rot_Pos);
  return Rot_Pos;
  }
}

void gaugeBatt()
{
  float voltage;
  float BatPercent=0;
  voltage=(calcVolts()*100);
  BatPercent=map(voltage,200,420,180,0);
  myservo.write(BatPercent);
}

 float calcVolts()
{
  int sensorValue = analogRead(batAnalogPin); //read the A0 pin value
  float voltage = sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage.
  //delay(100);//this delay will break the rotary encoder for some reason
  return voltage;
}
