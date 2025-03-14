/* Auto generated from checkpoint pitch_vsmallconv1.pth */


#ifndef PITCHDNN_DATA_H
#define PITCHDNN_DATA_H

#include "nnet.h"


#include "opus_types.h"

#define DENSE_IF_UPSAMPLER_1_OUT_SIZE 64

#define DENSE_IF_UPSAMPLER_2_OUT_SIZE 64

#define DENSE_DOWNSAMPLER_OUT_SIZE 64

#define DENSE_FINAL_UPSAMPLER_OUT_SIZE 192

#define GRU_1_OUT_SIZE 64

#define GRU_1_STATE_SIZE 64


#define PITCH_DNN_MAX_RNN_UNITS 64


struct PitchDNN {
    LinearLayer dense_if_upsampler_1;
    LinearLayer dense_if_upsampler_2;
    LinearLayer dense_downsampler;
    LinearLayer dense_final_upsampler;
    Conv2dLayer conv2d_1;
    Conv2dLayer conv2d_2;
    LinearLayer gru_1_input;
    LinearLayer gru_1_recurrent;
};

int init_pitchdnn(PitchDNN *model, const WeightArray *arrays);

#endif /* PITCHDNN_DATA_H */
