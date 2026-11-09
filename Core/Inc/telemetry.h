/*
 * telemetry.h
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */

#ifndef INC_TELEMETRY_H_
#define INC_TELEMETRY_H_

#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <math.h>

#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_accelero.h"
#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_tsensor.h"
#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_gyro.h"
#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_magneto.h"
#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_psensor.h"
#include "../../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_hsensor.h"


// Threshold Limits
#define TEMP_LOW              15.0f
#define TEMP_HIGH             35.0f
#define PRESSURE_LOW          950.0f
#define PRESSURE_HIGH         1050.0f
#define HUMIDITY_LOW          20.0f
#define HUMIDITY_HIGH         95.0f
#define GYRO_HIGH             100.0f
#define MAG_HIGH              3.0f
#define UPSIDE_DOWN_THRESHOLD -5.0f

// Globally accessible sensor data fields
extern float temp_data;
extern float pressure_data;
extern float humidity_data;
extern float accel_z;
extern float gyro_mag;
extern float mag_mag;

// System Declarations
void Sensors_Init(void);
void Sensors_ReadAll(void);
void Telemetry_Send(void);
int Threshold_Check(void);
int IsUpsideDown(void);



#endif /* INC_TELEMETRY_H_ */
