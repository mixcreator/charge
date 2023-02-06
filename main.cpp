#include <linux/can.h>
#include <linux/can/raw.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#include "charging.h"
#include "utils.h"
#include "messages.h"
#include "meter_value.h"
#include "ps_control.h"
#include "settings.h"
#include "inireader.h"


//bool is_running=true;

//FILE *log_stream = NULL;

static void sigterm(int) {
    //is_running = false;
    //fclose(fstream);
    //close(fdserial);
    //exit(1);
    TerminateCharging();
    is_rs485_terminate = true;
}

static void launcher(Settings &settings){
    MThread *th[] = {
        new CANReader,
        new CANWriter(settings),
        new ChargingCtl(settings),
        new TerminationTh,
        new RS485ControlThread,
        NULL
    };
    //Log(LOG_INFO, "SZ:%d", sizeof(th)/sizeof(MThread*));
    for(MThread **t=th;*t;(*t++)->start());//{
    
    MeterValueThread mvTh;
    mvTh.setcancelstate(PTHREAD_CANCEL_ENABLE);
    mvTh.start();

    for(MThread **t=th;*t;(*t++)->join());//{

    mvTh.cancel();
    Log(LOG_INFO, "[MeterValue]Finishing  ...");
    mvTh.join();

    for(MThread **t=th;*t;delete *t++);//{

}

//int main(int argc, char *argv[]){
int main(){
    openLog("CHAdeMO");
    
    //const char *can_interface;
    //const char *rs_dev;
    //int sockfd = -1;
    Log(LOG_INFO, "Start CHAdeMO");

    signal(SIGINT, sigterm); // Ctrl + C !!!


    ini_file reader("settings.ini");
    Settings settings(reader);
    Log(LOG_INFO, "max_available_voltage:    %d V", settings.max_available_voltage());
    Log(LOG_INFO, "available_output_current: %d A", settings.available_output_current());
    Log(LOG_INFO, "local_current_limit:      %d A", settings.local_current_limit());
    
    Log(LOG_INFO, "smooth_incriment_voltage: %s", settings.smooth_incriment_voltage()?"true":"false");
    Log(LOG_INFO, "min_available_voltage:    %d V", settings.min_available_voltage());
    Log(LOG_INFO, "max_step_number:          %d", settings.max_step_number());
    Log(LOG_INFO, "inc_voltage:              %d V", settings.inc_voltage());

    Log(LOG_INFO, "-----------------------------------------------------------\n");
    Log(LOG_INFO, "smooth_incriment_current: %s", settings.smooth_incriment_current()?"true":"false");
    Log(LOG_INFO, "min_available_current:    %d A", settings.min_available_current());
    Log(LOG_INFO, "max_step_number_current:  %d", settings.max_step_number_current());
    Log(LOG_INFO, "inc_current:              %d A", settings.inc_current());



    // ("Press any key!!\n");
    
    // // Start read routine
    
    // getchar();
    launcher(settings);

    Log(LOG_INFO, "Finished All Thread");
    Log(LOG_INFO, "Finished CHAdeMO");
    closeLog();
    return 0;
}
