// Example by Tom Igoe

import processing.serial.*;

Serial myPort;    // The serial port
String apiMsg;  // Input string from serial port
int lf = 10;      // ASCII linefeed

// radius around the point
int regionTolerance = 50;

int screenX = 600;
int screenY = 450;

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
  regions.add(new Region(1001,screenX + 555,row_baseline,0,0,"Toggle FAV",0));
  regions.add(new Region(1002,screenX + 555,row_baseline*3,0,0,"ON/OFF",0));
  regions.add(new Region(1003,screenX + 555,row_baseline*5,0,0,"?",0));
}

void setup() {
  // first we have to initialize default regions
  // default 3/4 grid
  /*
  int columns = 4;
  int rows = 3;
  int rx = screenX / columns;
  int ry = screenY / rows;
  int light = 0;
  int py = ry/2;
  
  regions = new Region[columns*rows];
  lights = new Light[columns*rows];
  
  for(int r = rows; r > 0; r--) {
    int px = rx/2;
    for(int c = columns; c > 0; c--) {
      print("set light number: ");
      println(light);
      
      if(light == 4)
        regions[light] = new Region(light, px, py, pwm[2], 0);
      else if(light == 7)
        regions[light] = new Region(light, px, py, pwm[15], 0);
      else
        regions[light] = new Region(px, py, 0, 0);
      lights[light] = new Light(light, 0, 0);

      px += rx;

      if(light < 12)
        light++;
    }

    py += ry;
  }
  */
  
  // init regions
  // regions = new ArrayList<Region>();
  
  initLocalButtons();
  
  // we add 150 for local buttons
  size(screenX + 150,screenY);
  
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
  stroke(125);
  line(screenX, 0, screenX, screenY);
  
  // check if any light on
  regions.get(1).intensity = 0;
  for(int l = 0; l < lights.size(); l++) {
    if(lights.get(l).intensity > 0) {
      regions.get(1).intensity = 4095;
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
          continue;
        }
        
        // on/off
        if(region.id == 1002) {
          println("GUI : on/off : "+ region.intensity);
          if(region.intensity > 0)
            myPort.write("allOff::;");
          else
            myPort.write("allOn::;");
          continue;
        }
        
        // ?
        if(region.id == 1003) {
          String label1 = "Do You feel lucky";
          String label2 = "Well do ya, PUNK !?";
          String label3 = "I DO !!";
          println();
          println();
          if(region.label.equals("?") == true) {
            region.label = label1;
            region.selected = true;
          }
          else if(region.label.equals(label1) == true) {
            region.label = label2;
            region.selected = true;
          }
          else if(region.label.equals(label2) == true) {
            region.label = label3;
            region.selected = true;
            luckyTime = millis();
          }
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
      int intensity = int(map(_mouseX,0,screenX,pwm[0],4095));
      //int level = int(map(_mouseX,0,screenX,0,15));
      //intensity = pwm[level];
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
    int regionFav = int(split(data[6], ":")[1]);
    int regionHasFav = int(split(data[7], ":")[1]);
  
    if(regions != null) {
      
        boolean regionNotFound = true;
      
        for( int r = 0; r < regions.size(); r++ ) {
          Region region = regions.get(r);
          if( regionID == region.id ) {
            //print("GUI : REGION UPDATE : ");
            //println(region.id);
            region.intensity = regionIntensity;
            region.hasFav = regionHasFav;
            return;
          }
       }
       
       if(regionNotFound) {
          // new region detected
          println("GUI : REGION ADD : " + regionID);
          regions.add(new Region(regionID,regionX,regionY,regionIntensity,regionFav,"",regionHasFav));
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
  int fav;
  int hasFav;
  boolean selected = false;
  String label = "";
  
  // constructor
  Region(int idUpdate, int xUpdate, int yUpdate, int intensityUpdate, int favUpdate) {
    id = idUpdate;
    x = int(map(xUpdate, 0, 1024, 0, screenX));
    y = int(map(yUpdate, 0, 1024, 0, screenY));
    intensity = intensityUpdate;
    fav = favUpdate;
  }
  
  Region(int idUpdate, int xUpdate, int yUpdate, int intensityUpdate, int favUpdate, String labelUpdate, int hasFavUpdate) {
    id = idUpdate;
    x = int(map(xUpdate, 0, 1024, 0, screenX));
    y = int(map(yUpdate, 0, 1024, 0, screenY));
    intensity = intensityUpdate;
    fav = favUpdate;
    label = labelUpdate;
    hasFav = hasFavUpdate;
    println(label);
  }
  
  // Custom method for drawing the object
   void draw() {
     strokeWeight(2);
     stroke(75);
     fill(25,25,25);
     
     if(hasFav > 0) {
       label = "FAV";
     }
     
     float brightness = map(intensity, 0, 4095, 100, 255);

     if( intensity > 0 ) {
       fill(brightness,brightness,0);
       stroke(200,200,200);
     }
     
     if(selected) {
       fill(brightness,0,brightness);
     }
     
     ellipse(x, y, regionTolerance*2, regionTolerance*2);
          
     if(label.length() > 0) {
       if(intensity > 2000)
         fill(0,0,0);
       else
         fill(200,200,200);
         
       textAlign(CENTER);
       textSize(15);
       text(label, x, y+4);
     }

   }
}
