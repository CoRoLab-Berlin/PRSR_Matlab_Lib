#define _POSIX_C_SOURCE 199309L

#include "main.h"
#include "bmi270.h"
#include "bmi270_config_file.h"

// -------------------------------------------------
// GLOBAL VARIABLES
// -------------------------------------------------

// for IMU
struct bmi270 sensor_upper = {.i2c_addr = I2C_PRIM_ADDR};
struct bmi270 sensor_lower = {.i2c_addr = I2C_SEC_ADDR};

// for package data
double temp_acc[3], temp_gyr[3];
struct timespec begin, data_timer, old_time1, old_time2;

// for timing
double UPDATE_RATE; // Hz       (Current Max: ~1000.0 Hz)
double UPDATE_TIME; // Seconds
struct timespec sleep_time;
struct timespec tic, toc;
double sleep_time_avg = 0.0;
double sleep_time_var = 0.0;
int i = 0;

// -------------------------------------------------
// HELPER FUNCTIONS
// -------------------------------------------------

double get_microseconds_delta(struct timespec *old_time, struct timespec *timer)
{
    double seconds = timer->tv_sec - old_time->tv_sec;
    double nanoseconds = timer->tv_nsec - old_time->tv_nsec;

    double elapsed_us = seconds * 1000000.0 + nanoseconds / 1000.0;

    return elapsed_us;
}

// -------------------------------------------------
// MAIN FUNCTIONS
// -------------------------------------------------

void imu_init(double UpdateRateInHertz)
{
    // -------------------------------------------------
    // UPDATE GLOBAL VARIABLES
    // -------------------------------------------------

    UPDATE_RATE = UpdateRateInHertz;
    UPDATE_TIME = (1.0 / UPDATE_RATE);

    struct timespec temp = {0, (long)(1.0 / UPDATE_RATE * 1000000000.0)};
    sleep_time = temp;

    // -------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------

    if (bmi270_init(&sensor_upper) == -1)
        printf("Failed to initialize sensor_upper. You might want to do a power cycle.\n");

    if (bmi270_init(&sensor_lower) == -1)
        printf("Failed to initialize sensor_lower. You might want to do a power cycle.\n");

    // -------------------------------------------------
    // HARDWARE CONFIGURATION
    // -------------------------------------------------

    set_mode(&sensor_upper, PERFORMANCE_MODE);
    set_acc_range(&sensor_upper, ACC_RANGE_2G);
    set_gyr_range(&sensor_upper, GYR_RANGE_1000);
    set_acc_bwp(&sensor_upper, ACC_BWP_OSR4);
    set_gyr_bwp(&sensor_upper, GYR_BWP_OSR4);
    disable_fifo_header(&sensor_upper);
    enable_data_streaming(&sensor_upper);
    enable_acc_filter_perf(&sensor_upper);
    enable_gyr_noise_perf(&sensor_upper);
    enable_gyr_filter_perf(&sensor_upper);

    set_mode(&sensor_lower, PERFORMANCE_MODE);
    set_acc_range(&sensor_lower, ACC_RANGE_2G);
    set_gyr_range(&sensor_lower, GYR_RANGE_1000);
    set_acc_bwp(&sensor_lower, ACC_BWP_OSR4);
    set_gyr_bwp(&sensor_lower, GYR_BWP_OSR4);
    disable_fifo_header(&sensor_lower);
    enable_data_streaming(&sensor_lower);
    enable_acc_filter_perf(&sensor_lower);
    enable_gyr_noise_perf(&sensor_lower);
    enable_gyr_filter_perf(&sensor_lower);

    // does not affect output frequency of timer in step() loop
    if (UPDATE_RATE <= 25.0)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_25);
        set_gyr_odr(&sensor_upper, GYR_ODR_25);
        set_acc_odr(&sensor_lower, ACC_ODR_25);
        set_gyr_odr(&sensor_lower, GYR_ODR_25);
    }
    else if (UPDATE_RATE <= 50.0)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_50);
        set_gyr_odr(&sensor_upper, GYR_ODR_50);
        set_acc_odr(&sensor_lower, ACC_ODR_50);
        set_gyr_odr(&sensor_lower, GYR_ODR_50);
    }
    else if (UPDATE_RATE <= 100)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_100);
        set_gyr_odr(&sensor_upper, GYR_ODR_100);
        set_acc_odr(&sensor_lower, ACC_ODR_100);
        set_gyr_odr(&sensor_lower, GYR_ODR_100);
    }
    else if (UPDATE_RATE <= 200)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_200);
        set_gyr_odr(&sensor_upper, GYR_ODR_200);
        set_acc_odr(&sensor_lower, ACC_ODR_200);
        set_gyr_odr(&sensor_lower, GYR_ODR_200);
    }
    else if (UPDATE_RATE <= 400)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_400);
        set_gyr_odr(&sensor_upper, GYR_ODR_400);
        set_acc_odr(&sensor_lower, ACC_ODR_400);
        set_gyr_odr(&sensor_lower, GYR_ODR_400);
    }
    else if (UPDATE_RATE <= 800)
    {
        set_acc_odr(&sensor_upper, ACC_ODR_800);
        set_gyr_odr(&sensor_upper, GYR_ODR_800);
        set_acc_odr(&sensor_lower, ACC_ODR_800);
        set_gyr_odr(&sensor_lower, GYR_ODR_800);
    }
    else
    {
        set_acc_odr(&sensor_upper, ACC_ODR_1600);
        set_gyr_odr(&sensor_upper, GYR_ODR_1600);
        set_acc_odr(&sensor_lower, ACC_ODR_1600);
        set_gyr_odr(&sensor_lower, GYR_ODR_1600);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
    old_time1 = begin;
    old_time2 = begin;
}

void step(double *imu_out)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &tic);

    // -------------------------------------------------
    // PACKAGE DATA
    // -------------------------------------------------

    clock_gettime(CLOCK_MONOTONIC_RAW, &data_timer);
    imu_out[0] = get_microseconds_delta(&old_time1, &data_timer); // Sensor 1 - Time
    old_time1 = data_timer;

    get_acc(&sensor_upper, &temp_acc[0], &temp_acc[1], &temp_acc[2]);
    get_gyr(&sensor_upper, &temp_gyr[0], &temp_gyr[1], &temp_gyr[2]);

    imu_out[1] = temp_acc[0];   // Sensor 1 - Acc X
    imu_out[2] = temp_acc[1];   // Sensor 1 - Acc Y
    imu_out[3] = temp_acc[2];   // Sensor 1 - Acc Z
    imu_out[4] = temp_gyr[0];   // Sensor 1 - Gyr X
    imu_out[5] = temp_gyr[1];   // Sensor 1 - Gyr Y
    imu_out[6] = temp_gyr[2];   // Sensor 1 - Gyr Z

    clock_gettime(CLOCK_MONOTONIC_RAW, &data_timer);
    imu_out[7] = get_microseconds_delta(&old_time2, &data_timer); // Sensor 2 - Time
    old_time2 = data_timer;

    get_acc(&sensor_lower, &temp_acc[0], &temp_acc[1], &temp_acc[2]);
    get_gyr(&sensor_lower, &temp_gyr[0], &temp_gyr[1], &temp_gyr[2]);

    imu_out[8]  = temp_acc[0];  // Sensor 2 - Acc X
    imu_out[9]  = temp_acc[1];  // Sensor 2 - Acc Y
    imu_out[10] = temp_acc[2];  // Sensor 2 - Acc Z
    imu_out[11] = temp_gyr[0];  // Sensor 2 - Gyr X
    imu_out[12] = temp_gyr[1];  // Sensor 2 - Gyr Y
    imu_out[13] = temp_gyr[2];  // Sensor 2 - Gyr Z

    // -------------------------------------------------
    // SLEEPING TO KEEP UPDATE RATE
    // -------------------------------------------------

    clock_gettime(CLOCK_MONOTONIC_RAW, &toc);
    double elapsed_time = (toc.tv_sec - tic.tv_sec) + (double)(toc.tv_nsec - tic.tv_nsec) / 1000000000.0;

    sleep_time_avg += (elapsed_time - sleep_time_avg) / (i + 1);
    sleep_time_var += (elapsed_time - sleep_time_avg) * (elapsed_time - sleep_time_avg) / (i + 1);
    i++;

    double adjustment = sqrt(sleep_time_var);

    sleep_time.tv_nsec = (long)((UPDATE_TIME - elapsed_time - adjustment) * 1000000000.0);

    if (i >= (UPDATE_RATE * 1.25))
    {
        sleep_time_avg = 0.0;
        sleep_time_var = 0.0;
        i = 0;
    }
    if (sleep_time.tv_nsec > 0)
        nanosleep(&sleep_time, NULL);
}
