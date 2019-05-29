#include "main.h"

int main() {
  LED = 1;
  sensor.init();
  initLineCommand();
  while(1) {
    wait(.5);
    LED = !LED;
  }
}