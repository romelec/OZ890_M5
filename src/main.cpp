#include <Arduino.h>
#include <M5Stack.h>
#include "OZ890.h"

OZ890 Bms;
uint8_t i = 0;

void setup()
{
    M5.begin();
    M5.Power.begin();
    M5.Lcd.begin();
    M5.Lcd.setBrightness(255);

    Serial.begin(115200);

    M5.Speaker.begin();
    M5.Speaker.setBeep(600, 100);


    if (!Bms.init())
    {
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.print("Comm Init ERROR");
        M5.Lcd.setTextSize(1);

        delay(2000);
        M5.Power.deepSleep();
    }
}

void loop()
{
    M5.update();

    M5.Lcd.clear(BLACK);

    if (M5.BtnC.pressedFor(2000))
    {
        M5.Power.deepSleep();
    }

    if (!Bms.update())
    {
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.print("Comm ERROR");
        M5.Lcd.setTextSize(1);

        delay(1000);
        return;
    }
    Serial.printf("%.2fV, %.2f (A), is_charging %d, is_discharging %d, is_fet_disabled %d\n",
        Bms.voltage, Bms.current, Bms.is_charging, Bms.is_discharging, Bms.is_fet_disabled);
    Serial.printf("UT %d, OT %d, SC %d, OC %d, UV %d, OV %d\n",
        Bms.is_undertemp, Bms.is_overtemp, Bms.is_shortcircuit, Bms.is_overcurrent, Bms.is_undervoltage, Bms.is_overvoltage);

    for(uint8_t i = 0; i < Bms.cell_count; i++)
    {
        Serial.printf("Cell %d: %.2fv, %d\n", i, Bms.cell[i], Bms.balancing[i]);

        float tmp = (Bms.cell[i] - 2.9) *100.0/(4.2-2.9);
        if (tmp < 0.0) tmp = 0.0;
        if (tmp > 100.0) tmp = 100.0;
        M5.Lcd.fillRect(10, 12*i, 300, 10,0);
        M5.Lcd.progressBar(10, 12*i, 300, 10, (uint8_t)tmp);
        M5.Lcd.drawFloat(Bms.cell[i],3,10,12*i+2);
    }

    M5.Lcd.setTextSize(2);

    M5.Lcd.setCursor(10, 160);
    M5.Lcd.printf("%.2fV, %.3fA", Bms.voltage, Bms.current);

    M5.Lcd.setCursor(10, 190);
    if (Bms.is_charging) M5.Lcd.print("CHG ");
    if (Bms.is_discharging) M5.Lcd.print("DCHG ");

    M5.Lcd.setCursor(10, 210);
    if (Bms.is_fet_disabled) M5.Lcd.print("FET: ");
    if (Bms.is_undertemp) M5.Lcd.print("UT ");
    if (Bms.is_overtemp) M5.Lcd.print("OT ");
    if (Bms.is_shortcircuit) M5.Lcd.print("SC ");
    if (Bms.is_overcurrent) M5.Lcd.print("OC ");
    if (Bms.is_undervoltage) M5.Lcd.print("UV ");
    if (Bms.is_overvoltage) M5.Lcd.print("OV ");
    M5.Lcd.setTextSize(1);

    if (Bms.is_fet_disabled)
    {
        M5.Speaker.beep();
        delay(500);
        M5.Speaker.mute();
        delay(4500);
    }
    else
    {
        M5.Speaker.mute();
        delay(1000);
    }
}