/* Auto generated from checkpoint plc4ar_16.pth */


#ifndef PLC_DATA_H
#define PLC_DATA_H

#include "nnet.h"


#include "opus_types.h"

#define PLC_DENSE_IN_OUT_SIZE 128

#define PLC_DENSE_OUT_OUT_SIZE 20

#define PLC_GRU1_OUT_SIZE 192

#define PLC_GRU1_STATE_SIZE 192

#define PLC_GRU2_OUT_SIZE 192

#define PLC_GRU2_STATE_SIZE 192


#define PLC_MAX_RNN_UNITS 192


typedef struct {
    LinearLayer plc_dense_in;
    LinearLayer plc_dense_out;
    LinearLayer plc_gru1_input;
    LinearLayer plc_gru1_recurrent;
    LinearLayer plc_gru2_input;
    LinearLayer plc_gru2_recurrent;
} PLCModel;

int init_plcmodel(PLCModel *model, const WeightArray *arrays);

#endif /* PLC_DATA_H */
