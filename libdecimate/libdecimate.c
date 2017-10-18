/* 
 * This file is part of the libdecimate library
 *   (https://github.com/zindy/libdecimate)
 * Copyright (c) 2017 Egor Zindy
 * 
 * libdecimate is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * libdecimate is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#include <omp.h>
 
#include "libdecimate.h"

//takes three arrays (RGB) and a max value, returns a Nx3 list of unique RGB values
void decimate(
        uint16_t *ArrayR, int ZdimR, int YdimR, int XdimR,
        uint16_t *ArrayG, int ZdimG, int YdimG, int XdimG,
        uint16_t *ArrayB, int ZdimB, int YdimB, int XdimB,
        int input_bit, int output_bit,
        uint16_t **ArrayOut, int *YdimOut, int *XdimOut)
{
    uint64_t i,j,n=0;

    //output array
    uint16_t* decomp = NULL;

    int decimation = input_bit - output_bit;
    decimation = (decimation > 0)?decimation:0;

    uint64_t max_out = pow(2,output_bit)-1;
    uint64_t signature, temp_r, temp_g, temp_b, max_signature=0;

    //This is the truth array that holds all three RGB values
    uint8_t* truth = NULL;

    if ( ZdimR != ZdimG || ZdimR != ZdimB || YdimR != YdimG || YdimR != YdimB || XdimR != XdimG || XdimR != XdimB) {
        errno = E2BIG; goto end;
    }
    
    //The truth table needs to encompass all the values from 0 to 2**outputbits in all three dimensions.
    truth = (uint8_t *)calloc(pow(2,(output_bit*3)), sizeof(uint8_t));
    if (truth == NULL) { errno = ENOMEM; goto end; }

    //The strategy here is to fill truth array if we have a hit. Simultaneously, we could have multiple threads hitting the same
    //truth index. So don't count the number of hits as part of this loop, we can do that afterwards
    #pragma omp parallel for        \
        default(shared) private(temp_r, temp_g, temp_b, signature)

    for (i=0; i<ZdimR*YdimR*XdimR; i++)
    {
        temp_r = (uint64_t)ArrayR[i] >> decimation;
        if (temp_r > max_out) temp_r = max_out;

        temp_g = (uint64_t)ArrayG[i] >> decimation;
        if (temp_g > max_out) temp_g = max_out;

        temp_b = (uint64_t)ArrayB[i] >> decimation;
        if (temp_b > max_out) temp_b = max_out;

        //construct the rgb signature
        signature = temp_r | (temp_g << output_bit) | (temp_b << (output_bit*2));

        if (truth[signature] == 1)
            continue;

        if (signature > max_signature) {
            max_signature = signature;
        }

        //printf("signature=%d max_sig=%d\n",signature,max_signature);
        truth[signature] = 1;
    }

    //we need to count the total number of hits, this is to determine the size of the output array (nx3).
    n = 0;
    #pragma omp parallel for        \
        default(shared) reduction( + : n )

    for (i=0; i<=max_signature; i++) {
        if (truth[i] > 0)
            n++;
    }

    //output is 16 bit (up to)
    //could definitely combine these both into a single realloc. This just reminds me what is going on.
    if (*ArrayOut == NULL) {
        decomp = (uint16_t *)malloc(3*n*sizeof(uint16_t));
    } else {
        decomp = (uint16_t *)realloc(*ArrayOut, 3*n*sizeof(uint16_t));
    }
    if (decomp == NULL) { errno = ENOMEM; goto end; }

    //use this as a mask of width output_bit to unpack the index back into 3 r,g,b values
    signature = pow(2,output_bit) - 1;

    j = 0;
    for (i=0; i<=max_signature; i++)
    {
        if (truth[i] > 0)
        {
            //printf("i=%d j=%d truth[i]=%d\n",i,j,truth[i]);
            temp_r = i & signature;
            temp_g = (i >> output_bit) & signature;
            temp_b = (i >> (output_bit*2)) & signature;

            decomp[j++] = (uint16_t)temp_r;
            decomp[j++] = (uint16_t)temp_g;
            decomp[j++] = (uint16_t)temp_b;
            truth[i] = 0;
        }
    }

end:
    if (truth != NULL) free(truth);

    *ArrayOut = decomp;
    *YdimOut = n;
    *XdimOut = 3;
}

//This re
void decimate_indexes(
        uint16_t *ArrayR, int ZdimR, int YdimR, int XdimR,
        uint16_t *ArrayG, int ZdimG, int YdimG, int XdimG,
        uint16_t *ArrayB, int ZdimB, int YdimB, int XdimB,
        int input_bit, int output_bit,
        uint64_t **ArrayOut, int *NdimOut)
{
    uint64_t i,j,n=0;

    //output array
    uint64_t* decomp = NULL;

    int decimation = input_bit - output_bit;
    decimation = (decimation > 0)?decimation:0;

    uint64_t max_out = pow(2,output_bit)-1;
    uint64_t signature, temp_r, temp_g, temp_b, max_signature=0;

    //This is the truth array that holds all three RGB values
    uint8_t* truth = NULL;
    uint64_t* truth_index = NULL;

    if ( ZdimR != ZdimG || ZdimR != ZdimB || YdimR != YdimG || YdimR != YdimB || XdimR != XdimG || XdimR != XdimB) {
        errno = E2BIG; goto end;
    }
    
    //The truth table needs to encompass all the values from 0 to 2**outputbits in all three dimensions.
    truth = (uint8_t *)calloc(pow(2,(output_bit*3)), sizeof(uint8_t));
    truth_index = (uint64_t *)malloc(pow(2,(output_bit*3))*sizeof(uint64_t));
    if (truth == NULL || truth_index == NULL) { errno = ENOMEM; goto end; }

    //The strategy here is to fill truth array if we have a hit. Simultaneously, we could have multiple threads hitting the same
    //truth index. So don't count the number of hits as part of this loop, we can do that afterwards
    #pragma omp parallel for        \
        default(shared) private(temp_r, temp_g, temp_b, signature)

    for (i=0; i<ZdimR*YdimR*XdimR; i++)
    {
        temp_r = (uint64_t)ArrayR[i] >> decimation;
        if (temp_r > max_out) temp_r = max_out;

        temp_g = (uint64_t)ArrayG[i] >> decimation;
        if (temp_g > max_out) temp_g = max_out;

        temp_b = (uint64_t)ArrayB[i] >> decimation;
        if (temp_b > max_out) temp_b = max_out;

        //construct the rgb signature
        signature = temp_r | (temp_g << output_bit) | (temp_b << (output_bit*2));

        if (truth[signature] == 1)
            continue;

        if (signature > max_signature) {
            max_signature = signature;
        }

        truth[signature] = 1;
        truth_index[signature] = i;
        //printf("signature=%d max_sig=%d i=%d truth_index[signature]=%d\n",signature,max_signature,i, truth_index[signature]);
    }

    //we need to count the total number of hits, this is to determine the size of the output array (nx3).
    n = 0;
    #pragma omp parallel for        \
        default(shared) reduction( + : n )

    for (i=0; i<=max_signature; i++) {
        if (truth[i] > 0)
            n++;
    }

    //output is 16 bit (up to)
    //could definitely combine these both into a single realloc. This just reminds me what is going on.
    if (*ArrayOut == NULL) {
        decomp = (uint64_t *)malloc(n*sizeof(uint64_t));
    } else {
        decomp = (uint64_t *)realloc(*ArrayOut, n*sizeof(uint64_t));
    }
    if (decomp == NULL) { errno = ENOMEM; goto end; }

    j = 0;
    for (i=0; i<=max_signature; i++)
    {
        if (truth[i] > 0)
        {
            //printf("i=%d j=%d truth[i]=%d truth_index[i]=%d\n",i,j,truth[i],truth_index[i]);
            decomp[j] = truth_index[i];
            truth[i] = 0;
            j++;
        }

    }

end:
    if (truth != NULL) free(truth);
    if (truth_index != NULL) free(truth_index);

    *ArrayOut = decomp;
    *NdimOut = n;
}

