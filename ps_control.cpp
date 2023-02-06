#include <sys/types.h>                                                    
#include <sys/stat.h>                                                     
#include <fcntl.h>                                                        
#include <termios.h>                                                      
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>                                             
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include <linux/serial.h>
#include <stdint.h>

#include "charging.h"
#include "ps_control.h"
#include "modbus.h"




#define SLAVE_DEVICE_ADDR    0x9
#define SET_CURRENT_ADDR     0
#define SET_VOLTAGE_ADDR     0x2
#define SET_CONTROL_ADDR     0x4

#define GET_CURRENT_ADDR     0xE
#define GET_VOLTAGE_ADDR     0xC

//uint16_t tab_reg[64];
modbus_t *ctx;


union u32float
{
    uint32_t v;
    float f;
};

uint32_t float2hex(float f){
    u32float u;
    u.f = f;
    return u.v;
}

float hex2float(uint32_t hex){
    u32float u;
    u.v = hex;
    return u.f;
}

MMutex psStateMtx;
static PS_STATE ps_state = PS_IDLE;
static bool is_on = false;
PS_STATE GetPSState(){
    Lock lock(psStateMtx);
    return ps_state;
}

void SetPSState(PS_STATE val){
    Lock lock(psStateMtx);
    ps_state = val;
}


MMutex realOutVoltageMtx;
static float realOutputVoltage = 1.0;
static float prevRealOutputVoltage = 1.0;
static void setRealOutputVoltage(float val){
    Lock lock(realOutVoltageMtx);
    realOutputVoltage = val;
}

float getRealOutputVoltage(){
    Lock lock(realOutVoltageMtx);
    return realOutputVoltage;
}

MMutex realOutCurrentMtx;
static float realOutputCurrent = 0.0;
static float prevRealOutputCurrent = 0.0;
static void setRealOutputCurrent(float val){
    Lock lock(realOutCurrentMtx);
    realOutputCurrent = val;
}

float getRealOutputCurrent(){
    Lock lock(realOutCurrentMtx);
    return realOutputCurrent;
}

#ifndef _METERVALUE 
static char devData[256];
#endif

static RWMutex mvMtx;
void get_meter_value(char *value){
    RLock lock(mvMtx);
    // TODO:
    // ...
    strcpy(value, devData);
}

void set_meter_value(char *value){
    RWLock lock(mvMtx);
    // TODO:
    // ...
    strcpy(devData, value);
}


RS485ControlThread::RS485ControlThread():MThread("PS Control"){}

RS485ControlThread::~RS485ControlThread(){}

volatile bool is_rs485_terminate=false;

int open_serial(const char *port);

int RS485ControlThread::run(){
    Log(LOG_INFO, "[%s]Started", context);
#ifndef _METERVALUE 
    int i=0;
    int fd = open("/dev/urandom", O_RDONLY);
    if(fd<0){
        Log(LOG_ERR, "[%s]error open /dev/udev: %s", context, strerror(errno)); 
        // TOTO: correct return
        Log(LOG_INFO, "[%s]Finishing ...", context);
        return 2;
    }    
#endif

    //while(!is_rs485_terminate){
    while(!IsTerminating()){
#ifdef _METERVALUE 
    // read real data from meter
    // send modbus command 
#else
    // read data from /dev/udev
        char udata[4];
        ssize_t rd = ::read(fd, udata, sizeof(udata));
        if(rd<0){
            printf("Error read from device: %s", strerror(errno));
            //Log(LOG_ERR, "[%s]Error read from device: %s", context, strerror(errno));
            continue;
        }
        char sRes[128] = {0};
        for(int j=0;j<rd;j++){
            char tRes[4]={0};
            sprintf(tRes, "%02X", udata[j]&0xFF);
            strcat(sRes, tRes);
        }
        struct timeval tv;
        gettimeofday(&tv, NULL);
        char ssRes[256];
        sprintf(ssRes, "%d\n", i++);

        // SET
        set_meter_value(ssRes);
#endif        
        switch(GetPSState()){
            case PS_WORKING:
            {
                Log(LOG_DEBUG, "[%s] WORKING", context);
                float vol = (float)GetOutputVoltage();
                float current = (float)GetOutputCurrent();
#if 0
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                r = set_voltage(vol);
                (r > 0)? Log(LOG_INFO, "RS485    set_voltage OK: %f", vol):Log(LOG_INFO, "RS485    set_voltage ERROR:%s %d\n", strerror(errno), errno);
                usleep(10000);

                // get voltage
                float voltage;
                if(get_actual_voltage(&voltage)){
                    Log(LOG_INFO, "RS485    get_actual_voltage error: set_prev voltage: %f", prevRealOutputVoltage);
                    setRealOutputVoltage(prevRealOutputVoltage);
                }else{
                    Log(LOG_INFO, "RS485    get_actual_voltage OK: %f", voltage);
                    prevRealOutputVoltage = voltage;
                    setRealOutputVoltage(voltage);
                }
                usleep(10000);
                // get current
                float curr;
                if(get_actual_current(&curr)){
                    Log(LOG_INFO, "RS485    get_actual_current error: set_prev current: %f", prevRealOutputCurrent);
                    setRealOutputCurrent(prevRealOutputCurrent);
                }else{
                    Log(LOG_INFO, "RS485    get_actual_current OK: %f", curr);
                    prevRealOutputCurrent = curr;
                    setRealOutputCurrent(curr);
                }
#endif
                //SetCurrentVoltage(current, vol);
                SetParams(current, vol);
                // get voltage
                float voltage;
                if(get_actual_voltage(&voltage)){
                    Log(LOG_INFO, "[%s]get_actual_voltage error: set_prev voltage: %f", context, prevRealOutputVoltage);
                    setRealOutputVoltage(prevRealOutputVoltage);
                }else{
                    //Log(LOG_INFO, "RS485    get_actual_voltage OK: %f", voltage);
                    prevRealOutputVoltage = voltage;
                    setRealOutputVoltage(voltage);
                }
                usleep(10000);
                // get current
                float curr;
                if(get_actual_current(&curr)){
                    Log(LOG_INFO, "[%s]get_actual_current error: set_prev current: %f", context, prevRealOutputCurrent);
                    setRealOutputCurrent(prevRealOutputCurrent);
                }else{
                    //Log(LOG_INFO, "RS485    get_actual_current OK: %f", curr);
                    prevRealOutputCurrent = curr;
                    setRealOutputCurrent(curr);
                }
                usleep(10000);
                break;
            }
            case PS_IDLE:
            {
                //Log(LOG_INFO, "RS485    PS IDLE ");
                usleep(100000);
                break;
            }
            case PS_OFF:
            {
                if(is_on){
                    is_on = false;
                    Log(LOG_INFO, "[%s]TRYING ...PS OFF", context);

                    SwitchSystem(false);
                    
                    modbus_close(ctx);
                    modbus_free(ctx);
                    SetPSState(PS_IDLE);

                    // TODO: return error code
                    Log(LOG_INFO, "[%s]Finishing ...", context);
                    return 1;
                }
                break;
            }
            case PS_ON:
            {
                if(!is_on){
                    is_on = true;
                Log(LOG_INFO, "[%s]PS ON", context);
                if(open_serial("/dev/ttyUSB0")){
                    Log(LOG_ERR, "[%s]Error open LIBMODBUS", context);
                    SetPSState(PS_IDLE);
                    break;
                }
                Log(LOG_INFO, "[%s]TRYING ...PS ON", context);
                float vol = (float)GetOutputVoltage();
                float current = (float)GetOutputCurrent();
#if 0
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                (set_current_limit(current))?Log(LOG_INFO, "RS485    set_current OK: %f", current):Log(LOG_INFO, "RS485    set_current ERROR:%s %d\n", modbus_strerror(errno), errno);
                usleep(10000);
                r = set_voltage(vol);
                (r > 0)? Log(LOG_INFO, "RS485    set_voltage OK: %f", vol):Log(LOG_INFO, "RS485    set_voltage ERROR:%s %d\n", strerror(errno), errno);
                usleep(10000);
#endif
                //SetCurrentVoltage(current, vol);
                SetParams(current, vol);
                SwitchSystem(true);
                SetPSState(PS_WORKING);
                }
                break;
            }
        }
    }
	//Log(LOG_INFO, "[%s]switch OFF: status=%d", context, is_on);
	/*if(is_on){
	    Log(LOG_INFO, "RS485    TRYING ...PS OFF");
        (switch_system(true))?Log(LOG_INFO, "RS485    OK\n"):Log(LOG_INFO, "RS485    ERROR: %s\n", modbus_strerror(errno));
        is_on = false;
    }*/
	//Log(LOG_INFO, "[%s]TRYING ...PS OFF. Finishing ...", context);
        if(is_on){
            SwitchSystem(false);
	//sleep(1);

    /* Close the connection */
	    modbus_close(ctx);
	    modbus_free(ctx);
        }
    Log(LOG_INFO, "[%s]Finishing ...", context);
    // TODO: return error code
    return 0;

}

int open_serial(const char *port){
	ctx = modbus_new_rtu(port, 115200, 'N', 8, 1);
	if (ctx == NULL) {
		return -1;
	}

	//("Create modbus rtu  OK\n");
    //("Press any key!!\n");
    //getchar();

	//modbus_set_debug(ctx, TRUE);
    //modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

    // Save original timeout
	//modbus_get_response_timeout(ctx, &old_response_timeout); ???
	
    // Define new
	//response_timeout.tv_sec = 20;
	//response_timeout.tv_usec = 20;
	//modbus_set_response_timeout(ctx, (struct timeval *)(&response_timeout));
		
	modbus_set_slave(ctx, SLAVE_DEVICE_ADDR);

	if (modbus_connect(ctx) == -1) {
		Log(LOG_INFO, "Connection failed: %s\n", modbus_strerror(errno));
		
		modbus_free(ctx);
		return -1;
	}
	//("Connect OK\n");
    //("Press any key!!\n");
    //getchar();
    return 0;
}


int RS485ControlThread::set_current_limit(float current){
    uint16_t data[2];
    uint32_t df = float2hex(current);
    data[0] = df & 0xFFFF;
    data[1] = (df >> 16) & 0xFFFF;
    modbus_set_slave(ctx, 1);
    return modbus_write_registers(ctx, SET_CURRENT_ADDR, 2, data, 0);
}

int RS485ControlThread::set_voltage(float voltage){
    uint16_t data[2];
    uint32_t df = float2hex(voltage);
    data[0] = df & 0xFFFF;
    data[1] = (df >> 16) & 0xFFFF;
    modbus_set_slave(ctx, 1);
    return modbus_write_registers(ctx, SET_VOLTAGE_ADDR, 2, data, 0);
}

int RS485ControlThread::switch_system(bool is_on){
    uint16_t data[1];
    data[0] = is_on?1:0;
    modbus_set_slave(ctx, 0x9);
    return modbus_write_registers(ctx, SET_CONTROL_ADDR, 1, data, 1);
}

int RS485ControlThread::get_actual_voltage(float *val){
    uint16_t data[16]; // ??
    modbus_set_slave(ctx, 0x9);
    int rc = modbus_read_registers(ctx, GET_VOLTAGE_ADDR, 1, data);
    if (rc < 0) {
        Log(LOG_ERR, "[%s]get_actual_voltage %s", context, modbus_strerror(errno));
        return -1;
    }
    *val = data[0];
    return 0;
}

int RS485ControlThread::get_actual_current(float *val){
    uint16_t data[16]; // ??
    modbus_set_slave(ctx, 0x9);
    int rc = modbus_read_registers(ctx, GET_CURRENT_ADDR, 2, data);
    if (rc < 0) {
        Log(LOG_ERR, "[%s]get_actual_current %s", context, modbus_strerror(errno));
        return -1;
    }
    *val = hex2float(data[1]<<16| data[0]);
    return 0;
}

#define MAX_ATTEMPTS_CURRENT_SET 4
#define MAX_ATTEMPTS_PS_SWITCH   5

//void RS485ControlThread::SetCurrentVoltage(float curr, float volt){
void RS485ControlThread::SetParams(float curr, float volt){
    for(int i=0;i<MAX_ATTEMPTS_CURRENT_SET;i++){
       if(set_current_limit(curr) <=0){
           Log(LOG_INFO, "[%s]set_current ERROR:%s %d\n", context, modbus_strerror(errno), errno);
       }
       usleep(10000);
    }
    if(set_voltage(volt)<=0)
        Log(LOG_INFO, "[%s]set_voltage ERROR:%s %d\n", context, strerror(errno), errno);
    usleep(10000);
}


void RS485ControlThread::SwitchSystem(bool is_on){
    for(int i=0;i<MAX_ATTEMPTS_PS_SWITCH; i++){
    if(switch_system(is_on)<=0)
        Log(LOG_INFO, "[%s]set %d ERROR: %s\n", context, is_on, modbus_strerror(errno));
    usleep(10000);
    }
}

