//
//  hw_spi.cpp
//
//
//  Created by Manuel on 26/2/20.
//

#include "hw_spi.h"

int fdADC;
int fdDAC;

struct spi_ioc_transfer dacDataframe;
struct spi_ioc_transfer adcDataframe;
uint8_t adcReadCode[8 + 1];

void spi_init()
{
    int SPI_SPEED = 5000000;
    int SPI_MODE = 0;
    static const char *SPI_DAC_DEV = "/dev/spidev0.1";
    static const char *SPI_ADC_DEV = "/dev/spidev0.0";
    // configuration spi_ioc_transfer DAC
    memset(&dacDataframe, 0, sizeof(dacDataframe));
    dacDataframe.len = 3;
    dacDataframe.speed_hz = SPI_SPEED;
    dacDataframe.bits_per_word = 8;

    // configuration spi_ioc_transfer ADC
    memset(&adcDataframe, 0, sizeof(adcDataframe));
    adcDataframe.len = 2;
    adcDataframe.speed_hz = SPI_SPEED;
    adcDataframe.bits_per_word = 8;

    // open sockets ADC & DAC
    fdDAC = open(SPI_DAC_DEV, O_RDWR);
    ioctl(fdDAC, SPI_IOC_WR_MODE, SPI_MODE);
    ioctl(fdDAC, SPI_IOC_RD_MAX_SPEED_HZ, SPI_SPEED);

    fdADC = open(SPI_ADC_DEV, O_RDWR);
    ioctl(fdADC, SPI_IOC_WR_MODE, SPI_MODE);
    ioctl(fdADC, SPI_IOC_RD_MAX_SPEED_HZ, SPI_SPEED);

    // codes to read ADC channels
    adcReadCode[0] = 0b10000100;
    adcReadCode[1] = 0b11000100;
    adcReadCode[2] = 0b10010100;
    adcReadCode[3] = 0b11010100;
    adcReadCode[4] = 0b10100100;
    adcReadCode[5] = 0b11100100;
    adcReadCode[6] = 0b10110100;
    adcReadCode[7] = 0b11110100;
    adcReadCode[8] = 0b10000100;
}

void setDAC(double vout[VOUT_LENGTH])
{
    // DAC LTC1859 12bit
    uint16_t writeBuffer = 0;
    unsigned char txbuf[3];
    unsigned char rxbuf[3];
    memset(txbuf, 0, sizeof txbuf);
    memset(rxbuf, 0, sizeof rxbuf);
    dacDataframe.tx_buf = (unsigned long)txbuf;
    dacDataframe.rx_buf = (unsigned long)rxbuf;
    for (int i = 0; i < VOUT_LENGTH; i++)
    {
        writeBuffer = (int)(vout[i] * 65535 / 10);
        txbuf[0] = ((LTC_WRITE << 4) | i);
        txbuf[1] = (writeBuffer >> 8);
        txbuf[2] = (writeBuffer & 0xFF);

        ioctl(fdDAC, SPI_IOC_MESSAGE(1), &dacDataframe);
    }
}

void getADC(double *vin)
{

    int16_t readBuffer = 0;
    unsigned char txbuf[2];
    unsigned char rxbuf[2];
    memset(txbuf, 0, sizeof(txbuf));
    memset(rxbuf, 0, sizeof(rxbuf));
    adcDataframe.tx_buf = (unsigned long)txbuf;
    adcDataframe.rx_buf = (unsigned long)rxbuf;

    // register configuration for adc port 0
    txbuf[0] = adcReadCode[0];
    ioctl(fdADC, SPI_IOC_MESSAGE(1), &adcDataframe);

    for (int i = 1; i < VIN_LEN + 1; i++)
    {
        txbuf[0] = adcReadCode[i];
        ioctl(fdADC, SPI_IOC_MESSAGE(1), &adcDataframe);
        readBuffer = rxbuf[0] << 8 | rxbuf[1];
        vin[i - 1] = (double)readBuffer * 10 / 32767;
    }
}
