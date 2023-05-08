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

double get_microseconds_delta(struct timespec *old_time, struct timespec *timer);

void imu_init(double UpdateRateInHertz);

void step(double *imu_out);

#endif // MAIN_H