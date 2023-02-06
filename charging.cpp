#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


#include "charging.h"
#include "messages.h"
#include "ps_control.h"
#include "utils.h"

enum TChargingState{
    //STANDBY,
    INIT,
    PRECHARGING,
    CHARGING,
    TERMINATION,
    WAITING,
    ETERMINATION,
};

struct TChargingStateDesc{
    TChargingState state;
    const char *desc;
};


static struct TChargingStateDesc sDescr[] ={
    {INIT,         "INIT"},
    {PRECHARGING,  "PRECHARGING"},
    {CHARGING,     "CHARGING"},
    {TERMINATION,  "TERMINATION"},
    {WAITING,      "WAITING"},
    {ETERMINATION, "ETERMINATION"},

};

static const char* getStateDescription(const TChargingState st){
    for(int i=0;i<(int)(sizeof(sDescr)/sizeof(struct TChargingStateDesc));i++)
        if(sDescr[i].state == st)
            return sDescr[i].desc;
    return NULL;
}

// make it in main.cpp
//set_pin(D1, UP)

//////////////////////////////////////////////////////////////////////
static TChargingState chargingState = INIT;
static MMutex chargingStateMtx;
static void setChargingState(TChargingState st){
    Lock lock(chargingStateMtx);
    chargingState = st;
}
static TChargingState getChargingState(){
    Lock lock(chargingStateMtx);
    return chargingState;
}

static uint8_t chargingCurrentRequest;
static MMutex chargingCurrentRequestMtx;
static void setChargingCurrentRequest(const uint8_t curr){
    Lock lock(chargingCurrentRequestMtx);
    chargingCurrentRequest = curr;
}
static uint8_t getChargingCurrentRequest(){
    Lock lock(chargingCurrentRequestMtx);
    return chargingCurrentRequest;
}
    
static faultflag_bitset vechFaultFlag;
static MMutex vechFaultFlagMtx;
static void setVechFaultFlag(const faultflag_bitset &fl){
    Lock lock(vechFaultFlagMtx);
    vechFaultFlag = fl;
}    
static faultflag_bitset getVechFaultFlag(){
    Lock lock(vechFaultFlagMtx);
    return vechFaultFlag;
}    

static statusflag_bitset vechStatusFlag;
static MMutex vechStatusFlagMtx;
static void setVechStatusFlag(const statusflag_bitset &fl){
    Lock lock(vechStatusFlagMtx);
    vechStatusFlag = fl;
}
static statusflag_bitset getVechStatusFlag(){
    Lock lock(vechStatusFlagMtx);
    return vechStatusFlag;
}

static statusChargFlag_bitset chargerStatusFaultFlag(0x20); // ?? 0x4
static MMutex chargerStatusFaultFlagMtx;
static statusChargFlag_bitset getChargerStatusFaultFlag(){
    Lock lock(chargerStatusFaultFlagMtx);
    return chargerStatusFaultFlag;
}       
static void setChargerStatusFaultFlag(const statusChargFlag_bitset &fl){
    Lock lock(chargerStatusFaultFlagMtx);
    chargerStatusFaultFlag = fl;
}

static uint16_t psOutputVoltage = 0;
static MMutex psOutputVoltageMtx;
static void SetPSOutputVoltage(const uint16_t val){
    Lock lock(psOutputVoltageMtx);
    psOutputVoltage = val;
}
uint16_t GetOutputVoltage(){
    Lock lock(psOutputVoltageMtx);
    return psOutputVoltage;
}

static uint8_t psOutputCurrent = 0;
static MMutex psOutputCurrentMtx;
static void SetPSOutputCurrent(const uint8_t val){
    Lock lock(psOutputCurrentMtx);
    psOutputCurrent = val;
}
uint8_t GetOutputCurrent(){
    Lock lock(psOutputCurrentMtx);
    return psOutputCurrent;
}


static bool terminating = false;
static MMutex terminatingMtx;
void TerminateCharging(){
    Lock lock(terminatingMtx);
    terminating = true;
}
bool IsTerminating(){
    Lock lock(terminatingMtx);
    return terminating;
}


// ??????
static uint8_t MaximumChargingTime = 0;
static uint8_t MaximumChargingTime_min = 0;

///////////////////////////////////////// CAN READER ///////////////////
//CANReader::CANReader(Settings &settings):MThread(), was_100(false), was_101(false), was_102(false), set(settings){}

CANReader::CANReader():MThread("CAN Reader"), sock(-1), received100(false), received101(false), received102(false){}

CANReader::~CANReader(){}


bool CANReader::catch_initial_frames(const struct can_frame& frame){
    // TODO:
    switch (frame.can_id) {
        case 0x100:
            received100 = true;
            break;
        case 0x101:
            received101 = true;
            break;
        case 0x102:
            received102 = true;
            break;
        
    }
    return (received100 && received101 && received102)?true:false;
}

int CANReader::process(const struct can_frame& frame, bool is_termination){
    static int num_termination_frames = 15; // TODO constant
    switch (frame.can_id) {
        case 0x100:
            {
                if(is_termination)
                    num_termination_frames--;
                
                Frame100 fr(frame.data);
                Log(LOG_DEBUG, "[%s](100) MBV:%dV  CRConst:%d", fr.getMaxVoltage(), fr.getChargingRate());
                // TODO: process values
                // Maximum battery voltage:
                //     — Use this value to calculate
                //       “Threshold voltage.”                
                //
                // Charged rate reference constant:
                //     — Use this value only to display
                //       charged rate on the charger.
                //     — Charged rate(for display) =  Charged rate(#102.6) / Charged rate reference constant(#100.6) × 100                

            }
            break;
        case 0x101:
            {
                if(is_termination)
                    num_termination_frames--;
                Frame101 fr(frame.data);
                Log(LOG_DEBUG, "[%s](101) MCT:%d(10sec) MCT:%d(min) ECT:%d(min)", 
                            fr.getMaxChargingTime10s(), fr.getMaxChargingTime_min(), fr.getEstimateChargingTime_min());
                // TODO :Lock, process !!!
                MaximumChargingTime = fr.getMaxChargingTime10s();
                // Maximum charging time Maximum charging time permitted to the charger by the vehicle.(by 10seconds)
                // — Use this value to terminate charging by timer.

                MaximumChargingTime_min = fr.getMaxChargingTime_min();
                // Maximum charging time Maximum charging time permitted to the charger by the vehicle.(by minute)
                // — Use this value to terminate charging by timer.
                // — If H’101.1 is 0xFF, use this value in “Maximum charging time calculation” process.

                // Estimated charging time (by minute)

            }
            break;
        case 0x102:
            {
                Frame102 fr(frame.data);
                Log(LOG_DEBUG, "[%s]Charging current request: %d A", context, fr.getChargCurrentReq());

                setChargingCurrentRequest(fr.getChargCurrentReq());
                setVechStatusFlag(fr.getStatusFlag());
                print_vech_status_flag(fr.getStatusFlag());

                setVechFaultFlag(fr.getFaultFlag());
                //print_vech_fault_flag(fr.getFaultFlag());
                if(is_termination)
                    num_termination_frames--;
            }
            break;
        case 0x200:
            break;
    }
    if(num_termination_frames <=0)
        return 1;
    return 0;
}

int CANReader::can_open(){
    //bool is_read = true;
    int rc;

    struct sockaddr_can addr;
    struct ifreq ifr;

    sock = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        Log(LOG_ERR, "[%s]socket error: %d %s", context, errno, strerror(errno));
        return -1;
    }
    //if(is_read){
        struct can_filter filter[4];
        filter[0].can_id   = 0x100;
        filter[0].can_mask = CAN_SFF_MASK;
        filter[1].can_id   = 0x101;
        filter[1].can_mask = CAN_SFF_MASK;
        filter[2].can_id   = 0x102;
        filter[2].can_mask = CAN_SFF_MASK;
                
        filter[3].can_id   = 0x200;
        filter[3].can_mask = CAN_SFF_MASK;
                
        rc = ::setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
        if (rc < 0) {
            Log(LOG_ERR, "[%s]setsockopt error: %d %s", context, errno, strerror(errno));
            ::close(sock);
            return -1;
        }
    //}
    // TODO: set nonblock !!
    
    int enable = 0;
    rc = ::setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(enable));
    if (rc < 0) {
        Log(LOG_ERR, "[%s]setsockopt error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    } 
    
    
    ::strncpy(ifr.ifr_name, "can0", IFNAMSIZ);
    if (::ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
        Log(LOG_ERR, "[%s]ioctl error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    }    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    rc = ::bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (rc < 0) {
        Log(LOG_ERR, "[%s]bind error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    }
    return sock;
}

int CANReader::can_read(void *frame){
        int numBytes;
#ifdef RWSELECT
        struct timeval tv;
        fd_set rd;
        FD_ZERO(&rd);
        //FD_ZERO(&wr);
        tv.tv_sec = 2; // A.6.5 CAN reception error (1sec)
        tv.tv_usec = 0;
        FD_SET(sock, &rd);
        int r = ::select(sock + 1, &rd, NULL, NULL, &tv);
        if(r <0){
            //if (EINTR == errno)
            //    continue;
            Log(LOG_ERR, "[%s]Error select: %d %s", context, errno, strerror(errno));
            return errno;
        }
        if(!r){
            Log(LOG_WARNING, "[%s]select: timeout !!!", context);
            return 1;
        }
        if(FD_ISSET(sock, &rd)){
#endif
            numBytes = ::read(sock, frame, CAN_MTU);
            if(numBytes < (int)sizeof(struct can_frame)) {
                //if (EINTR == errno)
                //    continue;

            //::close(sockfd); ???
                Log(LOG_ERR, "[%s]Error read: %d %s", context, errno, strerror(errno));
                return -1;
           }
#ifdef RWSELECT
        }
#endif
    return 0;
}

int CANReader::can_close(){
    ::shutdown(sock, SHUT_RDWR);
    return ::close(sock);
}

int CANReader::run(){
    Log(LOG_INFO, "[%s]Started", context);
    volatile bool is_terminate  = false;
    int res = can_open();
    if(res <0){
        // error
        return -1;
    }
    while(!is_terminate){
        struct can_frame frame;
        //int res = can_read((void*)&frame);
        if(can_read((void*)&frame)<0){
            is_terminate = true;
            // TODO: may be set state TERMINATION ??
            setChargingState(ETERMINATION);
            continue;
        }
        switch(getChargingState()){
            case INIT:
            {
                if(catch_initial_frames(frame)){
                    setChargingState(PRECHARGING);   
                }
                break;
            }
            case PRECHARGING:
            case CHARGING:
            case WAITING:
                process(frame);
                break;
            case TERMINATION:
                process(frame, true);
                break;
            case ETERMINATION:
                is_terminate = true;
        }
    }
    can_close();
    Log(LOG_INFO, "[%s]Finishing ...", context);
    return 0;
}

void CANReader::print_vech_fault_flag(const faultflag_bitset &vhff){
    Log(LOG_DEBUG, "VFF(102): %lX", vhff.to_ulong());
    if(vhff.any()){
        vhff.test(BatteryOverVoltage)?
            Log(LOG_WARNING, "    BatteryOverVoltage .............. FAULT"):
            Log(LOG_WARNING, "    BatteryOverVoltage .............. OK");
        vhff.test(BatteryUunderVoltage)?
            Log(LOG_WARNING, "    BatteryUunderVoltage ... ........ FAULT"):
            Log(LOG_WARNING, "    BatteryUunderVoltage ............ OK");
        vhff.test(BatteryCurrentDeviationError)?
            Log(LOG_WARNING, "    BatteryCurrentDeviationError .... FAULT"):
            Log(LOG_WARNING, "    BatteryCurrentDeviationError .... OK");
        vhff.test(HighBatteryTemperature)?
            Log(LOG_WARNING, "    HighBatteryTemperature .......... FAULT"):
            Log(LOG_WARNING, "    HighBatteryTemperature .......... OK");
        vhff.test(BatteryVoltageDeviationError)?
            Log(LOG_WARNING, "    BatteryVoltageDeviationError .... FAULT"):
            Log(LOG_WARNING, "    BatteryVoltageDeviationError .... OK");
        return;
    }
}

void CANReader::print_vech_status_flag(const statusflag_bitset &vhsf){
    Log(LOG_DEBUG, "VFF(102): %lX", vhsf.to_ulong());
    vhsf.test(VehicleChargingEnabled)?
        Log(LOG_DEBUG, "    Vehicle charging enabled ... enabled"):
        Log(LOG_DEBUG, "    Vehicle charging enabled ... disabled");
    vhsf.test(VehicleShiftPosition)?
        Log(LOG_DEBUG, "    Vehicle shift position ..... other position"):
        Log(LOG_DEBUG, "    Vehicle shift position ..... 'Parking' position");
    vhsf.test(ChargingSystemFault)?
        Log(LOG_DEBUG, "    ChargingSystemFault ........ fault"):
        Log(LOG_DEBUG, "    ChargingSystemFault ........ normal");
    vhsf.test(VehicleStatus)?
        Log(LOG_DEBUG, "    VehicleStatus .............. 1: EV contactor open or termination of welding detection"):
        Log(LOG_DEBUG, "    VehicleStatus .............. 0: EV contactor close or during welding detection");
    vhsf.test(NormalStopRequestBeforeCharging)?
        Log(LOG_DEBUG, "    NormalStopRequestBeforeCharging ...Stop request"):
        Log(LOG_DEBUG, "    NormalStopRequestBeforeCharging ...no request");

}

/////////////////////////////////// CAN WRITER  /////////////////////////////
CANWriter::CANWriter(Settings &settings):MThread("CAN Writer"), set(settings){
    current_veh_set = set.min_available_current();
    cnt_sm_curr = 0;
}

CANWriter::~CANWriter(){}


int CANWriter::fill(Frame108 *f108, Frame109 *fr109, bool is_tremination){
    static int num_termination_frames = 5; // TODO constant
    if(!is_tremination){

        uint8_t avialable_current;
        if(set.smooth_incriment_current()){
            if(current_veh_set < set.available_output_current()){
                cnt_sm_curr++;
                if(!(cnt_sm_curr%set.max_step_number_current())){
                    cnt_sm_curr = 0;
                    current_veh_set += set.inc_current();
                    Log(LOG_INFO, "[%s]Set Veh Curr: %d.", context, current_veh_set);
                }
            }
            avialable_current = current_veh_set;
        }else{
           avialable_current = set.available_output_current();
        }
        Frame108 frame108(
                   1,
                   435,                             // V
                   avialable_current,               // A
                   set.max_available_voltage());    // V

        // fill frame  108
        *f108 = frame108;


        float Present_output_voltage = getRealOutputVoltage();
        float Present_output_current = getRealOutputCurrent();
        // fill frame 109
        Frame109 frame109(
            // protocol number:                                                                        
            // 0 - Before 0.9
            // 1 - 0.9, 0.9.1
            // 2 - 1.0.0, 1.0.1
             1,     // may be get from 102.0 (similary with 109.0)
             (uint16_t)Present_output_voltage,
             (uint8_t)Present_output_current,
             getChargerStatusFaultFlag(),
             MaximumChargingTime,  // 10 s/bit, 0 s to 2540 s, “0xFF” indicates usage of Byte 2 (by minute)
             MaximumChargingTime_min);        
        *fr109 = frame109;
        // TODO: terminate read thread
        //       May be wait when rd timeout 
    }else{
        uint8_t avialable_current = set.available_output_current();
        Frame108 frame108(
                   1,
                   435,                             // V
                   avialable_current,               // A
                   set.max_available_voltage());    // V
        *f108 = frame108;
        // smooth decrement voltage ????
        // if (current_voltage >= 0) {
        //    current_voltage --
        //    SetOutputVoltage()
        // }else {
        // }

        //SetOutputVoltage(0);


        float Present_output_voltage = getRealOutputVoltage();
        float Present_output_current = getRealOutputCurrent();
        Frame109 frame109(
            // protocol number:                                                                        
            // 0 - Before 0.9
            // 1 - 0.9, 0.9.1
            // 2 - 1.0.0, 1.0.1
             1,     // may be get from 102.0 (similary with 109.0)
             (uint16_t)Present_output_voltage,
             (uint8_t)Present_output_current,
             getChargerStatusFaultFlag(),
             MaximumChargingTime,  // 10 s/bit, 0 s to 2540 s, “0xFF” indicates usage of Byte 2 (by minute)
             MaximumChargingTime_min);        
        *fr109 = frame109;
        num_termination_frames --;
        if(num_termination_frames <=0){
            return 1;
        }

    }
    return 0;
}

int CANWriter::can_open(){
    int rc;

    struct sockaddr_can addr;
    struct ifreq ifr;

    sock = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        Log(LOG_ERR, "[%s]socket error: %d %s", context, errno, strerror(errno));
        return -1;
    }
    // TODO: set nonblock !!
    
    int enable = 0;
    rc = ::setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(enable));
    if (rc < 0) {
        Log(LOG_ERR, "[%s]setsockopt error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    } 
    
    
    ::strncpy(ifr.ifr_name, "can0", IFNAMSIZ);
    if (::ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
        Log(LOG_ERR, "[%s]ioctl error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    }    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    rc = ::bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (rc < 0) {
        Log(LOG_ERR, "[%s]bind error: %d %s", context, errno, strerror(errno));
        ::close(sock);
        return -1;
    }
    return sock;
}

int CANWriter::can_close(){
    ::shutdown(sock, SHUT_RDWR);
    return ::close(sock);
}

int CANWriter::can_write(const void *data){
#ifdef RWSELECT
    struct timeval tv;
    fd_set wr;
    //FD_ZERO(&rd);
    FD_ZERO(&wr);
    tv.tv_sec = 2; // 1 sec
    tv.tv_usec = 0;
    FD_SET(sock, &wr);
    int r = ::select(sock + 1, NULL, &wr, NULL, &tv);
    if(r <0){
        //if (EINTR == errno)
        //    continue;
            Log(LOG_ERR, "[%s]select error select: %d %s", context, errno, strerror(errno));
            return -1;
    }
    if(!r){
        Log(LOG_WARNING, "[%s]select: timeout", context);
        return -1;
    }
    if(FD_ISSET(sock, &wr)){
#endif
        int numBytes = ::write(sock, data, sizeof(struct can_frame));
        if(numBytes < (int)sizeof(struct can_frame)) {
                //if (EINTR == errno)
                //continue;

            Log(LOG_ERR, "[%s]Error write: %d %s", context, errno, strerror(errno));
	        return -1;
        }
#ifdef RWSELECT
    }
#endif
    return 0;
}

int CANWriter::run(){
    Log(LOG_INFO, "[%s]Started", context);
    volatile bool is_terminate  = false;
    int res = can_open();
    if(res < 0) {
        // error
    }
    while(!is_terminate){
        Frame108 fr108;
        Frame109 fr109;
        switch(getChargingState()){
            case INIT:
                break;
            case PRECHARGING:
            case CHARGING:
            case WAITING:
            {
                fill(&fr108, &fr109);
                // TODO: check results
                int ret = can_write((void*)&fr108);
                if(ret <0) {
                    is_terminate = true;
                    setChargingState(ETERMINATION);
                    continue;
                }
                ret = can_write((void*)&fr109);
                if(ret <0) {
                    is_terminate = true;
                    setChargingState(ETERMINATION);
                    continue;
                }
            }
            break;
            case TERMINATION:
            {
                if(fill(&fr108, &fr109, true)){
                    is_terminate = true;
                    continue;
                }
                int ret = can_write((void*)&fr108);
                if(ret <0) {
                    is_terminate = true;
                    continue;
                }
                ret = can_write((void*)&fr109);
                if(ret <0) {
                    is_terminate = true;
                    continue;
                }
            }
            break;
            case ETERMINATION:
            {
                is_terminate = true;
                continue;                
            }

        }
        usleep(100000);
    }
    can_close();
    Log(LOG_INFO, "[%s]Finishing ...", context);
    return 0;
}

// GPIO0 11
#define D1_PIN  0
                    
// GPIO2 13
#define D2_PIN  2
                    
// GPIO3 15.
#define J_PIN   3
                        
                    
void setup_GPIO(){
    wiringPiSetup ();
    pinMode(D1_PIN, OUTPUT);
    pinMode(D2_PIN, OUTPUT);
    pinMode(J_PIN, INPUT);

    // switch off rele
    digitalWrite (D1_PIN, LOW);
    delayMicroseconds(250000);
    digitalWrite (D2_PIN, LOW);
    delayMicroseconds(250000);
}

ChargingCtl::ChargingCtl(Settings &settings):MThread("ChargingCtl"), set(settings){}

ChargingCtl::~ChargingCtl(){}


int ChargingCtl::run(){
    Log(LOG_INFO, "[%s]Started", context);
    int waitVechStatusCnt = 0;
    volatile bool is_terminate = false;
    bool isCharging = false;

    setup_GPIO();
    // UP D1
    digitalWrite (D1_PIN, HIGH);

    while(!is_terminate){
        int JPIN = digitalRead(J_PIN);
        statusflag_bitset vStatus = getVechStatusFlag();
        faultflag_bitset vFault = getVechFaultFlag();
        uint8_t vIreq = getChargingCurrentRequest();
        if(vFault.any()){
            CANReader::print_vech_fault_flag(vFault);
            setChargingState(ETERMINATION); // TERMINATION ??
        }
        TChargingState chState = getChargingState();
        Log(LOG_DEBUG, "[%s]Charge state: %d %s", context, (int)chState, getStateDescription(chState));
        switch(chState){
            case INIT:
                break;
            case PRECHARGING:
                //if(!JPIN && GetVehicleChargingEnabled()){
                //if(!JPIN && vStatus[VehicleChargingEnabled]){
                if(!JPIN && vStatus.test(VehicleChargingEnabled)){
                   // TODO: Insulation test on output DC circuit, then UP D2 !!!
                    digitalWrite (D2_PIN, HIGH);
                    Log(LOG_INFO, "[%s]PRECHARGING: D2 UP", context);
                    statusChargFlag_bitset tfl = getChargerStatusFaultFlag();
                    // SetChargerStatus(1); // may be if request current >=6 !!!
                    // SetChargingStopControl(0);
                    //tfl[ChargerStatus]       = 1;
                    //tfl[ChargingStopControl] = 0;
                    tfl.set(ChargerStatus);
                    tfl.reset(ChargingStopControl);

                    // "Charging connector lock" flag to 1 ?? 
                    //tfl[ChargingConnectorLock] = 1;
                    tfl.set(ChargingConnectorLock);
                    setChargerStatusFaultFlag(tfl);

                    setChargingState(WAITING);
                }
                break;
            case WAITING:
                {
                    if(!isCharging){
                        //if(!vehStatus){
                        //if(!vStatus[VehicleStatus]){
                        if(!vStatus.test(VehicleStatus)){
                            // TODO: May be set timeout
                            waitVechStatusCnt=0;
                            isCharging = true;
                            setChargingState(CHARGING);
                            SetPSState(PS_ON);
                            Log(LOG_INFO, "[%s]WAITING: SWITCH ON PS", context);
                            SetPSOutputVoltage(set.max_available_voltage());
                        }else{
                            waitVechStatusCnt++;
                            if(waitVechStatusCnt >= 10){
                                // May be terminate
                                Log(LOG_WARNING, "[%s]Waiting(noch) more than 1 sec (%d)!!!", context, waitVechStatusCnt);
                            } // ~ 1sec
                        }
                    }else{
                        //if(vehStatus){
                        //if(vStatus[VehicleStatus]){
                        if(vStatus.test(VehicleStatus)){
                            // TODO: May be set timeout
                            waitVechStatusCnt=0;
                            isCharging = false;
                            // digitalWrite (D2_PIN, LOW);
                            // digitalWrite (D1_PIN, LOW);

                            statusChargFlag_bitset tfl = getChargerStatusFaultFlag();
                            // "Charging connector lock" flag to 0 ??
                            //tfl[ChargingConnectorLock] = 0;
                            tfl.reset(ChargingConnectorLock);
                            setChargerStatusFaultFlag(tfl);
                            setChargingState(TERMINATION);
                            Log(LOG_INFO, "[%s]WAITING: SET TERMINATION", context);
                        }else{
                            waitVechStatusCnt++;
                            if(waitVechStatusCnt >= 10){
                                // May be terminate
                                Log(LOG_WARNING, "[%s]Waiting(ch) more than 1 sec (%d)!!!", context, waitVechStatusCnt);
                            } // ~ 1sec
                        }
                    }
                }
                break;
            case CHARGING:
            {
                uint8_t tCurr = (uint8_t)((vIreq<=set.local_current_limit())?vIreq:set.local_current_limit());
                SetPSOutputCurrent(tCurr);
                Log(LOG_INFO, "[%s]CHARGING: SET CURRENT: %d", context, tCurr);
                //if (JPIN && !GetVehicleChargingEnabled()){
                //if(JPIN && !vStatus[VehicleChargingEnabled]){
                if(JPIN && !vStatus.test(VehicleChargingEnabled)){
                    // SetChargerStatus(0);
                    // SetChargingStopControl(1);
                    // "Charging connector lock" flag to 0 ??
                    statusChargFlag_bitset tfl = getChargerStatusFaultFlag();
                    //tfl[ChargerStatus]       = 0;
                    //tfl[ChargingStopControl] = 1;
                    tfl.reset(ChargerStatus);
                    tfl.set(ChargingStopControl);
                    
                    // "Charging connector lock" flag to 0 ??
                    //    tfl[ChargingConnectorLock] = 0;


                    setChargerStatusFaultFlag(tfl);

                    setChargingState(WAITING);
                    SetPSOutputCurrent(0);
                    SetPSState(PS_OFF);
                    Log(LOG_INFO, "[%s]CHARGING: SWITCH OFF PS", context);
                }
                break;
            }
            case TERMINATION:
            {
                is_terminate = true;
                digitalWrite (D2_PIN, LOW);
                Log(LOG_INFO, "[%s]TERMINATION: D2 DOWN", context);
            }
            break;
            case ETERMINATION:
            {
                is_terminate = true;
                SetPSOutputCurrent(0);
                SetPSState(PS_OFF);
                Log(LOG_INFO, "[%s]EMCY TERMINATION: SWITCH OFF PS", context);
                digitalWrite (D2_PIN, LOW);
                Log(LOG_INFO, "[%s]EMCY TERMINATION: D2 DOWN", context);
            }
            break;
        }
        usleep(100000);
    }
    // DOWN D1
    digitalWrite (D1_PIN, LOW);
    Log(LOG_INFO, "[%s]Finishing  ...", context);
    return 0;
}

TerminationTh::TerminationTh():MThread("Termination"){}
TerminationTh::~TerminationTh(){}

int TerminationTh::run(){
    Log(LOG_INFO, "[%s]Started", context);
    while(1){
        if(IsTerminating())
            break;
        if(getChargingState() == ETERMINATION){
            TerminateCharging();
            break;
        }
        usleep(100000);
    }
//    SetChargerStatus(0); // may be if request current >=6 !!!
//    SetChargingStopControl(1);
    statusChargFlag_bitset tfl = getChargerStatusFaultFlag();
    //tfl[ChargerStatus]       = 0;
    //tfl[ChargingStopControl] = 1;
    tfl.reset(ChargerStatus);
    tfl.set(ChargingStopControl);

    setChargerStatusFaultFlag(tfl);
    Log(LOG_INFO, "[%s]Finishing  ...", context);
    return 0;
}