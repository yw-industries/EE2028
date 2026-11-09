/*
 * telemetry.c
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */


#include "telemetry.h"
#include "uart.h"

// Global Definitions
float temp_data;
float pressure_data;
float humidity_data;
float accel_z;
float gyro_mag;
float mag_mag;

void Sensors_Init(void)
{
    BSP_ACCELERO_Init();
    BSP_TSENSOR_Init();
    BSP_GYRO_Init();
    BSP_MAGNETO_Init();
    BSP_PSENSOR_Init();
    BSP_HSENSOR_Init();
}

void Sensors_ReadAll(void)
{
    int16_t accel_data_i16[3] = { 0 };
    BSP_ACCELERO_AccGetXYZ(accel_data_i16);
    accel_z = (float)accel_data_i16[2] * (9.8f / 1000.0f);

    temp_data = BSP_TSENSOR_ReadTemp();
    humidity_data = BSP_HSENSOR_ReadHumidity();
    pressure_data = BSP_PSENSOR_ReadPressure();

    float gyro_data[3];
    BSP_GYRO_GetXYZ(gyro_data);
    gyro_mag = sqrtf(gyro_data[0]*gyro_data[0] + gyro_data[1]*gyro_data[1] + gyro_data[2]*gyro_data[2]) / 1000.0f;

    int16_t mag_data_i16[3] = { 0 };
    BSP_MAGNETO_GetXYZ(mag_data_i16);
    mag_mag = sqrtf((float)mag_data_i16[0]*mag_data_i16[0] + (float)mag_data_i16[1]*mag_data_i16[1] + (float)mag_data_i16[2]*mag_data_i16[2]) / 1000.0f;
}

void Telemetry_Send(void)
{
    char message_print[200];
    sprintf(message_print, "T: %.2f C, P: %.2f hPa, H: %.2f %%RH, A: %.2f m/s^2, G: %.2f dps, M: %.2f gauss\r\n",
            temp_data, pressure_data, humidity_data, accel_z, gyro_mag, mag_mag);
    UART_Print(message_print);
}

int Threshold_Check(void)
{
    char message_print[100];
    int violations = 0;

    if (temp_data < TEMP_LOW) {
        sprintf(message_print, "T: %.2f C, below threshold of %.2f C\r\n", temp_data, TEMP_LOW);
        UART_Print(message_print);
        violations++;
    }
    if (temp_data > TEMP_HIGH) {
        sprintf(message_print, "T: %.2f C, exceeds threshold of %.2f C\r\n", temp_data, TEMP_HIGH);
        UART_Print(message_print);
        violations++;
    }
    if (pressure_data < PRESSURE_LOW) {
        sprintf(message_print, "P: %.2f hPa, below threshold of %.2f hPa\r\n", pressure_data, PRESSURE_LOW);
        UART_Print(message_print);
        violations++;
    }
    if (pressure_data > PRESSURE_HIGH) {
        sprintf(message_print, "P: %.2f hPa, exceeds threshold of %.2f hPa\r\n", pressure_data, PRESSURE_HIGH);
        UART_Print(message_print);
        violations++;
    }
    if (humidity_data < HUMIDITY_LOW) {
        sprintf(message_print, "H: %.2f %%RH, below threshold of %.2f %%RH\r\n", humidity_data, HUMIDITY_LOW);
        UART_Print(message_print);
        violations++;
    }
    if (humidity_data > HUMIDITY_HIGH) {
        sprintf(message_print, "H: %.2f %%RH, exceeds threshold of %.2f %%RH\r\n", humidity_data, HUMIDITY_HIGH);
        UART_Print(message_print);
        violations++;
    }
    if (gyro_mag > GYRO_HIGH) {
        sprintf(message_print, "G: %.2f dps, exceeds threshold of %.2f dps\r\n", gyro_mag, GYRO_HIGH);
        UART_Print(message_print);
        violations++;
    }
    if (mag_mag > MAG_HIGH) {
        sprintf(message_print, "M: %.2f gauss, exceeds threshold of %.2f gauss\r\n", mag_mag, MAG_HIGH);
        UART_Print(message_print);
        violations++;
    }

    return violations;
}


int IsUpsideDown(void)
{
    return (accel_z < UPSIDE_DOWN_THRESHOLD) ? 1 : 0;
}
