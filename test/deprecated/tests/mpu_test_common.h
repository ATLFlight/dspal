/*==============================================================================
 Copyright (c) 2015 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Proprietary and Confidential.
 ==============================================================================*/

#ifndef MPU_TEST_COMMON_H
#define MPU_TEST_COMMON_H

#define SPI_DEV_PATH   "/dev/spi-1"  // BAM1 device

#define SPI_INT_GPIO   65  // GPIO device for MPU data ready interrupt

void calculate_stats(uint64_t *samples, uint64_t sample_cnt, float *avg,
                     float *stdev, uint64_t *min, uint64_t *max);

#endif // MPU_TEST_COMMON_H