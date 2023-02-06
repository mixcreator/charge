#ifndef __METER_VALUE_H__
#define __METER_VALUE_H__ 20181108

#ifndef __cplusplus
#error C++ is required
#endif


#include "utils.h"

class MeterValueThread: public MThread{
public:
    //RS485ControlThread(int fd);
    MeterValueThread();
    
    virtual ~MeterValueThread();
    
    virtual int run();
};


#endif
