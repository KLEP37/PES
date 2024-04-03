#include "Button.h"

Button::Button(byte pin) {
this->pin = pin;
lastReading = LOW;
init();
}

void Button::init() {
	pinMode(pin, INPUT_PULLUP);
  update();
}

void Button::update() {
	byte newReading = digitalRead(pin);
  if (newReading != lastReading) {
        //lastDebounceTime = millis();
        state = newReading;
      }
  /*if (millis() - lastDebounceTime > debounceDelay) {
           state = newReading;
        } */
  lastReading = newReading;
}

byte Button::getState() {
        update();
        return state;
}

bool Button::isPressed() {
    if (getState() == LOW) {
      delay(300);
      return true;
    }
    else return false;
}
