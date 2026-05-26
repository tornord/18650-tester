#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SKIP_CHARGING 0
#define DISCHARGE_WAIT_TIME 3600.0f       // seconds
#define CHARGE_WAIT_TIME 60.0f          // seconds
#define DISCHARGE_CUTOFF_VOLTAGE 3.00f  // volts, 3.00
#define ERROR_VOLTAGE 2.50f             // volts
#define ERROR_INTERNAL_RESISTANCE 0.30f // ohms

#define NO_BATTERY_VOLTAGE 0.10f    // volts
#define FULLY_CHARGED_VOLTAGE 4.05f // volts
#define MIN_CHARGING_CURRENT 70.0f  // mA

#define TIME_BETWEEN_MEASURE_POINTS 300.0f // seconds
#define MEASURE_POINTS 60

#endif // CONSTANTS_H
