// START SHARED
#include <math.h> 

struct Loc {
  float x;
  float y;
};

struct Color {
  int r;
  int g;
  int b;
};

struct Node {
  int skull;
  int eye;
  int light;
  Color color;
};



class LightController {

private:
  Node lights[48];
  int led_eye_count = 12;
  int skull_mm = 300;
  int eye_mm = 80;
  int light_mm = 20;
  float pi = 3.14159;
  Loc source = {-50, 0};
  int delta = 10;
  char* pattern = "fade"; // fade, line, blink, fire

  // Blink
  int blinkVal = 200;
  int blinkDelta = -1;
  int eyeVal = 2;

  // Fire
  int fireI = 0;

public:
  LightController () {
    int i = 0;
    for (int skull=0; skull<2; skull++) {
      for (int eye=0; eye<2; eye++) {
        for (int light=0; light<this->led_eye_count; light++) {
          Color color = {skull * 255, eye * 255, light * 20};
          Node node = {skull, eye, light, color};
          lights[i] = node;
          i++;
        }
      }
    }
  }
  
  Node getLight(int i) {
     return lights[i];
  }
  
  Node* getLights() {
    return lights;
  }

  Loc getLightLoc(int skull, int eye, int light) {
    float x = skull * skull_mm + eye * eye_mm + cos(float(light) / led_eye_count * 2 * pi) * light_mm;
    float y = sin(float(light) / led_eye_count * 2 * pi) * light_mm;
    return {x, y};
  }

  float getDistLoc(Loc source, Loc target) {
    return pow(pow(source.x - target.x, 2) + pow(source.y - target.y, 2), 0.5);
  }

  void next() {
    
    // Fade pattern
    if (this->pattern=="fade") {
      int i = 0;
      for (int skull=0; skull<2; skull++) {
        for (int eye=0; eye<2; eye++) {
          for (int light=0; light<led_eye_count; light++) {
            Loc target = this->getLightLoc(skull, eye, light);
            float dist = getDistLoc(this->source, target);
            Color color = {int(dist / 600 * 255), 255 - int(dist / 600 * 255), 0};
            Node node = {skull, eye, light, color};
            this->lights[i] = node;
            i++;
          }
        }
      }
      float x = this->source.x + this->delta;
      float y = this->source.y;
      if (x>400) { delta = -10; }
      if (x<0) { delta = 10; }
      this->source = {x, y};
    }

    // Line moving sideways
    if (this->pattern=="line") {
      delta = 5;
      int i = 0;
      for (int skull=0; skull<2; skull++) {
        for (int eye=0; eye<2; eye++) {
          for (int light=0; light<led_eye_count; light++) {
            Loc target = this->getLightLoc(skull, eye, light);
            // float dist = getDistLoc(this->source, target);
            float dist = abs(source.x - target.x);
            Color color = {255 - int(dist) * 15, 255 - int(dist) * 15, 255 - int(dist) * 15};
            Node node = {skull, eye, light, color};
            this->lights[i] = node;
            i++;
          }
        }
      }
      float x = this->source.x + this->delta;
      float y = this->source.y;
      if (x>800) { x = -50; }
//      if (x<0) { delta = 10; }
      this->source = {x, y};
    }

    // Blink
    if (this->pattern=="blink") {
      int i = 0;
      for (int skull=0; skull<2; skull++) {
        for (int eye=0; eye<2; eye++) {
          for (int light=0; light<led_eye_count; light++) {
            Loc target = this->getLightLoc(skull, eye, light);
            Color color;
            color = {255, 255, 255};
            if (skull*2 + eye==eyeVal) {
              if (abs(target.y) > blinkVal) {
                color = {0, 0, 0};
              }
            }
            Node node = {skull, eye, light, color};
            this->lights[i] = node;
            i++;
          }
        }
      }
      
      blinkVal += blinkDelta;
      if (blinkVal<-10) { blinkDelta = 1; }
      if (blinkVal>200) {
        blinkDelta = -1; 
        eyeVal = eyeVal + 1;
        if (eyeVal > 3) {
          eyeVal = 0;
        }
      }
    }

    // Fire
    if (this->pattern=="fire") {
      int i = 0;
      float val = iToVal(fireI) * 20;
      for (int skull=0; skull<2; skull++) {
        for (int eye=0; eye<2; eye++) {
          for (int light=0; light<led_eye_count; light++) {
            Loc target = this->getLightLoc(skull, eye, light);
            int strandNr = skull * 4 + eye * 2;
            if (cos(float(light) / led_eye_count * 2 * pi) > 0) {
              strandNr +=1;         
            }
            switch(strandNr) {
              case 0:
                val = iToVal(fireI) * 20;
                break;
              case 1:
                val = iToVal(fireI + 3113, 200, 750, 810) * 20;
                break;
              case 2:
                val = iToVal(fireI + 5335, 580, 930, 150) * 20;
                break;
              case 3:
                val = iToVal(fireI + 4383, 240, 290, 150) * 20;
                break;
              case 4:
                val = iToVal(fireI + 1453, 230, 703, 203) * 20;
                break;
              case 5:
                val = iToVal(fireI + 2385, 230, 2240, 260) * 20;
                break;
              case 6:
                val = iToVal(fireI + 9482, 590, 200, 520) * 20;
                break;
              case 7:
                val = iToVal(fireI + 2952, 290, 740, 920) * 20;
                break;
            }
            
            Color color;
            if (target.y > val) {
              float y = target.y * -1 + 20;
              float demon = val * -1 + 20;
              float ratio = y / demon;
              color = {255, int(ratio * 255), 0};
            } else {
              color = {0, 0, 0};
            }
            Node node = {skull, eye, light, color};
            this->lights[i] = node;
            i++;
          }
        }
      }
      fireI = fireI + 5;
    }

  }

  float iToVal(int i, int a=50, int b=70, int c=110) {
    return sin(i/a) * 1/1 + sin(i/b) * 1/2 + sin(i/c) * 1/3;
  }
    

//       'a': i_to_val(i),
//       'b': i_to_val(i+31134, 20, 75, 81),
//       'c': i_to_val(i+5335, 58, 93, 15),
//       'd': i_to_val(i+4383, 24, 29, 15),
//       'e': i_to_val(i+1453, 23, 73, 23),
//       'f': i_to_val(i+2385, 23, 24, 26),
//       'g': i_to_val(i+9482, 59, 24, 52),
//       'h': i_to_val(i+2952, 29, 74, 92),
};
// END SHARED
