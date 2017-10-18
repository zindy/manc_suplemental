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

%module libdecimate
%{
#include <errno.h>
#include <stdint.h>
#include "libdecimate.h"

#define SWIG_FILE_WITH_INIT
%}

%include "numpy_mod.i"
%include "exception.i"

%init %{
    import_array();
%}

%exception
{
    errno = 0;
    $action

    if (errno != 0)
    {
        switch(errno)
        {
            case E2BIG:
                PyErr_Format(PyExc_ValueError, "All three images must have the same dimensions");
                break;
            case ENOMEM:
                PyErr_Format(PyExc_MemoryError, "Not enough memory");
                break;
            case EPERM:
                PyErr_Format(PyExc_IndexError, "Unknown index value");
                break;
            default:
                PyErr_Format(PyExc_Exception, "Unknown exception");
        }
        SWIG_fail;
    }
}

%apply (uint16_t* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {(uint16_t *ArrayR, int ZdimR, int YdimR, int XdimR)}
%apply (uint16_t* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {(uint16_t *ArrayG, int ZdimG, int YdimG, int XdimG)}
%apply (uint16_t* IN_ARRAY3, int DIM1, int DIM2, int DIM3) {(uint16_t *ArrayB, int ZdimB, int YdimB, int XdimB)}
%apply (uint16_t** ARGOUTVIEWM_ARRAY2, int* DIM1, int* DIM2) {(uint16_t **ArrayOut, int *YdimOut, int *XdimOut)}
%apply (uint64_t** ARGOUTVIEWM_ARRAY1, int* DIM1) {(uint64_t **ArrayOut, int *NdimOut)}

%rename (decimate) decimate_safe;
%rename (decimate_indexes) decimate_indexes_safe;

%inline %{

void decimate_safe(
        uint16_t *ArrayR, int ZdimR, int YdimR, int XdimR,
        uint16_t *ArrayG, int ZdimG, int YdimG, int XdimG,
        uint16_t *ArrayB, int ZdimB, int YdimB, int XdimB,
        int input_bit, int output_bit,
        uint16_t **ArrayOut, int *YdimOut, int *XdimOut)
{

    int _ZdimR = (ZdimR == -1)?1:ZdimR;
    int _YdimR = (YdimR == -1)?1:YdimR;
    int _ZdimG = (ZdimG == -1)?1:ZdimG;
    int _YdimG = (YdimG == -1)?1:YdimG;
    int _ZdimB = (ZdimB == -1)?1:ZdimB;
    int _YdimB = (YdimB == -1)?1:YdimB;

    decimate(
        ArrayR, _ZdimR, _YdimR, XdimR,
        ArrayG, _ZdimG, _YdimG, XdimG,
        ArrayB, _ZdimB, _YdimB, XdimB,
        input_bit, output_bit,
        ArrayOut, YdimOut, XdimOut);
}

void decimate_indexes_safe(
        uint16_t *ArrayR, int ZdimR, int YdimR, int XdimR,
        uint16_t *ArrayG, int ZdimG, int YdimG, int XdimG,
        uint16_t *ArrayB, int ZdimB, int YdimB, int XdimB,
        int input_bit, int output_bit,
        uint64_t **ArrayOut, int *NdimOut)
{

    int _ZdimR = (ZdimR == -1)?1:ZdimR;
    int _YdimR = (YdimR == -1)?1:YdimR;
    int _ZdimG = (ZdimG == -1)?1:ZdimG;
    int _YdimG = (YdimG == -1)?1:YdimG;
    int _ZdimB = (ZdimB == -1)?1:ZdimB;
    int _YdimB = (YdimB == -1)?1:YdimB;

    decimate_indexes(
        ArrayR, _ZdimR, _YdimR, XdimR,
        ArrayG, _ZdimG, _YdimG, XdimG,
        ArrayB, _ZdimB, _YdimB, XdimB,
        input_bit, output_bit,
        ArrayOut, NdimOut);
}

%}

%ignore decimate;
%ignore decimate_indexes;
%include "libdecimate.h"
