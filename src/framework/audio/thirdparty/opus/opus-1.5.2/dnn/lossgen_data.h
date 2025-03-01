/* Auto generated from checkpoint lossgen2_2000.pth */


#ifndef LOSSGEN_DATA_H
#define LOSSGEN_DATA_H

#include "nnet.h"


#include "opus_types.h"

#define LOSSGEN_DENSE_IN_OUT_SIZE 8

#define LOSSGEN_DENSE_OUT_OUT_SIZE 1

#define LOSSGEN_GRU1_OUT_SIZE 16

#define LOSSGEN_GRU1_STATE_SIZE 16

#define LOSSGEN_GRU2_OUT_SIZE 32

#define LOSSGEN_GRU2_STATE_SIZE 32


#define LOSSGEN_MAX_RNN_UNITS 32


typedef struct {
    LinearLayer lossgen_dense_in;
    LinearLayer lossgen_dense_out;
    LinearLayer lossgen_gru1_input;
    LinearLayer lossgen_gru1_recurrent;
    LinearLayer lossgen_gru2_input;
    LinearLayer lossgen_gru2_recurrent;
} LossGen;

int init_lossgen(LossGen *model, const WeightArray *arrays);

#endif /* LOSSGEN_DATA_H */
