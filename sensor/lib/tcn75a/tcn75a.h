#ifndef _TCN75A_H_
#define _TCN75A_H_

#include <stdint.h>

class Tcn75a
{
public:
    void begin();
    int16_t read();
};

#endif
