#ifndef _SMS_H_
#define _SMS_H_

#include <Adafruit_FONA.h>

typedef void (*SmsReceivedHandler)(char *from, char *text);

class SMS : public Adafruit_FONA
{

public:
    SMS() : Adafruit_FONA(9) {}

    bool beginSms();
    void loop();
    void onSmsReceived(SmsReceivedHandler handler);

private:
    SmsReceivedHandler _handler;
    bool lineAvailable();
};

#endif
