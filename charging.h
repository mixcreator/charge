#ifndef __CHARGING_H__
#define __CHARGING_H__ 20200205

#ifndef __cplusplus
#error C++ is required
#endif


#include <stdint.h>

#include <wiringPi.h>

#include "messages.h"
#include "settings.h"
#include "utils.h"


class CANReader: public MThread{
public:
    CANReader();
    //explicit CANReader(Settings &settings);
    virtual ~CANReader();

    virtual int run();

    static void print_vech_fault_flag(const faultflag_bitset &vhff);
    static void print_vech_status_flag(const statusflag_bitset &vhf);
 
private:
    int process(const struct can_frame& frame,  bool is_termination = false);
    bool catch_initial_frames(const struct can_frame& frame);
    int can_open();
    int can_close();
    int can_read(void *data);
private:
    int sock;
    bool received100;
    bool received101;
    bool received102;
    //Settings &set;
};

class CANWriter : public MThread{
public:

    explicit CANWriter(Settings &settings);

    virtual ~CANWriter();

    virtual int run();
private:
    int fill(Frame108 *f108, Frame109 *fr109, bool is_termination = false);
    int can_open();
    int can_close();
    int can_write(const void *data);

private:

    int sock;
    uint16_t voltage_ps_set;
    int cnt_sm_volt;
    uint8_t current_veh_set;
    int cnt_sm_curr;
    Settings &set;
};

class ChargingCtl: public MThread{
public:
    explicit ChargingCtl(Settings &settings);

    virtual ~ChargingCtl();

    virtual int run();
private:
    Settings &set;
};

class TerminationTh: public MThread{
public:
    TerminationTh();
    virtual ~TerminationTh();
    virtual int run();
};

void TerminateCharging();

bool IsTerminating();

uint16_t GetOutputVoltage();

uint8_t GetOutputCurrent();


#endif
