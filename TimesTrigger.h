typedef void (*TimesOutEvent)();

class TimesTrigger {
  public:
    TimesTrigger(int times, int resetMs);
    TimesOutEvent onTimesOut;
    void count();

  private:
    int _times;
    int _resetMillis;
    int _count;
    unsigned long _lastTime;
};
