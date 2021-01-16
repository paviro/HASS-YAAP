#include <Arduino.h>
#include <EspMQTTClient.h>
#include <Dictionary.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <screen.h>
#include <config.h>

// general
String currentAlarmState;
String code;

// Dictionaries
Dictionary & internalStates = * (new Dictionary(6));
Dictionary & strings = * (new Dictionary(6));

EspMQTTClient client(WiFi_SSID,WiFi_PW,MQTT_SERVER_IP,MQTT_USER,MQTT_PW,MQTT_CLIENT_NAME);

byte keypadRowPins[KEYPAD_ROWS] = {KEYPAD_ROW_PINS};
byte keypadColPins[KEYPAD_COLS] = {KEYPAD_COL_PINS};
Keypad customKeypad = Keypad(makeKeymap(keypadMap), keypadRowPins, keypadColPins, KEYPAD_ROWS, KEYPAD_COLS);

LiquidCrystal_I2C lcd(SCREEN_ADDRESS, SCREEN_WIDTH, SCREEN_HEIGHT);

Screen screen(DEFAULT_TIMEOUT*1000, BACKLIGHT_TIMEOUT*1000, WELCOME_MESSAGE_TIMEOUT*1000, TEXT_SCROLL_SPEED_DELAY);

void setup() {
  lcd.init();
  strings.jload(languageJSON);

  // Needed for comparision between new requested state and current state
  internalStates("away", "armed_away");
  internalStates("home", "armed_home");
  internalStates("night", "armed_night");
  internalStates("disarm", "disarmed");
}

void Screen::turnOnLCD() {
  backlightMillis = millis();
  lcd.backlight();
}

void Screen::drawCenterText(int row, String text) {
  int offset = (SCREEN_WIDTH - text.length()) / 2;
  lcd.setCursor(offset, row);
  lcd.print(text);
}

String Screen::repeatStringByDisplayWidth(String stringToRepeat) {
  String string;
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    string += stringToRepeat;
  }
  return string;
}

void Screen::scrollText() {
  if (scrollingStarted == true and millis() - lastScrolled >= (unsigned)scrollDelay) {
    String scrollPadding = repeatStringByDisplayWidth(" ");
    String text = scrollPadding + peopleArrived + scrollPadding;
    String displaySubstring = text.substring(scrollStringStart, scrollStringStart + SCREEN_WIDTH);

    lcd.setCursor(0, 2);
    lcd.print(displaySubstring);

    scrollStringStart += 1;
    lastScrolled = millis();
    if (scrollStringStart > ((signed)text.length() - SCREEN_WIDTH)) {
      scrollStringStart = 0;
    }

  } else if (screen.displayed != "peopleArrived") {
    scrollingStarted = false;
  }
}

void Screen::drawConnecting() {
  const char *loadingAnimation[11] = { ".", "..", "...", "....", ".....", "......", ".......", "........", ".........", "..........", "          " };
  if (screen.displayed != "connectingToServer") {
    screen.displayed = "connectingToServer";
    animationStep = 0;
    lcd.clear();
    drawCenterText(1, strings["connecting"]);
    turnOnLCD();
  } else if (millis() - lastAnimated >= 100) {
    lastAnimated = millis();
    drawCenterText(2, loadingAnimation[animationStep]);
    if (animationStep == 10) {
      animationStep = 0;
    } else {
      animationStep += 1;
    }
  }
}

void Screen::drawAlarmSystemState() {
  screen.displayed = "alarmSystemState";
  lcd.clear();
  drawCenterText(1, strings["systemTitle"]);
  drawCenterText(2, strings[currentAlarmState]);
  turnOnLCD();
}

void Screen::drawArmingError(int error) {
  screen.displayed = "armingError";
  autoTimeoutMillis = millis();
  lcd.clear();
  switch (error) {
    case (0):
      drawCenterText(1, strings["invalidCodeHeader"]);
      drawCenterText(2, strings["invalidCodeMessage"]);
      break;
    case (1):
      drawCenterText(0, strings["currentStateHeader"]);
      drawCenterText(1, repeatStringByDisplayWidth("-"));
      drawCenterText(2, strings["currentStateMessage"]);
      drawCenterText(3, strings[currentAlarmState]);
      break;
  }
}

void Screen::drawLightsOff(int state) {
  screen.displayed = "lightsTurnedOff";
  autoTimeoutMillis = millis();
  lcd.clear();
  drawCenterText(1, strings["lightsTurnOffHeader"]);
  switch (state) {
    case (0):
      drawCenterText(2, strings["lightsTurnOffRequested"]);
      break;
    case (1):
      drawCenterText(2, strings["lightsTurnOffSuccess"]);
      break;
  }
  turnOnLCD();
}

void Screen::drawPersonArrived(const String & person) {
  personArrivedMillis = millis();

  if (screen.displayed != "peopleArrived") {
    peopleArrived = "";
    lcd.clear();
    drawCenterText(1, strings["peopleArrivedWelcome"]);
    turnOnLCD();
    screen.displayed = "peopleArrived";
  }

  if (peopleArrived.length() == 0) {
    peopleArrived = person;
  } else {
    peopleArrived += " & " + person;
  }

  if (peopleArrived.length() > SCREEN_WIDTH) {
    if (scrollingStarted == false) {
      scrollingStarted = true;
      scrollStringStart = 0;
    }
  } else {
    drawCenterText(2, peopleArrived);
  }
}

void Screen::drawCodeInputMode(const char & input) {
  screen.displayed = "codeInput";
  autoTimeoutMillis = millis();
  if (input == '*') {
    lcd.clear();
    drawCenterText(0, strings["enterCode"]);
    drawCenterText(1, repeatStringByDisplayWidth("-"));
    cursorPosRow = 0;
    cursorPosCol = 2;
    lcd.setCursor(cursorPosRow, cursorPosCol);
    code = "";
  } else {

    if (code.length() < SCREEN_WIDTH * 2 - 1) {
      lcd.setCursor(cursorPosRow, cursorPosCol);
      if (SHOW_PIN == true) {
        lcd.print(input);
      } else {
        lcd.print("*");
      }
      cursorPosRow += 1;
      code += input;
    }

    if (code.length() == SCREEN_WIDTH) {
      cursorPosRow = 0;
      cursorPosCol = 3;
      lcd.setCursor(cursorPosRow, cursorPosCol);
    }
  }
  turnOnLCD();
}

void Screen::checkTimeout() {
  unsigned long currentMillis = millis();

  // Screen
  if (currentMillis - backlightMillis >= (unsigned)backlightTimeout and screen.displayed == "alarmSystemState") {
    lcd.noBacklight();
  }

  // autoTimeout
  if (screen.displayed == "codeInput" or screen.displayed == "lightsTurnedOff" or screen.displayed == "armingError") {
    if (currentMillis - screen.autoTimeoutMillis >= (unsigned)autoTimeout) {
      if (screen.displayed == "codeInput") {
        code = "";
      }
      screen.drawAlarmSystemState();
    }
  }

  // person arrived timeout
  if (screen.displayed == "peopleArrived") {
    if (currentMillis - screen.personArrivedMillis >= (unsigned)peopleArrivedScreenTimeout) {
      screen.drawAlarmSystemState();
    }
  }
}

void armSystem(const String & type, const String & pin) {
  if (currentAlarmState == internalStates[type]) {
    screen.drawArmingError(1);
  } else {
    if (type == "disarm") {
      client.publish("alarmpanel/disarm", pin);
    } else {
      client.publish("alarmpanel/arm/" + type, pin);
    }
  }
  code = "";
}

void turnLightsOff() {
  client.publish("alarmpanel/lights", "off");
  screen.drawLightsOff(0);
}


void parseInput(const char & input) {
  if (input == 'A' and screen.displayed == "codeInput") {
    armSystem("away", code);
  } else if (input == 'B' and screen.displayed == "codeInput") {
    armSystem("night", code);
  } else if (input == 'C' and screen.displayed == "codeInput") {
    armSystem("home", code);
  } else if (input == 'D' and screen.displayed == "codeInput") {
    armSystem("disarm", code);
  } else if (input == '#' and screen.displayed == "alarmSystemState") {
    turnLightsOff();
  } else if (input == '#' and screen.displayed != "alarmSystemState") {
    screen.drawAlarmSystemState();
  } else if (input == '*' or screen.displayed == "codeInput") {
    screen.drawCodeInputMode(input);
  } else {
    screen.turnOnLCD();
  }
}

void readKeypad() {
  char inputKey = customKeypad.getKey();
  if (inputKey) {
    parseInput(inputKey);
  }
}

void onLCD(const String & message) {
  if (message == "on") {
    screen.turnOnLCD();
  } else if (message == "off") {
    lcd.noBacklight();
  }
}

void onArmingError(const String & message) {
  if (message == "invalidCode") {
    screen.drawArmingError(0);
  }
}

void onPersonArrived(const String & message) {
  screen.drawPersonArrived(message);
}

void onLightsCallback(const String & message) {
  if (message == "success") {
    screen.drawLightsOff(1);
  }
}

void onAlarmSystemStateChange(const String & message) {
  currentAlarmState = message;
  if ( !(screen.displayed == "codeInput" and (currentAlarmState == "triggered" or currentAlarmState == "pending")) )
  {
    screen.drawAlarmSystemState();
  }
}

void onConnectionEstablished() {
  client.subscribe("person/arrived", onPersonArrived);
  client.subscribe("alarmpanel/state", onAlarmSystemStateChange);
  client.subscribe("alarmpanel/arm/error", onArmingError);
  client.subscribe("alarmpanel/lights/callback", onLightsCallback);
  client.subscribe("alarmpanel/lcd", onLCD);

  client.publish("alarmpanel/get", "state");
}

void loop() {
  client.loop();

  if (client.isConnected() == true) {
    readKeypad();
    screen.checkTimeout();
    screen.scrollText();
  } else {
    screen.drawConnecting();
  }

  if (screen.displayed == "codeInput") {
    lcd.blink();
  } else {
    lcd.noBlink();
  }

}