#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SKIP_CHARGING 1
#define DISCHARGE_WAIT_TIME 10.0f       // seconds
#define CHARGE_WAIT_TIME 60.0f          // seconds
#define DISCHARGE_CUTOFF_VOLTAGE 6.00f  // volts, 3.00
#define ERROR_VOLTAGE 5.0f             // volts
#define ERROR_INTERNAL_RESISTANCE 0.50f // ohms

#define NO_BATTERY_VOLTAGE 0.10f    // volts
#define FULLY_CHARGED_VOLTAGE 8.1f // volts
#define MIN_CHARGING_CURRENT 70.0f  // mA

#define TIME_BETWEEN_MEASURE_POINTS 600.0f // seconds
#define MEASURE_POINTS 60

#endif // CONSTANTS_H
