/*
 * Copyright (C) 2017-2018 infactum (infactum@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__    20190606

#include <stdint.h>

#include "inireader.h"

class Settings {
private:
    uint16_t max_available_voltage_;

    uint8_t available_output_current_;

    uint8_t local_current_limit_;

    bool smooth_incriment_voltage_;

    uint16_t min_available_voltage_;

    int max_step_number_;

    int inc_voltage_;

    
    bool smooth_incriment_current_;
    
    uint8_t min_available_current_;

    int max_step_number_current_;

    int inc_current_;


public:
    explicit Settings(ini_file &reader);

    //Settings(const Settings &) = delete;

    //Settings &operator=(const Settings &) = delete;

    uint16_t max_available_voltage() const { return max_available_voltage_; };

    uint8_t available_output_current() const { return available_output_current_; };

    uint8_t local_current_limit() const { return local_current_limit_; };

    bool smooth_incriment_voltage() const { return smooth_incriment_voltage_; };

    uint16_t min_available_voltage() const { return min_available_voltage_; };

    int max_step_number() const { return max_step_number_; };

    int inc_voltage() const { return inc_voltage_; };
    
    
    
    bool smooth_incriment_current() const {return smooth_incriment_current_; };
    
    uint8_t min_available_current() const { return min_available_current_; };

    int max_step_number_current() const {return max_step_number_current_; };

    int inc_current() const {return inc_current_; };


};

#endif // __SETTINGS_H__
