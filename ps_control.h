#ifndef __PS_CONTROL_H__
#define __PS_CONTROL_H__    20180717

#include "utils.h"

enum PS_STATE{
    PS_OFF=0,
    PS_ON,
    PS_WORKING,
    PS_IDLE
};

class RS485ControlThread: public MThread{
public:
    //RS485ControlThread(int fd);
    RS485ControlThread();
    
    ~RS485ControlThread();
    
    virtual int run();

private:
    //int set_ping();
    int switch_system(bool is_on);
    int set_current_limit(float val);
    int set_voltage(float val);
    int get_actual_voltage(float *val);
    int get_actual_current(float *val); 
    
    //void SetCurrentVoltage(float curr, float volt);
    void SetParams(float curr, float volt);
    void SwitchSystem(bool is_on);
    
//private:
//    int fdserial;

};

//PS_STATE GetPSState();

void SetPSState(PS_STATE val);

//int open_serial(const char *port);

//int set_interface_attribs(int fd);

//void setRealOutputVoltage(float val);
float getRealOutputVoltage();

//void setRealOutputCurrent(float val);
float getRealOutputCurrent();

extern volatile bool is_rs485_terminate;

#endif
