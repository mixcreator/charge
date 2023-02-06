#include "charging.h"
#include "meter_value.h"
#include "utils.h"



static void send_data(const char *ctx, char *data){
    printf("%s", data);
    fflush(stdout);
    Log(LOG_INFO, "[%s]SEND: %s", ctx, data);
}

// TODO:
void get_meter_value(char *value);

MeterValueThread::MeterValueThread():MThread("MeterValue"){}

MeterValueThread::~MeterValueThread(){}

int MeterValueThread::run(){
    Log(LOG_INFO, "[%s]Started", context);
    while(!IsTerminating()){
        char inData[256]={0};
        Log(LOG_INFO, "[%s]Reading  ...", context);
        fgets(inData, 10, stdin);
        // TODO: check request type
        //Log(LOG_INFO, "[METERVALUE] GET: %s", inData);
        get_meter_value(inData);
        send_data(context, inData);
    }
    //Log(LOG_INFO, "[%s]Finishing  ...", context);
    return 0;
}