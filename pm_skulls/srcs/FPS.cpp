
#include <chrono>
#include <iostream>

class FPS 
{

private:
    int count = 0;
    float fps = 0.0;
    long lastUpdateTime = micros();
    long SECOND = 1000000;

public:

    // cpp
    long micros()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    float get_fps() {
      this->count += 1;
      long currentTime = micros();
      long deltaTime = currentTime - lastUpdateTime;

      // calculate fps when sufficient time has passed
      if (deltaTime > this->SECOND) {
        this->fps = float(count) / deltaTime * this->SECOND;
        this->count = 0.0;
        this->lastUpdateTime = currentTime;

        // cpp
        std::cout << "fps: " << this->fps << std::endl;

        //arduino
        // Serial.print("fps: ");
        // Serial.println(this->fps);
      } 
      return this->fps;
    }

};