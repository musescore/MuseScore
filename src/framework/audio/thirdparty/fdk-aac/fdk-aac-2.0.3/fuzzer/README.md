# Fuzzer for libFraunhoferAAC decoder

## Plugin Design Considerations
The fuzzer plugin for aac decoder is designed based on the understanding of the
codec and tries to achieve the following:

##### Maximize code coverage

This fuzzer makes use of the following config parameters:
1. Transport type (parameter name: `TRANSPORT_TYPE`)

| Parameter| Valid Values| Configured Value|
|------------- |-------------| ----- |
| `TRANSPORT_TYPE` | 0.`TT_UNKNOWN  ` 1.`TT_MP4_RAW ` 2.`TT_MP4_ADIF ` 3.`TT_MP4_ADTS ` 4.`TT_MP4_LATM_MCP1 ` 5.`TT_MP4_LATM_MCP0  ` 6.`TT_MP4_LOAS ` 7.`TT_DRM ` | `TT_MP4_ADIF ` |

Note: Value of `TRANSPORT_TYPE` could be set to any of these values.
It is set to `TT_MP4_ADIF` in the fuzzer plugin.

##### Maximize utilization of input data
The plugin feeds the entire input data to the codec using a loop.
 * If the decode operation was successful, the input is advanced by an
   offset calculated using valid bytes.
 * If the decode operation was un-successful, the input is advanced by 1 byte
   till it reaches a valid frame or end of stream.

This ensures that the plugin tolerates any kind of input (empty, huge,
malformed, etc) and doesnt `exit()` on any input and thereby increasing the
chance of identifying vulnerabilities.

## Build

This describes steps to build aac_dec_fuzzer binary.

## Android

### Steps to build
Build the fuzzer
```
  $ mm -j$(nproc) aac_dec_fuzzer
```

### Steps to run
Create a directory CORPUS_DIR and copy some aac files to that folder.
Push this directory to device.

To run on device
```
  $ adb sync data
  $ adb shell /data/fuzz/arm64/aac_dec_fuzzer/aac_dec_fuzzer CORPUS_DIR
```
To run on host
```
  $ $ANDROID_HOST_OUT/fuzz/x86_64/aac_dec_fuzzer/aac_dec_fuzzer CORPUS_DIR
```

# Fuzzer for libFraunhoferAAC encoder

## Plugin Design Considerations
The fuzzer plugin for aac encoder is designed based on the understanding of the
codec and tries to achieve the following:

##### Maximize code coverage

The configuration parameters are not hardcoded, but instead selected based on
incoming data. This ensures more code paths are reached by the fuzzer.

Following arguments are passed to aacEncoder_SetParam to set the respective AACENC_PARAM parameter:

| AACENC_PARAM param| Valid Values| Configured Value|
|-------------| ----- |----- |
|`AACENC_SBR_MODE` | `-1 ` `0 ` `1 ` `2 ` | Calculated using first byte of data |
|`AACENC_AOT` |`AOT_NONE ` `AOT_NULL_OBJECT ` `AOT_AAC_MAIN ` `AOT_AAC_LC ` `AOT_AAC_SSR ` `AOT_AAC_LTP ` `AOT_SBR ` `AOT_AAC_SCAL ` `AOT_TWIN_VQ ` `AOT_CELP ` `AOT_HVXC ` `AOT_RSVD_10 ` `AOT_RSVD_11 ` `AOT_TTSI ` `AOT_MAIN_SYNTH ` `AOT_WAV_TAB_SYNTH ` `AOT_GEN_MIDI ` `AOT_ALG_SYNTH_AUD_FX ` `AOT_ER_AAC_LC ` `AOT_RSVD_18 ` `AOT_ER_AAC_LTP ` `AOT_ER_AAC_SCAL ` `AOT_ER_TWIN_VQ ` `AOT_ER_BSAC ` `AOT_ER_AAC_LD ` `AOT_ER_CELP ` `AOT_ER_HVXC ` `AOT_ER_HILN ` `AOT_ER_PARA ` `AOT_RSVD_28 ` `AOT_PS ` `AOT_MPEGS ` `AOT_ESCAPE ` `AOT_MP3ONMP4_L1 ` `AOT_MP3ONMP4_L2 ` `AOT_MP3ONMP4_L3 ` `AOT_RSVD_35 ` `AOT_RSVD_36 ` `AOT_AAC_SLS ` `AOT_SLS ` `AOT_ER_AAC_ELD ` `AOT_USAC ` `AOT_SAOC ` `AOT_LD_MPEGS ` `AOT_MP2_AAC_LC ` `AOT_MP2_SBR ` `AOT_DRM_AAC ` `AOT_DRM_SBR ` `AOT_DRM_MPEG_PS ` `AOT_DRM_SURROUND ` `AOT_DRM_USAC `  | Calculated using second byte of data  |
|`AACENC_SAMPLERATE` |  `8000 ` `11025 ` `12000 ` `16000 ` `22050 ` `24000 ` `32000 ` `44100 ` `48000 ` `64000 ` `88200 ` `96000 `| Calculated using third byte of data  |
|`AACENC_BITRATE` | In range `8000 ` to `960000 ` | Calculated using fourth, fifth and sixth byte of data  |
|`AACENC_CHANNELMODE` | `MODE_1 ` `MODE_2 ` `MODE_1_2 ` `MODE_1_2_1 ` `MODE_1_2_2 ` `MODE_1_2_2_1 ` `MODE_1_2_2_2_1 ` `MODE_6_1 `  `MODE_7_1_BACK ` `MODE_7_1_TOP_FRONT ` `MODE_7_1_REAR_SURROUND ` `MODE_7_1_FRONT_CENTER ` `MODE_212 ` | Calculated using seventh byte of data |
|`AACENC_TRANSMUX` | `TT_MP4_RAW ` `TT_MP4_ADIF ` `TT_MP4_ADTS ` `TT_MP4_LATM_MCP1 ` `TT_MP4_LATM_MCP0 ` `TT_MP4_LOAS ` `TT_DRM `  | Calculated using eight byte of data  |`AACENC_SBR_RATIO` |`-1 ` `0 ` `1 ` `2 ` | Calculated using ninth byte of data |
|`AACENC_BITRATEMODE` |`AACENC_BR_MODE_INVALID ` `AACENC_BR_MODE_CBR ` `AACENC_BR_MODE_VBR_1 ` `AACENC_BR_MODE_VBR_2 ` `AACENC_BR_MODE_VBR_3 ` `AACENC_BR_MODE_VBR_4 ` `AACENC_BR_MODE_VBR_5 ` `AACENC_BR_MODE_FF ` `AACENC_BR_MODE_SFR `  | Calculated using thirty-fourth byte of data |
|`AACENC_GRANULE_LENGTH` |`120 ` `128 ` `240 ` `256 ` `480 ` `512 ` `1024 ` | Calculated using thirty-fifth byte of data |
|`AACENC_CHANNELORDER` |`CH_ORDER_MPEG ` `CH_ORDER_WAV ` | Calculated using thirty-sixth byte of data |
|`AACENC_AFTERBURNER` |`0 ` `1 ` | Calculated using thirty-seventh byte of data |
|`AACENC_BANDWIDTH` |`0 ` `1`  | Calculated using thirty-eigth byte of data |
|` AACENC_IDX_PEAK_BITRATE` | In range `8000 ` to `960000 ` | Calculated using thirty-ninth byte of data |
|` AACENC_HEADER_PERIOD` |In range `0 ` to `255 ` | Calculated using fortieth byte of data |
|` AACENC_SIGNALING_MODE` |`-1 ` `0 ` `1 ` `2 ` `3 `  | Calculated using forty-first byte of data |
|` AACENC_TPSUBFRAMES` |In range `0 ` to `255 ` | Calculated using forty-second byte of data |
|` AACENC_AUDIOMUXVER` |`-1 ` `0 ` `1 ` `2 ` | Calculated using forty-third byte of data |
|` AACENC_PROTECTION` |`0 ` `1 ` | Calculated using forty-fourth of data |
|`AACENC_ANCILLARY_BITRATE` |In range `0 ` to `960000 `| Calculated using forty-fifth byte of data |
|`AACENC_METADATA_MODE ` |`0 ` `1 ` `2 ` `3 ` | Calculated using forty-sixth byte of data |

Following values are configured to set up the meta data represented by the class variable `mMetaData ` :

| Variable name| Possible Values| Configured Value|
|------------- | ----- |----- |
| `drc_profile`   | `AACENC_METADATA_DRC_NONE ` `AACENC_METADATA_DRC_FILMSTANDARD ` `AACENC_METADATA_DRC_FILMLIGHT ` `AACENC_METADATA_DRC_MUSICSTANDARD ` `AACENC_METADATA_DRC_MUSICLIGHT ` `AACENC_METADATA_DRC_SPEECH ` `AACENC_METADATA_DRC_NOT_PRESENT `  | Calculated using tenth byte of data |
| `comp_profile`   | `AACENC_METADATA_DRC_NONE ` `AACENC_METADATA_DRC_FILMSTANDARD ` `AACENC_METADATA_DRC_FILMLIGHT ` `AACENC_METADATA_DRC_MUSICSTANDARD ` `AACENC_METADATA_DRC_MUSICLIGHT ` `AACENC_METADATA_DRC_SPEECH ` `AACENC_METADATA_DRC_NOT_PRESENT `  | Calculated using eleventh byte of data |
| `drc_TargetRefLevel`   | In range `0 ` to `255 `  | Calculated using twelfth byte of data |
| `comp_TargetRefLevel`   | In range `0 ` to `255 `  | Calculated using thirteenth byte of data |
| `prog_ref_level_present`   | `0 ` `1 `  | Calculated using fourteenth byte of data |
| `prog_ref_level`   | In range `0 ` to `255 `  | Calculated using fifteenth byte of data |
| `PCE_mixdown_idx_present`   | `0 ` `1 `   | Calculated using sixteenth byte of data |
| `ETSI_DmxLvl_present`   | `0 ` `1 `   | Calculated using seventeenth byte of data |
| `centerMixLevel`   | In range `0 ` to `7 `  | Calculated using eighteenth byte of data |
| `surroundMixLevel`   | In range `0 ` to `7 `  | Calculated using nineteenth byte of data |
| `dolbySurroundMode`   | In range `0 ` to `2 `   | Calculated using twentieth byte of data |
| `drcPresentationMode`   | In range `0 ` to `2 `   | Calculated using twenty-first byte of data |
| `extAncDataEnable`   | `0 ` `1 `  | Calculated using twenty-second byte of data |
| `extDownmixLevelEnable`   | `0 ` `1 `  | Calculated using twenty-third byte of data |
| `extDownmixLevel_A`   | In range `0 ` to `7 `  | Calculated using twenty-fourth byte of data |
| `extDownmixLevel_B`   | In range `0 ` to `7 `  | Calculated using twenty-fifth byte of data |
| `dmxGainEnable`   |  `0 ` `1 `   | Calculated using twenty-sixth byte of data |
| `dmxGain5`   | In range `0 ` to `255 `  | Calculated using twenty-seventh byte of data |
| `dmxGain2`   | In range `0 ` to `255 `  | Calculated using twenty-eighth byte of data |
| `lfeDmxEnable`   | `0 ` `1 `  | Calculated using twenty-ninth byte of data |
| `lfeDmxLevel`   | In range `0 ` to `15 `  | Calculated using thirtieth byte of data |

Indexes `mInBufferIdx_1`, `mInBufferIdx_2`  and `mInBufferIdx_3`(in range `0 ` to `2`) are calculated using the thirty-first, thirty-second and thirty-third byte respectively.

##### Maximize utilization of input data
The plugin feeds the entire input data to the codec and continues with the encoding even on a failure. This ensures that the plugin tolerates any kind of input (empty, huge, malformed, etc) and doesnt `exit()` on any input and thereby increasing the chance of identifying vulnerabilities.

## Build

This describes steps to build aac_enc_fuzzer binary.

## Android

### Steps to build
Build the fuzzer
```
  $ mm -j$(nproc) aac_enc_fuzzer
```

### Steps to run
Create a directory CORPUS_DIR and copy some raw files to that folder.
Push this directory to device.

To run on device
```
  $ adb sync data
  $ adb shell /data/fuzz/arm64/aac_enc_fuzzer/aac_enc_fuzzer CORPUS_DIR
```
To run on host
```
  $ $ANDROID_HOST_OUT/fuzz/x86_64/aac_enc_fuzzer/aac_enc_fuzzer CORPUS_DIR
```

## References:
 * http://llvm.org/docs/LibFuzzer.html
 * https://github.com/google/oss-fuzz
