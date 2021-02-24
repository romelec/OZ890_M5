/**
 * OZ890 BMS chip library
 * 
 * Reads data from the OZ890.
 * The configuration (EEPROM write) is not implemented, 
 * not needed for this project
 * 
 * Romelec 2020
 * Inspired from https://github.com/amund7/OZ890-BMS
 */


#include <Arduino.h>
#include <M5Stack.h>

#include "OZ890.h"

#define OZ890_DEBUG true

#define OZ890_DEVICE_ADDR       0x30   // Base address 
#define OZ890_DEVICE_ADDR_NB    0x0F   // Number of different addresses (to find it)

#define OZ890_ADDR_CELL_NB  0x06    // bits 0->3
#define OZ890_ADDR_FET_ERR  0x1F    // bits 0->5
#define OZ890_ADDR_STATE    0x20    // DSG: bit 2 - CHG: bit 3
#define OZ890_ADDR_BLEEDING 0x22    // 2 bytes
#define OZ890_ADDR_CELL     0x32    // 26 bytes
#define OZ890_ADDR_CURRENT  0x54    // 2 bytes


OZ890::OZ890(){
}

bool OZ890::init()
{
    // Initialize I2C
    //Wire.begin();
    //Wire.setClock(100000);

    // Look for the address
    uint8_t error = 1;
    uint8_t i = 0;
    /*for (; i<OZ890_DEVICE_ADDR_NB && error != 0; i++)
    {
        Wire.beginTransmission(OZ890_DEVICE_ADDR+i);
        error = Wire.endTransmission();
    }

    if (i == OZ890_DEVICE_ADDR_NB || error)
    {
        // Init failed
        Serial.println("init failed");
        return false;
    }
    address = OZ890_DEVICE_ADDR + i;
#ifdef OZ890_DEBUG
    Serial.print("Address: ");
    Serial.println(address, HEX);
    Serial.println(i);
#endif*/
    address = 0x30;

    // Get the cell count
    if (!M5.I2C.readByte(address, OZ890_ADDR_CELL_NB, &cell_count))
    {
#ifdef OZ890_DEBUG
        Serial.println("Can't get cell count");
#endif
        return false;
    }
    cell_count = cell_count & 0x0F;
    if (cell_count < 5 || cell_count > OZ890_CELL_MAX)
    {
#ifdef OZ890_DEBUG
        Serial.print("Invalid cell count:");
        Serial.println(cell_count);
#endif
        
        return false;
    }
    Serial.print("cell count:");
        Serial.println(cell_count);
    return true;
}

bool OZ890::update()
{
    uint8_t tmpu8;
    uint8_t tmpu8_2 [2];
    uint16_t tmpu16;
    int32_t tmp32;
    uint8_t i;

    // Get the cell voltages
    voltage = 0.0;
    for(i=0; i < cell_count; i++)
    {
        if (!M5.I2C.readBytes(address, OZ890_ADDR_CELL + (2*i), 2, tmpu8_2))
        {
#ifdef OZ890_DEBUG
            Serial.println("Can't read cell voltages");
#endif
            return false;
        }
        tmp32 = ((tmpu8_2[0] >> 3) & 0x1F) | (tmpu8_2[1] << 5);
        cell[i] = ((float)tmp32 * 1.22) / 1000.0;
        voltage += cell[i];
    }

    // Get the FET disable state
    if (!M5.I2C.readByte(address, OZ890_ADDR_FET_ERR, &tmpu8))
    {
#ifdef OZ890_DEBUG
        Serial.println("Can't get FET state");
#endif
        return false;
    }
    is_fet_disabled = !!(tmpu8 & 0x3F);
    is_undertemp    = !!(tmpu8 & 0x20);
    is_overtemp     = !!(tmpu8 & 0x10);
    is_shortcircuit = !!(tmpu8 & 0x08);
    is_overcurrent  = !!(tmpu8 & 0x04);
    is_undervoltage = !!(tmpu8 & 0x02);
    is_overvoltage  = !!(tmpu8 & 0x01);

    // Get the charge state
    if (!M5.I2C.readByte(address, OZ890_ADDR_STATE, &tmpu8))
    {
#ifdef OZ890_DEBUG
        Serial.println("Can't get cell state");
#endif
        return false;
    }
    is_charging = !!(tmpu8 & 0x04);
    is_discharging = !!(tmpu8 & 0x02);

    // Get the balancing state
    if (!M5.I2C.readBytes(address, OZ890_ADDR_BLEEDING, 2, (uint8_t*)&tmpu16))
    {
#ifdef OZ890_DEBUG
        Serial.println("Can't read balancing state");
#endif
        return false;
    }
    for (i = 0; i < OZ890_CELL_MAX; i++)
    {
        balancing[i] = !!((tmpu16 >> i) & 0x01);
    }

    // Get the current
    if (!M5.I2C.readBytes(address, OZ890_ADDR_CURRENT, 2, tmpu8_2))
    {
#ifdef OZ890_DEBUG
        Serial.println("Can't read cell voltages");
#endif
        return false;
    }
    tmp32 = tmpu8_2[0] | (tmpu8_2[1] << 8);

    // TODO get the sense recicsot from EEPROM (2 mOhm)
    current = (float)tmp32 * 7.63 / 2000;

    return true;
}