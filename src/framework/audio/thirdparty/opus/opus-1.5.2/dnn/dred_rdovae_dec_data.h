/* Auto generated from checkpoint rdovae_sparse5m_32.pth */


#ifndef DRED_RDOVAE_DEC_DATA_H
#define DRED_RDOVAE_DEC_DATA_H

#include "nnet.h"


#include "opus_types.h"

#include "dred_rdovae.h"

#include "dred_rdovae_constants.h"


#define DEC_DENSE1_OUT_SIZE 96

#define DEC_GLU1_OUT_SIZE 96

#define DEC_GLU2_OUT_SIZE 96

#define DEC_GLU3_OUT_SIZE 96

#define DEC_GLU4_OUT_SIZE 96

#define DEC_GLU5_OUT_SIZE 96

#define DEC_OUTPUT_OUT_SIZE 80

#define DEC_HIDDEN_INIT_OUT_SIZE 128

#define DEC_GRU_INIT_OUT_SIZE 480

#define DEC_GRU1_OUT_SIZE 96

#define DEC_GRU1_STATE_SIZE 96

#define DEC_GRU2_OUT_SIZE 96

#define DEC_GRU2_STATE_SIZE 96

#define DEC_GRU3_OUT_SIZE 96

#define DEC_GRU3_STATE_SIZE 96

#define DEC_GRU4_OUT_SIZE 96

#define DEC_GRU4_STATE_SIZE 96

#define DEC_GRU5_OUT_SIZE 96

#define DEC_GRU5_STATE_SIZE 96

#define DEC_CONV1_OUT_SIZE 32

#define DEC_CONV1_IN_SIZE 192

#define DEC_CONV1_STATE_SIZE (192 * (1))

#define DEC_CONV1_DELAY 0

#define DEC_CONV2_OUT_SIZE 32

#define DEC_CONV2_IN_SIZE 320

#define DEC_CONV2_STATE_SIZE (320 * (1))

#define DEC_CONV2_DELAY 0

#define DEC_CONV3_OUT_SIZE 32

#define DEC_CONV3_IN_SIZE 448

#define DEC_CONV3_STATE_SIZE (448 * (1))

#define DEC_CONV3_DELAY 0

#define DEC_CONV4_OUT_SIZE 32

#define DEC_CONV4_IN_SIZE 576

#define DEC_CONV4_STATE_SIZE (576 * (1))

#define DEC_CONV4_DELAY 0

#define DEC_CONV5_OUT_SIZE 32

#define DEC_CONV5_IN_SIZE 704

#define DEC_CONV5_STATE_SIZE (704 * (1))

#define DEC_CONV5_DELAY 0

struct RDOVAEDec {
    LinearLayer dec_dense1;
    LinearLayer dec_glu1;
    LinearLayer dec_glu2;
    LinearLayer dec_glu3;
    LinearLayer dec_glu4;
    LinearLayer dec_glu5;
    LinearLayer dec_output;
    LinearLayer dec_hidden_init;
    LinearLayer dec_gru_init;
    LinearLayer dec_gru1_input;
    LinearLayer dec_gru1_recurrent;
    LinearLayer dec_gru2_input;
    LinearLayer dec_gru2_recurrent;
    LinearLayer dec_gru3_input;
    LinearLayer dec_gru3_recurrent;
    LinearLayer dec_gru4_input;
    LinearLayer dec_gru4_recurrent;
    LinearLayer dec_gru5_input;
    LinearLayer dec_gru5_recurrent;
    LinearLayer dec_conv1;
    LinearLayer dec_conv2;
    LinearLayer dec_conv3;
    LinearLayer dec_conv4;
    LinearLayer dec_conv5;
};

int init_rdovaedec(RDOVAEDec *model, const WeightArray *arrays);

#endif /* DRED_RDOVAE_DEC_DATA_H */
