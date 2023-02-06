#ifndef __MESSAGES_H__
#define __MESSAGES_H__ 20180516

#ifndef __cplusplus
#error C++ is required
#endif


#include <stdlib.h>
#include <stdint.h>

#include <linux/can.h>
#include <linux/can/raw.h>


#include <bitset>


const int numb_vehicle_status_flags_bits = 5;
const int numb_vehicle_fault_flags_bits  = 5;
const int numb_charger_status_flags_bits = 6;

typedef std::bitset<numb_vehicle_fault_flags_bits> faultflag_bitset;
typedef std::bitset<numb_vehicle_status_flags_bits> statusflag_bitset;

// Vechicle Frames
// [ID:H‘100]
class Frame100 {
public:
    //Frame100(const unsigned char* data, int len);
    explicit Frame100(const unsigned char* data);
 
    inline uint16_t getMaxVoltage() {
		return max_voltage;
    }
    
    inline uint8_t getChargingRate() const{
		return charging_rate;
	}

private:
    // Maximum battery voltage [ID:H‘100.4,5] 
    uint16_t max_voltage;
    
    // Charged rate reference constant [ID:H‘100.6]
    uint8_t charging_rate;
};


// [ID:H‘101]
class Frame101 {
public:
    //Frame101(const unsigned char* data, int len);
    explicit Frame101(const unsigned char* data);
	
	inline uint8_t getMaxChargingTime10s() const{
		return max_charging_time_10s;
	}

    inline uint8_t getMaxChargingTime_min() const{
		return max_charging_time_min;
	}
	
    inline uint8_t getEstimateChargingTime_min() const{
		return estimate_charging_time_min;
	}
	
    inline uint16_t getBatTotalCapacity() const{
		return bat_total_capacity;
	}

private:
    // Maximum charging time (by 10 s) [ID:H‘101.1]
    uint8_t max_charging_time_10s;
    
    // Maximum charging time (by minute) [ID:H‘101.2]
    uint8_t max_charging_time_min;
    
    // Estimated charging time (by minute) [ID:H‘101.3]
    uint8_t estimate_charging_time_min;
    
    // Total capacity of battery [ID:H‘101.5,6]
    uint16_t bat_total_capacity;
};

// faultflag bitset indexes
enum{
    BatteryOverVoltage,             // (0: normal, 1: fault)
    BatteryUunderVoltage,           // (0: normal, 1: fault)
    BatteryCurrentDeviationError,   // (0: normal, 1: fault)
    HighBatteryTemperature,         // (0: normal, 1: fault)
    BatteryVoltageDeviationError    // (0: normal, 1: fault)
};

// statusflag bitset indexes
enum{
    VehicleChargingEnabled,         // (0: disabled, 1: enabled)
    VehicleShiftPosition,           // (0: “Parking” position, 1: other position)
    ChargingSystemFault,            // (0: normal, 1: fault)
    VehicleStatus,                  /* (0: EV contactor close or during welding
                                        detection, 1: EV contactor open or
                                        termination of welding detection)
    
                                    */
    NormalStopRequestBeforeCharging // (0: No request, 1: Stop request)    
};

// [ID:H‘102]
class Frame102 {
public:

    //Frame102(const unsigned char* data, int len);
    explicit Frame102(const unsigned char* data);

    inline uint8_t getProtocolNumber() const{
        return protocol_number;
    }
    
    inline uint16_t  getTargetBatVoltage() const{
        return target_bat_voltage;
    }
    
    inline uint8_t getChargCurrentReq() const{
        return charging_current_req;
    }
    
    inline faultflag_bitset getFaultFlag() const{
        return fault_flag;
    }
    
    inline statusflag_bitset getStatusFlag() const{
        return status_flag;
    }
    
    inline uint8_t getChargedRate() const{
        return charged_rate;
    }

private:
    // CHAdeMO control protocol number [ID:H‘102.0]
    uint8_t protocol_number;
    
    // Target battery voltage [ID:H‘102.1,2]
    uint16_t target_bat_voltage;
    
    // Charging current request [ID:H‘102.3]
    uint8_t charging_current_req;
    
    // Fault flag [ID:H‘102.4]
    faultflag_bitset fault_flag;
    
    // Status flag [ID:H‘102.5]
    statusflag_bitset status_flag;
    
    // Charged rate [ID:H‘102.6]
    uint8_t charged_rate;
};

//Charger Frames
// [ID:H‘108]
class Frame108 {
public:

    Frame108(){}

    //Frame108(const unsigned char* data, int len);
    explicit Frame108(const unsigned char* data);

    Frame108(uint8_t identifier, uint16_t av_out_volt,
             uint8_t av_out_curr, uint16_t treshold_volt);
    
    inline struct can_frame &getFrame() const{
        return (can_frame&)frame;
    }

    inline uint8_t getIdentifier() const{
        return identifier;
    }
    
    inline uint16_t getAvailableOutVoltage() const{
        return available_out_voltage;
    }

    inline uint8_t getAvailableCurrent() const{
        return available_out_current;
    }

    inline uint16_t getTresholdVoltage() const{
        return treshold_voltage;
    }
    
//private:
    uint8_t identifier;
    uint16_t available_out_voltage;
    uint8_t available_out_current;
    uint16_t treshold_voltage;
    struct can_frame frame;
};

// charger statusfault flag indexes
enum{
    ChargerStatus,                  // (0: standby, 1: charging)       
    ChargerMalfunction,             // (0: normal, 1: fault)
    ChargingConnectorLock,          // (0: open, 1: locked)
    BatteryIncompatibility,         // (0: compatible, 1: incompatible)
    ChargingSystemMalfunction,      // (0: normal, 1: malfunction)
    ChargingStopControl             // (0: operating, 1: stopped or stop charging)    
};

typedef std::bitset<numb_charger_status_flags_bits> statusChargFlag_bitset;

// [ID:H‘109]
class Frame109 {
public:
    
    Frame109(){};

    //Frame109(const unsigned char* data, int len);
    explicit Frame109(const unsigned char* data);

    Frame109(
             uint8_t prot_number,
             uint16_t present_out_volt,
             uint8_t present_charging_curr,
             //uint8_t status_fault_flag,
             statusChargFlag_bitset status_fault_flag,
             uint8_t remaining_charg_tm_10s,
             uint8_t remeining_charg_tm_min);

    inline struct can_frame &getFrame() const{
        return (can_frame&)frame;
    }

    inline uint8_t getProtocolNumber() const{
        return protocol_number;
    }
    
    inline uint16_t getPresentOutVoltage() const{
        return present_out_voltage;
    }
    
    inline uint8_t getPresentChargingCurrent() const{
        return present_charging_current;
    }
    
    inline uint8_t getRemainingChargingTime() const{
        return remaining_charging_time_10s;
    }

    inline uint8_t getRemainingChargingTime_min() const{
        return remeining_charging_time_min;
    }

    inline statusChargFlag_bitset getStatusFaultFlag() const{
        return status_fault_flag;
    }
    
//private:
    uint8_t protocol_number;
    uint16_t present_out_voltage;
    uint8_t present_charging_current;
    statusChargFlag_bitset status_fault_flag;
    uint8_t remaining_charging_time_10s;
    uint8_t remeining_charging_time_min;
    struct can_frame frame;
};
#endif
