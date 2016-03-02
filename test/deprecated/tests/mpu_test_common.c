/*==============================================================================
 Copyright (c) 2015 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Proprietary and Confidential.
 ==============================================================================*/
#include <stdint.h>
#include <math.h>
#include "mpu_test_common.h"

void calculate_stats(uint64_t *samples, uint64_t sample_cnt, float *avg,
                     float *stdev, uint64_t *min, uint64_t *max)
{
   int i;
   uint64_t sum = 0;
   *min = 0xFFFFFFFF;
   *max = 0;

   for (i = 0; i < sample_cnt; i++)
   {
      sum += samples[i];
      if (*min > samples[i])
         *min = samples[i];
      if (*max < samples[i])
         *max = samples[i];
   }

   *avg = (float)sum / sample_cnt;

   for (i = 0; i < sample_cnt; i++)
   {
      sum += (samples[i] - *avg)*(samples[i] - *avg);
   }

   *stdev = sqrt((float)sum/(float)(sample_cnt-1));
}

