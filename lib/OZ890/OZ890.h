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

#define OZ890_CELL_MAX      13U

typedef enum OZ890State_t
{
    OZ890_STATE_IDLE = 0,
    OZ890_STATE_DISCHARGING = 1,
    OZ890_STATE_CHARGING = 2,
    OZ890_STATE_INVALID = 3,
} OZ890State;

class OZ890 {
    public:
        OZ890();

        // Initialize the communication, shall be called first
        bool init();

        // Update all the data
        bool update();

        // Data, to get after a successful update()
        uint8_t cell_count;
        bool is_charging;
        bool is_discharging;
        bool is_fet_disabled; // If true, one or more flags below are set
        bool is_undertemp;
        bool is_overtemp;
        bool is_shortcircuit;
        bool is_overcurrent;
        bool is_undervoltage;
        bool is_overvoltage;

        float cell[OZ890_CELL_MAX];
        bool balancing[OZ890_CELL_MAX];
        float voltage;
        float current;

    private:
        uint8_t address = 0;    // Will be set at init
};