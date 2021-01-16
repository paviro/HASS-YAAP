class Screen {
  private:
    // Functions
    String repeatStringByDisplayWidth(String stringToRepeat);
    void drawCenterText(int row, String text);

    // connection screen
    int animationStep;
    unsigned long lastAnimated;

    // Person arrived Screen
    int scrollStringStart;
    String peopleArrived;
    boolean scrollingStarted;

    // code mode stuff
    int cursorPosRow;
    int cursorPosCol;

    // Timeout and scroll millis
    unsigned long personArrivedMillis;
    unsigned long autoTimeoutMillis;
    unsigned long backlightMillis;
    unsigned long lastScrolled;

    // Timeout delays
    int autoTimeout;
    int backlightTimeout;
    int peopleArrivedScreenTimeout;
    int scrollDelay;

  public:
    Screen(int autoTimeout, int backlightTimeout, int peopleArrivedScreenTimeout, int scrollDelay) :
       autoTimeout(autoTimeout), backlightTimeout(backlightTimeout), peopleArrivedScreenTimeout(peopleArrivedScreenTimeout), scrollDelay(scrollDelay) {}

    void scrollText();
    void drawConnecting();
    void drawAlarmSystemState();
    void drawArmingError(int error);
    void drawLightsOff(int state);
    void drawPersonArrived(const String & person);
    void drawCodeInputMode(const char & input);
    void checkTimeout();
    void turnOnLCD();

    String displayed;
};