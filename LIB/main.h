#pragma once

#ifndef MAIN_H
#define MAIN_H

#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#include "hw_spi.h"

int32_t get_microseconds_delta(struct timespec *old_time, struct timespec *timer);

void imu_init(double UpdateRateInHertz);

void step(int32_t *imu_out, double vout[VOUT_LENGTH], double* spi_out);

#endif // MAIN_H