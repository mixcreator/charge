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

#include "settings.h"

Settings::Settings(ini_file &reader) {


    max_available_voltage_ = static_cast<uint16_t>(reader.valueInt("general", "max_available_voltage", 435));

    available_output_current_ = (uint8_t)reader.valueInt("general", "available_output_current", 3);

    local_current_limit_ = (uint8_t)reader.valueInt("general", "local_current_limit", 20);

    smooth_incriment_voltage_ = (bool)reader.valueBool("smooth", "smooth_incriment_voltage", false);

    min_available_voltage_ = (uint16_t)reader.valueInt("smooth", "min_available_voltage", 380);

    max_step_number_ = reader.valueInt("smooth", "max_step_number", 10);

    inc_voltage_ = reader.valueInt("smooth", "inc_voltage", 1);
    


    smooth_incriment_current_ = (bool)reader.valueBool("smooth_current", "smooth_incriment_current", false);
    
    min_available_current_ = (uint8_t)reader.valueInt("smooth_current", "min_available_current", 1);

    max_step_number_current_ = reader.valueInt("smooth_current", "max_step_number_current", 10);

    inc_current_ = reader.valueInt("smooth_current", "inc_current", 1);

    
}

