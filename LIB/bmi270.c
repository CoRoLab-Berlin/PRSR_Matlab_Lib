#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "bmi270.h"

extern const uint8_t bmi270_config_file[];

/* ----------------------------------------------------
                     FUNCTIONS
-----------------------------------------------------*/

void print_binary(uint8_t num)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (num >> i) & 1);
    }
    printf("\n");
}

uint8_t read_register(struct bmi270 *sensor, uint8_t reg_addr)
{
    uint8_t result = 0;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg i2c_msg[2];
    char write_buf[1] = {reg_addr};
    char read_buf[1] = {0};

    // Setup I2C write operation to send register address to device
    i2c_msg[0].addr = sensor->i2c_addr;
    i2c_msg[0].flags = 0;
    i2c_msg[0].len = sizeof(write_buf);
    i2c_msg[0].buf = write_buf;

    // Setup I2C read operation to read data from device
    i2c_msg[1].addr = sensor->i2c_addr;
    i2c_msg[1].flags = I2C_M_RD;
    i2c_msg[1].len = sizeof(read_buf);
    i2c_msg[1].buf = read_buf;

    // Setup I2C data struct for ioctl
    i2c_data.msgs = i2c_msg;
    i2c_data.nmsgs = 2;

    // Send I2C transaction to read data from register
    if (ioctl(sensor->i2c_fd, I2C_RDWR, &i2c_data) < 0)
    {
        printf("0x%X --> Failed to read from I2C device\n", sensor->i2c_addr);
        return -1;
    }

    // Combine the read buffer to an integer value
    result = (uint8_t)read_buf[0];

    return result;
}

int read_register_block(struct bmi270 *sensor, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg i2c_msg[2];
    char write_buf[1] = {reg_addr};

    // Setup I2C write operation to send register address to device
    i2c_msg[0].addr = sensor->i2c_addr;
    i2c_msg[0].flags = 0;
    i2c_msg[0].len = sizeof(write_buf);
    i2c_msg[0].buf = write_buf;

    // Setup I2C read operation to read data from device
    i2c_msg[1].addr = sensor->i2c_addr;
    i2c_msg[1].flags = I2C_M_RD;
    i2c_msg[1].len = len;
    i2c_msg[1].buf = data;

    // Setup I2C data struct for ioctl
    i2c_data.msgs = i2c_msg;
    i2c_data.nmsgs = 2;

    // Send I2C transaction to read data from registers
    if (ioctl(sensor->i2c_fd, I2C_RDWR, &i2c_data) < 0)
    {
        printf("0x%X --> Failed to read from I2C device\n", sensor->i2c_addr);
        return -1;
    }

    return 0;
}

int write_register(struct bmi270 *sensor, uint8_t reg_addr, uint8_t value)
{
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg i2c_msg[1];
    char write_buf[2] = {reg_addr, value};

    // Setup I2C write operation to send register address and data to device
    i2c_msg[0].addr = sensor->i2c_addr;
    i2c_msg[0].flags = 0;
    i2c_msg[0].len = sizeof(write_buf);
    i2c_msg[0].buf = write_buf;

    // Setup I2C data struct for ioctl
    i2c_data.msgs = i2c_msg;
    i2c_data.nmsgs = 1;

    // Send I2C transaction to write data to register
    if (ioctl(sensor->i2c_fd, I2C_RDWR, &i2c_data) < 0)
    {
        printf("0x%X --> Failed to write to I2C device\n", sensor->i2c_addr);
        return -1;
    }

    return 0;
}

int write_register_block(struct bmi270 *sensor, uint8_t reg_addr, uint8_t len, const uint8_t *data)
{
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg i2c_msg[1];
    char write_buf[33] = {reg_addr};

    for (int i = 0; i < len; i++)
    {
        write_buf[i + 1] = data[i];
    }

    // Setup I2C write operation to send register address and data to device
    i2c_msg[0].addr = sensor->i2c_addr;
    i2c_msg[0].flags = 0;
    i2c_msg[0].len = len + 1;
    i2c_msg[0].buf = write_buf;

    // Setup I2C data struct for ioctl
    i2c_data.msgs = i2c_msg;
    i2c_data.nmsgs = 1;

    // Send I2C transaction to write data to register
    if (ioctl(sensor->i2c_fd, I2C_RDWR, &i2c_data) < 0)
    {
        printf("0x%X --> Failed to write to I2C device\n", sensor->i2c_addr);
        return -1;
    }

    return 0;
}

int load_config_file(struct bmi270 *sensor)
{
    sensor->internal_status = read_register(sensor, INTERNAL_STATUS);

    if (sensor->internal_status & 0x01)
    {
        printf("0x%x --> Already initialized!\n", sensor->i2c_addr);
    }
    else if (sensor->internal_status & 0x02)
    {
        printf("0x%x --> IMPORTANT: Sensor needs power cycle!\n", sensor->i2c_addr);
    }
    else
    {
        printf("0x%x --> Initializing...\n", sensor->i2c_addr);
        write_register(sensor, PWR_CONF, 0x00);
        usleep(450);
        write_register(sensor, INIT_CTRL, 0x00);

        for (int i = 0; i < 256; i++)
        {
            write_register(sensor, INIT_ADDR_0, 0x00);
            write_register(sensor, INIT_ADDR_1, i);
            write_register_block(sensor, INIT_DATA, 32, &bmi270_config_file[i * 32]);
            usleep(20);
        }

        write_register(sensor, INIT_CTRL, 0x01);
        usleep(20000);

        sensor->internal_status = read_register(sensor, INTERNAL_STATUS);
    }

    printf("0x%x --> Initialization status: %x (1 --> OK)\n", sensor->i2c_addr, sensor->internal_status);
}

int bmi270_init(struct bmi270 *sensor)
{
    // Open I2C bus
    if ((sensor->i2c_fd = open(I2C_DEVICE, O_RDWR)) < 0)
    {
        printf("Error: Could not open I2C bus for %s\n", I2C_DEVICE);
        return -1;
    }

    // Set I2C address
    if (ioctl(sensor->i2c_fd, I2C_SLAVE, sensor->i2c_addr) < 0)
    {
        printf("Error: Could not set I2C address to 0x%X\n", sensor->i2c_addr);
        close(sensor->i2c_fd);
        return -1;
    }

    printf("0x%X --> I2C setup successfull!\n", sensor->i2c_addr);

    // Check if chip ID matches
    if ((sensor->chip_id = read_register(sensor, CHIP_ID_ADDRESS)) != BMI270_CHIP_ID)
    {
        printf("Error: Chip ID mismatch! --> CURRENT: 0x%X - BMI270: 0x%X\n", sensor->chip_id, BMI270_CHIP_ID);
        return -1;
    }

    printf("0x%X --> Chip ID: 0x%X\n", sensor->i2c_addr, sensor->chip_id);

    load_config_file(sensor);

    return 0;
}

void set_mode(struct bmi270 *sensor, uint8_t mode)
{
    if (mode == LOW_POWER_MODE)
    {

        write_register(sensor, PWR_CTRL, 0x04);
        write_register(sensor, ACC_CONF, 0x17);
        write_register(sensor, GYR_CONF, 0x28);
        write_register(sensor, PWR_CONF, 0x03);
        sensor->acc_odr = 50;
        sensor->gyr_odr = 100;
        printf("0x%X --> Mode set to: LOW_POWER_MODE\n", sensor->i2c_addr);
    }
    else if (mode == NORMAL_MODE)
    {
        write_register(sensor, PWR_CTRL, 0x0E);
        write_register(sensor, ACC_CONF, 0xA8);
        write_register(sensor, GYR_CONF, 0xA9);
        write_register(sensor, PWR_CONF, 0x02);
        sensor->acc_odr = 100;
        sensor->gyr_odr = 200;
        printf("0x%X --> Mode set to: NORMAL_MODE\n", sensor->i2c_addr);
    }
    else if (mode == PERFORMANCE_MODE)
    {
        write_register(sensor, PWR_CTRL, 0x0E);
        write_register(sensor, ACC_CONF, 0xA8);
        write_register(sensor, GYR_CONF, 0xE9);
        write_register(sensor, PWR_CONF, 0x02);
        sensor->acc_odr = 100;
        sensor->gyr_odr = 200;
        printf("0x%X --> Mode set to: PERFORMANCE_MODE\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong sensor mode. Use 'LOWER_POWER_MODE', 'NORMAL_MODE' or 'PERFORMANCE_MODE'\n", sensor->i2c_addr);
}

void enable_aux(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) | BIT_0));
    printf("0x%X --> Auxiliary sensor enabled\n", sensor->i2c_addr);
}

void disable_aux(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) & ~BIT_0));
    printf("0x%X --> Auxiliary sensor disabled\n", sensor->i2c_addr);
}

void enable_acc(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) | BIT_1));
    printf("0x%X --> Accelerometer enabled\n", sensor->i2c_addr);
}

void disable_acc(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) & ~BIT_1));
    printf("0x%X --> Accelerometer disabled\n", sensor->i2c_addr);
}

void enable_gyr(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) | BIT_2));
    printf("0x%X --> Gyroscope enabled\n", sensor->i2c_addr);
}

void disable_gyr(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) & ~BIT_2));
    printf("0x%X --> Gyroscope disabled\n", sensor->i2c_addr);
}

void enable_temp(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) | BIT_3));
    printf("0x%X --> Temperature sensor enabled\n", sensor->i2c_addr);
}

void disable_temp(struct bmi270 *sensor)
{
    write_register(sensor, PWR_CTRL, (read_register(sensor, PWR_CTRL) & ~BIT_3));
    printf("0x%X --> Temperature sensor disabled\n", sensor->i2c_addr);
}

void set_acc_range(struct bmi270 *sensor, uint8_t range)
{
    if (range == ACC_RANGE_2G)
    {
        write_register(sensor, ACC_RANGE, ACC_RANGE_2G);
        sensor->acc_range = 2.0 * GRAVITY;
        printf("0x%X --> ACC range set to: 2G\n", sensor->i2c_addr);
    }
    else if (range == ACC_RANGE_4G)
    {
        write_register(sensor, ACC_RANGE, ACC_RANGE_4G);
        sensor->acc_range = 4.0 * GRAVITY;
        printf("0x%X --> ACC range set to: 4G\n", sensor->i2c_addr);
    }
    else if (range == ACC_RANGE_8G)
    {
        write_register(sensor, ACC_RANGE, ACC_RANGE_8G);
        sensor->acc_range = 8.0 * GRAVITY;
        printf("0x%X --> ACC range set to: 8G\n", sensor->i2c_addr);
    }
    else if (range == ACC_RANGE_16G)
    {
        write_register(sensor, ACC_RANGE, ACC_RANGE_16G);
        sensor->acc_range = 16.0 * GRAVITY;
        printf("0x%X --> ACC range set to: 16G\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong ACC range. Use 'ACC_RANGE_2G', 'ACC_RANGE_4G', 'ACC_RANGE_8G' or 'ACC_RANGE_16G'\n", sensor->i2c_addr);
}

void set_gyr_range(struct bmi270 *sensor, uint8_t range)
{
    if (range == GYR_RANGE_2000)
    {
        write_register(sensor, GYR_RANGE, GYR_RANGE_2000);
        sensor->gyr_range = 2000.0 * DEG2RAD;
        printf("0x%X --> GYR range set to: 2000 DPS\n", sensor->i2c_addr);
    }
    else if (range == GYR_RANGE_1000)
    {
        write_register(sensor, GYR_RANGE, GYR_RANGE_1000);
        sensor->gyr_range = 1000.0 * DEG2RAD;
        printf("0x%X --> GYR range set to: 1000 DPS\n", sensor->i2c_addr);
    }
    else if (range == GYR_RANGE_500)
    {
        write_register(sensor, GYR_RANGE, GYR_RANGE_500);
        sensor->gyr_range = 500.0 * DEG2RAD;
        printf("0x%X --> GYR range set to: 500 DPS\n", sensor->i2c_addr);
    }
    else if (range == GYR_RANGE_250)
    {
        write_register(sensor, GYR_RANGE, GYR_RANGE_250);
        sensor->gyr_range = 250.0 * DEG2RAD;
        printf("0x%X --> GYR range set to: 250 DPS\n", sensor->i2c_addr);
    }
    else if (range == GYR_RANGE_125)
    {
        write_register(sensor, GYR_RANGE, GYR_RANGE_125);
        sensor->gyr_range = 125.0 * DEG2RAD;
        printf("0x%X --> GYR range set to: 125 DPS\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong GYR range. Use 'GYR_RANGE_2000', 'GYR_RANGE_1000', 'GYR_RANGE_500', 'GYR_RANGE_250' or 'GYR_RANGE_125'\n", sensor->i2c_addr);
}

void set_acc_odr(struct bmi270 *sensor, uint8_t odr)
{
    if (odr == ACC_ODR_1600)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_1600));
        sensor->acc_odr = 1600;
        printf("0x%X --> ACC ODR set to: 1600 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_800)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_800));
        sensor->acc_odr = 800;
        printf("0x%X --> ACC ODR set to: 800 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_400)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_400));
        sensor->acc_odr = 400;
        printf("0x%X --> ACC ODR set to: 400 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_200)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_200));
        sensor->acc_odr = 200;
        printf("0x%X --> ACC ODR set to: 200 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_100)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_100));
        sensor->acc_odr = 100;
        printf("0x%X --> ACC ODR set to: 100 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_50)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_50));
        sensor->acc_odr = 50;
        printf("0x%X --> ACC ODR set to: 50 Hz\n", sensor->i2c_addr);
    }
    else if (odr == ACC_ODR_25)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & MSB_MASK_8BIT) | ACC_ODR_25));
        sensor->acc_odr = 25;
        printf("0x%X --> ACC ODR set to: 25 Hz\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong ACC ODR. Use 'ACC_ODR_1600', 'ACC_ODR_800', 'ACC_ODR_400', 'ACC_ODR_200', 'ACC_ODR_100', 'ACC_ODR_50' or 'ACC_ODR_25'\n", sensor->i2c_addr);
}

void set_gyr_odr(struct bmi270 *sensor, uint8_t odr)
{
    if (odr == GYR_ODR_3200)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_3200));
        sensor->gyr_odr = 3200;
        printf("0x%X --> GYR ODR set to: 3200 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_1600)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_1600));
        sensor->gyr_odr = 1600;
        printf("0x%X --> GYR ODR set to: 1600 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_800)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_800));
        sensor->gyr_odr = 800;
        printf("0x%X --> GYR ODR set to: 800 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_400)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_400));
        sensor->gyr_odr = 400;
        printf("0x%X --> GYR ODR set to: 400 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_200)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_200));
        sensor->gyr_odr = 200;
        printf("0x%X --> GYR ODR set to: 200 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_100)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_100));
        sensor->gyr_odr = 100;
        printf("0x%X --> GYR ODR set to: 100 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_50)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_50));
        sensor->gyr_odr = 50;
        printf("0x%X --> GYR ODR set to: 50 Hz\n", sensor->i2c_addr);
    }
    else if (odr == GYR_ODR_25)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & MSB_MASK_8BIT) | GYR_ODR_25));
        sensor->gyr_odr = 25;
        printf("0x%X --> GYR ODR set to: 25 Hz\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong GYR ODR. Use 'GYR_ODR_3200', 'GYR_ODR_1600', 'GYR_ODR_800', 'GYR_ODR_400', 'GYR_ODR_200', 'GYR_ODR_100', 'GYR_ODR_50' or 'GYR_ODR_25'\n", sensor->i2c_addr);
}

void set_acc_bwp(struct bmi270 *sensor, uint8_t bwp)
{
    if (bwp == ACC_BWP_OSR4)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_OSR4 << 4)));
        printf("0x%X --> ACC BWP set to: OSR4\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_OSR2)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_OSR2 << 4)));
        printf("0x%X --> ACC BWP set to: OSR2\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_NORMAL)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_NORMAL << 4)));
        printf("0x%X --> ACC BWP set to: NORMAL\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_CIC)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_CIC << 4)));
        printf("0x%X --> ACC BWP set to: CIC\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_RES16)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_RES16 << 4)));
        printf("0x%X --> ACC BWP set to: RES16\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_RES32)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_RES32 << 4)));
        printf("0x%X --> ACC BWP set to: RES32\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_RES64)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_RES64 << 4)));
        printf("0x%X --> ACC BWP set to: RES64\n", sensor->i2c_addr);
    }
    else if (bwp == ACC_BWP_RES128)
    {
        write_register(sensor, ACC_CONF, ((read_register(sensor, ACC_CONF) & LSB_MASK_8BIT_8) | (ACC_BWP_RES128 << 4)));
        printf("0x%X --> ACC BWP set to: RES128\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong ACC BWP. Use 'ACC_BWP_OSR4', 'ACC_BWP_OSR2', 'ACC_BWP_NORMAL', 'ACC_BWP_CIC', 'ACC_BWP_RES16', 'ACC_BWP_RES32', 'ACC_BWP_RES64' or 'ACC_BWP_RES128'\n", sensor->i2c_addr);
}

void set_gyr_bwp(struct bmi270 *sensor, uint8_t bwp)
{
    if (bwp == GYR_BWP_OSR4)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & LSB_MASK_8BIT_8) | (GYR_BWP_OSR4 << 4)));
        printf("0x%X --> GYR BWP set to: OSR4\n", sensor->i2c_addr);
    }
    else if (bwp == GYR_BWP_OSR2)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & LSB_MASK_8BIT_8) | (GYR_BWP_OSR2 << 4)));
        printf("0x%X --> GYR BWP set to: OSR2\n", sensor->i2c_addr);
    }
    else if (bwp == GYR_BWP_NORMAL)
    {
        write_register(sensor, GYR_CONF, ((read_register(sensor, GYR_CONF) & LSB_MASK_8BIT_8) | (GYR_BWP_NORMAL << 4)));
        printf("0x%X --> GYR BWP set to: NORMAL\n", sensor->i2c_addr);
    }
    else
        printf("0x%X --> Wrong GYR BWP. Use 'GYR_BWP_OSR4', 'GYR_BWP_OSR2' or 'GYR_BWP_NORMAL'\n", sensor->i2c_addr);
}

void enable_fifo_header(struct bmi270 *sensor)
{
    write_register(sensor, FIFO_CONFIG_1, (read_register(sensor, FIFO_CONFIG_1) | BIT_4));
    printf("0x%X --> FIFO header enabled\n", sensor->i2c_addr);
}

void disable_fifo_header(struct bmi270 *sensor)
{
    write_register(sensor, FIFO_CONFIG_1, (read_register(sensor, FIFO_CONFIG_1) & ~BIT_4));
    printf("0x%X --> FIFO Header disabled (ODR of all enabled sensors need to be identical)\n", sensor->i2c_addr);
}

void enable_data_streaming(struct bmi270 *sensor)
{
    write_register(sensor, FIFO_CONFIG_1, (read_register(sensor, FIFO_CONFIG_1) | LAST_3_BITS));
    printf("0x%X --> Data streaming mode enabled (no data will be stored in FIFO)\n", sensor->i2c_addr);
}

void disable_data_streaming(struct bmi270 *sensor)
{
    write_register(sensor, FIFO_CONFIG_1, (read_register(sensor, FIFO_CONFIG_1) & ~LAST_3_BITS));
    printf("0x%X --> Data streaming mode disabled (data will be stored in FIFO)\n", sensor->i2c_addr);
}

void enable_acc_filter_perf(struct bmi270 *sensor)
{
    write_register(sensor, ACC_CONF, (read_register(sensor, ACC_CONF) | BIT_7));
    printf("0x%X --> ACC filter performance enabled (performance optimized)\n", sensor->i2c_addr);
}

void disable_acc_filter_perf(struct bmi270 *sensor)
{
    write_register(sensor, ACC_CONF, (read_register(sensor, ACC_CONF) & ~BIT_7));
    printf("0x%X --> ACC filter performance disabled (power optimized)\n", sensor->i2c_addr);
}

void enable_gyr_noise_perf(struct bmi270 *sensor)
{
    write_register(sensor, GYR_CONF, (read_register(sensor, GYR_CONF) | BIT_6));
    printf("0x%X --> GYR noise performance enabled (performance optimized)\n", sensor->i2c_addr);
}

void disable_gyr_noise_perf(struct bmi270 *sensor)
{
    write_register(sensor, GYR_CONF, (read_register(sensor, GYR_CONF) & ~BIT_6));
    printf("0x%X --> GYR noise performance disabled (power optimized)\n", sensor->i2c_addr);
}

void enable_gyr_filter_perf(struct bmi270 *sensor)
{
    write_register(sensor, GYR_CONF, (read_register(sensor, GYR_CONF) | BIT_7));
    printf("0x%X --> GYR filter performance enabled (performance optimized)\n", sensor->i2c_addr);
}

void disable_gyr_filter_perf(struct bmi270 *sensor)
{
    write_register(sensor, GYR_CONF, (read_register(sensor, GYR_CONF) & ~BIT_7));
    printf("0x%X --> GYR filter performance disabled (power optimized)\n", sensor->i2c_addr);
}

void get_acc_raw(struct bmi270 *sensor, int16_t *acc_x_raw, int16_t *acc_y_raw, int16_t *acc_z_raw)
{
    uint8_t buffer[6];

    read_register_block(sensor, ACC_X_7_0, (uint8_t *)&buffer, 6);

    *acc_x_raw = (buffer[1] << 8) | buffer[0];
    *acc_y_raw = (buffer[3] << 8) | buffer[2];
    *acc_z_raw = (buffer[5] << 8) | buffer[4];
}

void get_gyr_raw(struct bmi270 *sensor, int16_t *gyr_x_raw, int16_t *gyr_y_raw, int16_t *gyr_z_raw)
{
    uint8_t buffer[6];

    read_register_block(sensor, GYR_X_7_0, (uint8_t *)&buffer, 6);

    *gyr_x_raw = (buffer[1] << 8) | buffer[0];
    *gyr_y_raw = (buffer[3] << 8) | buffer[2];
    *gyr_z_raw = (buffer[5] << 8) | buffer[4];
}

void get_temp_raw(struct bmi270 *sensor, int16_t *temp)
{
    uint8_t buffer[2];

    read_register_block(sensor, TEMP_7_0, (uint8_t *)&buffer, 2);

    *temp = (buffer[1] << 8) | buffer[0];
}

void get_acc(struct bmi270 *sensor, double *acc_x, double *acc_y, double *acc_z)
{
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;

    get_acc_raw(sensor, &acc_x_raw, &acc_y_raw, &acc_z_raw);

    *acc_x = (double)acc_x_raw * sensor->acc_range / 32768.0;
    *acc_y = (double)acc_y_raw * sensor->acc_range / 32768.0;
    *acc_z = (double)acc_z_raw * sensor->acc_range / 32768.0;
}

void get_gyr(struct bmi270 *sensor, double *gyr_x, double *gyr_y, double *gyr_z)
{
    int16_t gyr_x_raw, gyr_y_raw, gyr_z_raw;

    get_gyr_raw(sensor, &gyr_x_raw, &gyr_y_raw, &gyr_z_raw);

    *gyr_x = (double)gyr_x_raw * sensor->gyr_range / 32768.0;
    *gyr_y = (double)gyr_y_raw * sensor->gyr_range / 32768.0;
    *gyr_z = (double)gyr_z_raw * sensor->gyr_range / 32768.0;
}

void get_temp(struct bmi270 *sensor, double *temp_celsius)
{
    int16_t temp_raw;

    get_temp_raw(sensor, &temp_raw);

    *temp_celsius = (double)temp_raw * 0.001952594 + 23.0;
}