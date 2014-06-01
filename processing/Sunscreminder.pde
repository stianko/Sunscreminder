import controlP5.*;
import processing.serial.*;

ControlP5 cp5;
Serial myPort;
String textValue = "";
int i = 0;
String[] allNames = new String[10];
String[] allAges = new String[10];
String[] allSkintype = new String[10];

void setup() {
  println(Serial.list());
  myPort = new Serial(this, Serial.list()[0], 9600);
  size(400,400);
  PFont font = createFont("arial",20);
  
  cp5 = new ControlP5(this);
  
  cp5.addTextfield("Name")
     .setPosition(20,100)
     .setSize(200,40)
     .setFont(font)
     .setFocus(true)
     //.setColor(color(255,0,0))
     ;
                 
  cp5.addTextfield("Age")
     .setPosition(20,170)
     .setSize(200,40)
     .setFont(createFont("arial",20))
     .setAutoClear(false)
     ;
       
  cp5.addBang("SAVE PROFILE")
     .setPosition(240,100)
     .setSize(80,40)
     .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
     ;  
   
  cp5.addBang("SAVE ALL")
     .setPosition(240,260)
     .setSize(100,60)
     .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
     ;  
  
  cp5.addTextfield("Skintype (1-5)")
     .setPosition(20,240)
     .setSize(200,40)
     .setFont(createFont("arial",20))
     //.setAutoClear(false)
     ;
     
  textFont(font);
}

void draw() {
  background(0);
  fill(255);
  }

void controlEvent(ControlEvent theEvent) {
  if(theEvent.controller().name()=="SAVE PROFILE") {
    String name = cp5.get(Textfield.class,"Name").getText();
    String age = cp5.get(Textfield.class,"Age").getText();
    String skintype = cp5.get(Textfield.class,"Skintype (1-5)").getText();
    allNames[i] = name;
    allAges[i]= age;
    allSkintype[i] = skintype;
    i++;
    textSize(15);
    text("PROFILE SAVED", 240,220);
    cp5.get(Textfield.class, "Name").clear();
    cp5.get(Textfield.class, "Age").clear();
    cp5.get(Textfield.class, "Skintype (1-5)").clear();
  }
  if(theEvent.controller().name()=="SAVE ALL") {
    textSize(15);
    println("test");
    text("ALL SAVED", 240,220);
    println(allNames);
    println(allAges);
    println(allSkintype);
    myPort.write(i);
    for(int j = 0;j<allNames.length;j++){
        if(allNames[j] != null){
            myPort.write(allNames[j]);
            myPort.write(allAges[j]);
            myPort.write(allSkintype[j]);
            println(allNames[j]);
        }else{
          break;
        }
    }
    println(i);
  }

}


public void input(String theText) {
  // automatically receives results from controller input
  println("a textfield event for controller 'input' : "+theText);
}


