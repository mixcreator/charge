#include "messages.h"

// Vechicle Frames
// [ID:H‘100]
//Frame100::Frame100(const unsigned char* data, int len) {
Frame100::Frame100(const unsigned char* data) {
		// ax_voltage = le16toh(*(std::uint16_t *)(data + 4));
		max_voltage = le16toh(*(uint16_t *)(data + 4));
		charging_rate = data[6];
}

// [ID:H‘101]
//Frame101::Frame101(const unsigned char* data, int len) {
Frame101::Frame101(const unsigned char* data) {
		max_charging_time_10s = data[1];
		max_charging_time_min = data[2];
		estimate_charging_time_min = data[3];
		bat_total_capacity = le16toh(*(uint16_t *)(data + 5));
}
	


// [ID:H‘102]
//Frame102::Frame102(const unsigned char* data, int len) {
Frame102::Frame102(const unsigned char* data) {
        protocol_number = data[0];
        target_bat_voltage= le16toh(*(uint16_t *)(data + 1));
        //target_bat_voltage= le16toh(*const_cast<std::uint16_t*>(data + 1));
        charging_current_req = data[3];
        fault_flag = data[4];
        status_flag = data[5];
        charged_rate = data[6];
}




//Charger Frames
// [ID:H‘108]
//Frame108::Frame108(const unsigned char* data, int len) {
Frame108::Frame108(const unsigned char* data) {
        identifier = data[0];
        available_out_voltage = le16toh(*(uint16_t *)(data + 1));
        available_out_current = data[3];
        treshold_voltage = le16toh(*(uint16_t *)(data + 4));
}

Frame108::Frame108(uint8_t identifier, uint16_t av_out_volt,
             uint8_t av_out_curr, uint16_t treshold_volt){
        frame.can_id = 0x108;
        frame.can_dlc = 8;
        frame.data[0] = identifier;
        uint16_t v = htole16(av_out_volt);
        frame.data[1] = v & 0xff;
        frame.data[2] = (v >> 8) & 0xff;
        uint8_t c = av_out_curr;
        frame.data[3] = c;
        uint16_t tv = htole16(treshold_volt);
        frame.data[4] = tv & 0xff;
        frame.data[5] = (tv >> 8) & 0xff;
        frame.data[6] = 0xF0;
        frame.data[7] = 0;
}
    

// [ID:H‘109]
//Frame109::Frame109(const unsigned char* data, int len) {
Frame109::Frame109(const unsigned char* data) {
        protocol_number = data[0];
        present_out_voltage = le16toh(*(uint16_t *)(data + 1));
        present_charging_current = data[3];
        status_fault_flag = data[5];
        remaining_charging_time_10s = data[6];
        remeining_charging_time_min = data[7];
}

Frame109::Frame109(
             uint8_t prot_number,
             uint16_t present_out_volt,
             uint8_t present_charging_curr,
             //uint8_t status_fault_flag,
             statusChargFlag_bitset status_fault_flag,
             uint8_t remaining_charg_tm_10s,
             uint8_t remeining_charg_tm_min){
     	frame.can_id = 0x109;
        frame.can_dlc = 8;
        frame.data[0] = prot_number;
        uint16_t pv = htole16(present_out_volt);
        frame.data[1] = pv & 0xff;
        frame.data[2] = (pv >> 8) & 0xff;
        frame.data[3] = present_charging_curr;
        frame.data[4] = 0;
        frame.data[5] = status_fault_flag.to_ulong();
        frame.data[6] = remaining_charg_tm_10s;
        frame.data[7] = remeining_charg_tm_min;
}
