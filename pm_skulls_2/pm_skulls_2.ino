

// START SHARED

// Custom return struct, update as needed
struct Element {
  int r;
  int g;
  int b;
  int x;
  int y;
};

class Controller {

  private:
    int overallCount = 0;

  public:
    Controller() {}

    void next() { overallCount++; }

    void getElements(Element * elements, int size) {
      
    }

};
// END SHARED


int FillArray(int * ar, int size)
{
  int t = 0;
  for (int i=0; i< size; i++)
  {
     ar[i] = t*t++;
  }
}

Controller controller = Controller();

void setup() {

}

void loop() {

  controller.next();

  int size = 48;
  Element elements[size];

  controller.getElements(elements, size);

}

void act(Element * elements, int size) {

  
}
