#include "sms.h"

bool SMS::beginSms()
{
    return begin(Serial) &&
           sendCheckReply(F("AT+CMGF=1"), ok_reply) &&       //text mode
           sendCheckReply(F("AT+CNMI=1,2,0,0,0"), ok_reply); // send SMS directly on serial
}

void SMS::onSmsReceived(SmsReceivedHandler handler)
{
    _handler = handler;
}

bool SMS::lineAvailable()
{
    static uint16_t replyidx = 0;

    while (mySerial->available())
    {
        char c = mySerial->read();
        if (c == '\r')
            continue;
        if (c == 0xA)
        {
            if (replyidx == 0) // the first 0x0A is ignored
                continue;
            replybuffer[replyidx] = 0;
            replyidx = 0;
            return true;
        }
        replybuffer[replyidx] = c;
        replyidx++;
        if (replyidx == 250)
        {
            replyidx = 0;
        }
    }

    return false;
}

void SMS::loop()
{
    char phonenum[20];
    if (lineAvailable() && parseReply(F("+CMT: \""), phonenum, '"'))
    {
        readline();
        if (_handler)
        {
            _handler(phonenum, replybuffer);
        }
    }
}