#pragma once

//
//  hw_spi.hpp
//
//
//  Created by Manuel on 26/2/20.
//

#ifndef hw_spi_h
#define hw_spi_h


#if (defined(MATLAB_MEX_FILE) || defined(RSIM_PARAMETER_LOADING) || defined(RSIM_WITH_SL_SOLVER))
#else
#include <stdint.h> //_t type ending
#include <linux/spi/spidev.h>
#include <stdio.h> //err out
#include <fcntl.h> //open RDWR
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h> //close socket
#include <time.h>

#define VOUT_LENGTH 4
#define VIN_LEN 8
#define LTC_WRITE 0b0011

void spi_init();
void getADC(double *vin);
void setDAC(double vout[VOUT_LENGTH]);

#endif
#endif /* hw_spi_h */
