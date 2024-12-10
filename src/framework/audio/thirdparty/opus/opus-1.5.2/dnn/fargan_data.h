
#ifndef FARGAN_DATA_H
#define FARGAN_DATA_H

#include "nnet.h"


#define COND_NET_PEMBED_OUT_SIZE 12

#define COND_NET_FDENSE1_OUT_SIZE 64

#define COND_NET_FCONV1_OUT_SIZE 128

#define COND_NET_FCONV1_IN_SIZE 64

#define COND_NET_FCONV1_STATE_SIZE (64 * (2))

#define COND_NET_FCONV1_DELAY 1

#define COND_NET_FDENSE2_OUT_SIZE 320

#define SIG_NET_COND_GAIN_DENSE_OUT_SIZE 1

#define SIG_NET_FWC0_CONV_OUT_SIZE 192

#define SIG_NET_FWC0_GLU_GATE_OUT_SIZE 192

#define SIG_NET_GRU1_OUT_SIZE 160

#define SIG_NET_GRU1_STATE_SIZE 160

#define SIG_NET_GRU2_OUT_SIZE 128

#define SIG_NET_GRU2_STATE_SIZE 128

#define SIG_NET_GRU3_OUT_SIZE 128

#define SIG_NET_GRU3_STATE_SIZE 128

#define SIG_NET_GRU1_GLU_GATE_OUT_SIZE 160

#define SIG_NET_GRU2_GLU_GATE_OUT_SIZE 128

#define SIG_NET_GRU3_GLU_GATE_OUT_SIZE 128

#define SIG_NET_SKIP_GLU_GATE_OUT_SIZE 128

#define SIG_NET_SKIP_DENSE_OUT_SIZE 128

#define SIG_NET_SIG_DENSE_OUT_OUT_SIZE 40

#define SIG_NET_GAIN_DENSE_OUT_OUT_SIZE 4

typedef struct {
    LinearLayer cond_net_pembed;
    LinearLayer cond_net_fdense1;
    LinearLayer cond_net_fconv1;
    LinearLayer cond_net_fdense2;
    LinearLayer sig_net_cond_gain_dense;
    LinearLayer sig_net_fwc0_conv;
    LinearLayer sig_net_fwc0_glu_gate;
    LinearLayer sig_net_gru1_input;
    LinearLayer sig_net_gru1_recurrent;
    LinearLayer sig_net_gru2_input;
    LinearLayer sig_net_gru2_recurrent;
    LinearLayer sig_net_gru3_input;
    LinearLayer sig_net_gru3_recurrent;
    LinearLayer sig_net_gru1_glu_gate;
    LinearLayer sig_net_gru2_glu_gate;
    LinearLayer sig_net_gru3_glu_gate;
    LinearLayer sig_net_skip_glu_gate;
    LinearLayer sig_net_skip_dense;
    LinearLayer sig_net_sig_dense_out;
    LinearLayer sig_net_gain_dense_out;
} FARGAN;

int init_fargan(FARGAN *model, const WeightArray *arrays);

#endif /* FARGAN_DATA_H */
