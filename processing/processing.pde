// Example by Tom Igoe

import processing.serial.*;

Serial myPort;    // The serial port
String apiMsg;  // Input string from serial port
int lf = 10;      // ASCII linefeed

// radius around the point
int regionTolerance = 75;

int screenX = 800;
int screenY = 600;

int sidebarWidth = 200;

// when we receive information about active setup mode disable refreshing
boolean switchInSetupMode = false;

// 16 steps of light intensity
int[] pwm = {
  45, 65, 90, 130,
  200, 330, 450, 600,
  800, 1100, 1450, 1700,
  2200, 2800, 3400, 4095,
};

ArrayList<Region> regions = new ArrayList<Region>();
ArrayList<Light> lights = new ArrayList<Light>();

  int clickX;
  int clickY;
  int clicks;
  float clicktime = 0.0;
  float tapDelay = 200.0;
  
  float refreshedTime = 0.0;
  int minRefreshTime = 100;
  
long luckyTime = 0;
long luckyUpdate = 0;


void initLocalButtons() {
  // all local buttons should have ID > 1000
  int row_baseline = (1024/3)/2;
  int col_baseline = (1024/4)/2;
  regions.add(new Region(1001,screenX + sidebarWidth + 155,row_baseline,0,0,"\u2665",0,100,100));
  regions.add(new Region(1002,screenX + sidebarWidth + 155,row_baseline*3,0,0,"\u263C",0,100,100));
  regions.add(new Region(1003,screenX + sidebarWidth + 155,row_baseline*5,0,0,"?",0, 100,100));
  regions.add(new Region(1004,screenX + sidebarWidth + 155,(row_baseline*6)-10,0,0,"reset",0, 80,40));
}

void setup() {
  // init regions
  // regions = new ArrayList<Region>();
  PFont fontFamily;
  fontFamily = loadFont("DejaVuSans-Bold-48.vlw");
  textFont(fontFamily);
  
  initLocalButtons();
  
  // we add for local buttons
  size(screenX + sidebarWidth,screenY);
  
  // List all the available serial ports:
  println(Serial.list());
  
  // Open whatever port is the one you're using.
  myPort = new Serial(this, Serial.list()[0], 115200);
  myPort.bufferUntil(lf);
  
  // get information about regions
  myPort.write("requestUpdateRegions::;");
  delay(50);
  myPort.write("requestUpdateLights::;");
  delay(50);
} 
 
void draw() {
  background(0);
  stroke(75);
  line(screenX, 0, screenX, screenY);
  
  regions.get(0).selected = false;
  // check if any light on on region.id = 1002
  regions.get(1).intensity = 0;
  regions.get(1).selected = false;
  regions.get(1).label = "\u263C";
  for(int l = 0; l < lights.size(); l++) {
    if(lights.get(l).intensity > 0) {
      //regions.get(1).intensity = 4095;
      regions.get(1).selected = true;
      regions.get(1).label = "\u2600";
    }
  }
  
  // lucky
  lucky();
  
  if(clicks > 0 && millis() - clicktime > tapDelay && clickX > 0 && clickY > 0){
    // do something with clicks
    
    int r = getClosestRegionID(clickX,clickY);
    if(r > -1) {
      for( int rr = 0; rr < regions.size(); rr++ ) {
        Region region = regions.get(rr);
        // get the right region
        if(region.id != r) continue;
        
        // fav
        if(region.id == 1001) {
          myPort.write("favToggle::;");
          region.selected = true;
          region.draw();
          continue;
        }
        
        // on/off
        if(region.id == 1002) {
          println("GUI : on/off : "+ region.intensity);
          if(region.intensity > 0 | region.selected)
            myPort.write("allOff::;");
          else
            myPort.write("allOn::;");
          continue;
        }
        
        // ?
        if(region.id == 1003) {
          String label1 = "Do you feel lucky\nwell do ya, punk !?";
          String label2 = "I DO !!";
          println();
          println();
          if(region.label.equals("?") == true) {
            region.label = label1;
            region.selected = true;
          }
          else if(region.label.equals(label1) == true) {
            region.label = label2;
            region.selected = true;
            luckyTime = millis();
          }
          continue;
        }
        
        // reset
        if(region.id == 1004) {
          myPort.write("reset::;");
          continue;
        }
        
        println("GUI : region clicked : "+ region.id);
        
        // don't toggle when region in selection mode
        if(clicks == 1 && !region.selected) {
          myPort.write("toggleRegion:"+ region.id +":;");
        }
        
        if(clicks == 2) {
          if(region.selected) region.selected = false;
          else region.selected = true;
        }
      } // for
    } else {
      // if clicked ouside region
      if(clicks == 2) {
        for( int rr = 0; rr < regions.size(); rr++ ) {
          regions.get(rr).selected = false;
        }
      }
    }
    
    //println("GUI : clicks : "+ clicks);
    
    // reset
    mouseReset();
  }

  // refresh regions
  if( regions.size() > 0 ) {
    for( int r = 0; r < regions.size(); r++ ) {
      regions.get(r).draw();
    }
  }
  
  delay(10);
}

void lucky() { 
  // 15 second animation
  int numLights = 12; //lights.size();
  int numRegions = 0;
  
  for( int r = 0; r < regions.size(); r++) {
    if(regions.get(r).id < 1000)
      numRegions++;
  }
 
  int[] luckyPwm = pwm;
  luckyPwm = (int[]) append(luckyPwm, 0);
  
  // update every 100 millis for 15 seconds
  if( luckyTime > 0 && millis() - luckyTime < 15000 && millis() - luckyUpdate > 50) {
    
    // animate all lights
    /**
    for( int l = 0; l < numLights; l++) {
      int intensity = luckyPwm[ int(random(luckyPwm.length)) ];
      myPort.write("dim:"+l+":"+intensity+";");
      delay(5);
    }
    /**/
    
    //animate Regions
    /**/
    for( int l = 0; l < numRegions; l++) {
      int intensity = luckyPwm[ int(random(luckyPwm.length)) ];
      myPort.write("dimRegion:"+l+":"+intensity+";");
      delay(5);
    }
    /**/
    
    luckyUpdate = millis();
  }
  
  if( luckyTime > 0 && millis() - luckyTime > 5000) {
    //myPort.write("allOff::;");
    luckyTime = 0;
    regions.get(2).label = "?";
    regions.get(2).selected = false;
  } 
}

int getClosestRegionID(int x, int y) {
  
  boolean sloppy = false;
  
  println("GUI : "+ x +" : "+ y);
  
  int closestRegion = -1;
  long closestDistance = 2147483647;
  
  if( regions.size() > 0 ) {
      for( int r = 0; r < regions.size(); r++ ) {
        
        Region region = regions.get(r);
        long dx = region.x - x;
        long dy = region.y - y;
        long distance = (dx * dx) + (dy + dy);
        

        if(
          (region.x != 0) & (region.y != 0)
          & (region.x > (x - regionTolerance))
          & (region.x < (x + regionTolerance))
          & (region.y > (y - regionTolerance))
          & (region.y < (y + regionTolerance))
          ) {
    
          // FIXME: why do I need something in here for r to be returned properly
          // without this, r=1023
          delay(0);
          println("GUI : distance : "+ distance);
    
          // if exact region is found were're done
          return region.id;
        }
        else if( sloppy & (region.x != 0) & (region.y != 0) & (distance < closestDistance) ) {
          // get closestRegion
          closestDistance = distance;
          closestRegion = region.id;
        }
      }
  }
  
  return closestRegion;
  
}

void mousePressed(MouseEvent e)
{

}

void mouseReset(){
  clicks = 0;
  clicktime = millis();
  clickX = -1;
  clickY = -1;
}

void mouseClicked(MouseEvent e) {
  
  clickX = e.getX();
  clickY = e.getY();
 
  if (e.getCount()==1 && LEFT == mouseButton){
    println("<single click>");
    clicks = 1;
  }
  
    
  if (e.getCount()==2 && LEFT == mouseButton){
    println("<double click>");
    clicks = 2;
  }
  
  if (e.getCount()==3 && LEFT == mouseButton){
    println("<tripple click>");
    clicks = 3;
  }
  
  if (e.getCount()==1 && RIGHT == mouseButton){
    println("<right button>");
  }
  
  clicktime = millis();
}

void mouseDragged(MouseEvent e) {
  // update dim on switch
  if(millis() - refreshedTime < minRefreshTime) return;
  
  boolean foundSelected = false;
  
  for( int r = 0; r < regions.size(); r++ ) {
    Region region = regions.get(r);
    if(region.selected) {
      int _mouseX = int(constrain(mouseX,0,screenX));
      // wolfram bulb needs more power than led, this mapping returns higher average intensity
      // than pwm[level] which is better for leds, wolfram buld truns of when intensity < 750
      int intensity = int(map(_mouseX,0,screenX,pwm[0],4095));
      //int level = int(map(_mouseX,0,screenX,0,15));
      //int intensity = pwm[level];
      myPort.write("dimRegion:"+region.id+":"+intensity+";");
     
      refreshedTime = millis();
      foundSelected = true;
      delay(20);
    }
  }
}

void serialEvent(Serial p) {
  apiMsg = p.readString().replace("\n","");
  
  String[] data = split(apiMsg, "|");
  
  print("GUI : apiMsg : ");
  println(apiMsg);
  
  if(data[0].equals("API") == false) return;
  if(data[1].equals("setupStart") == true) switchInSetupMode = true;
  if(data[1].equals("setupDone") == true) switchInSetupMode = false;
  
  if(data[1].equals("updateRegions") == true | data[1].equals("setupStart") == true) {
    // clear regions
    println("GUI : UPDATE REGIONS");
    if( regions.size() > 0 ) {
      regions.clear();
      lights.clear();
      initLocalButtons();
    }
    return;
  }
  
  if(data[1].equals("regionInfo") == true) {
    //println("GUI : REGION INFO");
    int regionID = int(split(data[2], ":")[1]);
    int regionX = int(split(data[3], ":")[1]);
    int regionY = int(split(data[4], ":")[1]);
    int regionIntensity = int(split(data[5], ":")[1]);
    int regionNumRegionLights = int(split(data[6], ":")[1]);
    int regionFav = int(split(data[7], ":")[1]);
    int regionHasFav = int(split(data[8], ":")[1]);
    String regionIrCode = trim(split(data[9], ":")[1]);
    
    String regionLabel = str(regionID+1) + " ("+ str(regionNumRegionLights) +")";
    if(regionHasFav > 0) regionLabel = regionLabel + "\n\u2665";
    if(regionIrCode.length() > 2) {
      regionLabel = regionLabel +"\n"+ regionIrCode;
    }
  println(regionIrCode);
    if(regions != null) {
      
        boolean regionNotFound = true;
      
        for( int r = 0; r < regions.size(); r++ ) {
          Region region = regions.get(r);
          if( regionID == region.id ) {
            //print("GUI : REGION UPDATE : ");
            //println(region.id);
            region.intensity = regionIntensity;
            region.hasFav = regionHasFav;
            region.numRegionLights = regionNumRegionLights;
            region.label = regionLabel;
            
            region.x = int(map(regionX, 0, 1024, 0, screenX));
            region.y = int(map(regionY, 0, 1024, 0, screenY));
            return;
          }
       }
       
       if(regionNotFound) {
          // new region detected
          println("GUI : REGION ADD : " + regionID);
          regions.add(new Region(regionID,regionX,regionY,regionIntensity,regionNumRegionLights,regionFav,regionLabel,regionHasFav));
       }
       
       for( int r = 0; r < regions.size(); r++ ) {
         //println("GUI : REGIONS : "+ r +" : "+ regions.get(r).id);
       }
    }
  }
  
    if(data[1].equals("lightInfo") == true) {
    //println("GUI : LIGHT INFO");
    int lightID = int(split(data[2], ":")[1]);
    int lightRegion = int(split(data[3], ":")[1]);
    int lightIntensity = int(split(data[4], ":")[1]);
    int lightFav = int(split(data[5], ":")[1]);
  
    if(lights != null) {
        for( int l = 0; l < lights.size(); l++ ) {
          Light light = lights.get(l);
          if( lightID == light.id ) {
            //print("GUI : LIGHT UPDATE : ");
            //println(light.id);
            light.intensity = lightIntensity;
            return;
          }
       }
       
       // new light detected
       println("GUI : LIGHT ADD : " + lightID);
       lights.add(new Light(lightID,lightRegion,lightIntensity,lightFav));
       
       for( int l = 0; l < lights.size(); l++ ) {
         //println("GUI : LIGHTS : "+ l +" : "+ lights.get(l).id);
       }
    }
  }
}

class Click{
  int x;
  int y;
  int clicks;
  float clicktime = 0.0;
  float tapDelay = 200.0;
  boolean clicked = false;
  
  Click(int xUpdate, int yUpdate, int clicksUpdate) {
    y = yUpdate;
    x = xUpdate;
    clicks = clicksUpdate;
  }
}

class Light{
  int id;
  int region;
  int intensity;
  int fav;
  
  // constructor
  Light(int idUpdate, int regionUpdate, int intensityUpdate, int favUpdate) {
    id = idUpdate;
    region = regionUpdate;
    intensity = intensityUpdate;
    fav = favUpdate;
  }
}

class Region {
  int id;
  int x;
  int y;
  int intensity;
  int numRegionLights;
  int fav;
  int hasFav;
  boolean selected = false;
  String label = "";
  int w = regionTolerance*2;
  int h = regionTolerance*2;
  
  // constructor
  Region(int idUpdate, int xUpdate, int yUpdate, int intensityUpdate, int favUpdate) {
    id = idUpdate;
    x = int(map(xUpdate, 0, 1024, 0, screenX));
    y = int(map(yUpdate, 0, 1024, 0, screenY));
    intensity = intensityUpdate;
    fav = favUpdate;
  }
  
  Region(int idUpdate, int xUpdate, int yUpdate, int intensityUpdate, int favUpdate, String labelUpdate, int hasFavUpdate, int wUpdate, int hUpdate) {
    id = idUpdate;
    x = int(map(xUpdate, 0, 1024, 0, screenX));
    y = int(map(yUpdate, 0, 1024, 0, screenY));
    intensity = intensityUpdate;
    fav = favUpdate;
    label = labelUpdate;
    hasFav = hasFavUpdate;
    w = wUpdate;
    h = hUpdate;
  }
  
  Region(int idUpdate, int xUpdate, int yUpdate, int intensityUpdate, int numRegionLightsUpdate, int favUpdate, String labelUpdate, int hasFavUpdate) {
    id = idUpdate;
    x = int(map(xUpdate, 0, 1024, 0, screenX));
    y = int(map(yUpdate, 0, 1024, 0, screenY));
    intensity = intensityUpdate;
    fav = favUpdate;
    label = labelUpdate;
    hasFav = hasFavUpdate;
    numRegionLights = numRegionLightsUpdate;
  }
  
  // Custom method for drawing the object
   void draw() {
     strokeWeight(1);
     stroke(20);
     fill(15,15,15);
     //rect(x - (w/2), y-(h/2), w, h, 10);
     
     strokeWeight(2);
     stroke(100);
     fill(45,45,45);
     
     float brightness = map(intensity, 0, 4095, 100, 255);

     if( intensity > 0 ) {
       fill(brightness,brightness,0);
       stroke(220,220,220);
     }
     
     if(selected) {
       fill(brightness,0,brightness);
     }
     
     rect(x - (w/2), y-(h/2), w, h, w/3);
     //ellipse(x, y, w, h);
          
     if(label.length() > 0) {
       if(intensity > 1500)
         fill(0,0,0);
       else
         fill(200,200,200);
         
       textAlign(CENTER);
       if(id==1001 || id==1002) {
         textSize(46);
         text(label, x, y+14);
       }
       else {
         textSize(18);
         text(label, x, y+4);
       }
     }

   }
}
