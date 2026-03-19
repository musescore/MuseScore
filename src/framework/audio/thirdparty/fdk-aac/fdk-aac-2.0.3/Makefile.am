ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = \
    -I$(top_srcdir)/libAACdec/include \
    -I$(top_srcdir)/libAACenc/include \
    -I$(top_srcdir)/libArithCoding/include \
    -I$(top_srcdir)/libDRCdec/include \
    -I$(top_srcdir)/libSACdec/include \
    -I$(top_srcdir)/libSACenc/include \
    -I$(top_srcdir)/libSBRdec/include \
    -I$(top_srcdir)/libSBRenc/include \
    -I$(top_srcdir)/libMpegTPDec/include \
    -I$(top_srcdir)/libMpegTPEnc/include \
    -I$(top_srcdir)/libSYS/include \
    -I$(top_srcdir)/libFDK/include \
    -I$(top_srcdir)/libPCMutils/include

AM_CXXFLAGS = -fno-exceptions -fno-rtti
libfdk_aac_la_LINK = $(LINK) $(libfdk_aac_la_LDFLAGS)
# Mention a dummy pure C file to trigger generation of the $(LINK) variable
nodist_EXTRA_libfdk_aac_la_SOURCES = dummy.c

fdk_aacincludedir = $(includedir)/fdk-aac
fdk_aacinclude_HEADERS = \
    $(top_srcdir)/libSYS/include/machine_type.h \
    $(top_srcdir)/libSYS/include/genericStds.h \
    $(top_srcdir)/libSYS/include/FDK_audio.h \
    $(top_srcdir)/libSYS/include/syslib_channelMapDescr.h \
    $(top_srcdir)/libAACenc/include/aacenc_lib.h \
    $(top_srcdir)/libAACdec/include/aacdecoder_lib.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fdk-aac.pc

lib_LTLIBRARIES = libfdk-aac.la

libfdk_aac_la_LDFLAGS = -version-info @FDK_AAC_VERSION@ -no-undefined \
    -export-symbols $(top_srcdir)/fdk-aac.sym

if EXAMPLE
bin_PROGRAMS = aac-enc$(EXEEXT)
noinst_PROGRAMS = test-encode-decode$(EXEEXT)

aac_enc_LDADD = libfdk-aac.la
aac_enc_SOURCES = aac-enc.c wavreader.c

test_encode_decode_LDADD = libfdk-aac.la
test_encode_decode_SOURCES = test-encode-decode.c wavreader.c sha1.c

noinst_HEADERS = wavreader.h
endif

AACDEC_SRC = \
    libAACdec/src/FDK_delay.cpp \
    libAACdec/src/aac_ram.cpp \
    libAACdec/src/aac_rom.cpp \
    libAACdec/src/aacdec_drc.cpp \
    libAACdec/src/aacdec_hcr.cpp \
    libAACdec/src/aacdec_hcr_bit.cpp \
    libAACdec/src/aacdec_hcrs.cpp \
    libAACdec/src/aacdec_pns.cpp \
    libAACdec/src/aacdec_tns.cpp \
    libAACdec/src/aacdecoder.cpp \
    libAACdec/src/aacdecoder_lib.cpp \
    libAACdec/src/block.cpp \
    libAACdec/src/channel.cpp \
    libAACdec/src/channelinfo.cpp \
    libAACdec/src/conceal.cpp \
    libAACdec/src/ldfiltbank.cpp \
    libAACdec/src/pulsedata.cpp \
    libAACdec/src/rvlc.cpp \
    libAACdec/src/rvlcbit.cpp \
    libAACdec/src/rvlcconceal.cpp \
    libAACdec/src/stereo.cpp \
    libAACdec/src/usacdec_ace_d4t64.cpp \
    libAACdec/src/usacdec_ace_ltp.cpp \
    libAACdec/src/usacdec_acelp.cpp \
    libAACdec/src/usacdec_fac.cpp \
    libAACdec/src/usacdec_lpc.cpp \
    libAACdec/src/usacdec_lpd.cpp \
    libAACdec/src/usacdec_rom.cpp

AACENC_SRC = \
    libAACenc/src/aacEnc_ram.cpp \
    libAACenc/src/aacEnc_rom.cpp \
    libAACenc/src/aacenc.cpp \
    libAACenc/src/aacenc_lib.cpp \
    libAACenc/src/aacenc_pns.cpp \
    libAACenc/src/aacenc_tns.cpp \
    libAACenc/src/adj_thr.cpp \
    libAACenc/src/band_nrg.cpp \
    libAACenc/src/bandwidth.cpp \
    libAACenc/src/bit_cnt.cpp \
    libAACenc/src/bitenc.cpp \
    libAACenc/src/block_switch.cpp \
    libAACenc/src/channel_map.cpp \
    libAACenc/src/chaosmeasure.cpp \
    libAACenc/src/dyn_bits.cpp \
    libAACenc/src/grp_data.cpp \
    libAACenc/src/intensity.cpp \
    libAACenc/src/line_pe.cpp \
    libAACenc/src/metadata_compressor.cpp \
    libAACenc/src/metadata_main.cpp \
    libAACenc/src/mps_main.cpp \
    libAACenc/src/ms_stereo.cpp \
    libAACenc/src/noisedet.cpp \
    libAACenc/src/pnsparam.cpp \
    libAACenc/src/pre_echo_control.cpp \
    libAACenc/src/psy_configuration.cpp \
    libAACenc/src/psy_main.cpp \
    libAACenc/src/qc_main.cpp \
    libAACenc/src/quantize.cpp \
    libAACenc/src/sf_estim.cpp \
    libAACenc/src/spreading.cpp \
    libAACenc/src/tonality.cpp \
    libAACenc/src/transform.cpp

ARITHCODING_SRC = \
    libArithCoding/src/ac_arith_coder.cpp

DRCDEC_SRC = \
    libDRCdec/src/FDK_drcDecLib.cpp \
    libDRCdec/src/drcDec_gainDecoder.cpp \
    libDRCdec/src/drcDec_reader.cpp \
    libDRCdec/src/drcDec_rom.cpp \
    libDRCdec/src/drcDec_selectionProcess.cpp \
    libDRCdec/src/drcDec_tools.cpp \
    libDRCdec/src/drcGainDec_init.cpp \
    libDRCdec/src/drcGainDec_preprocess.cpp \
    libDRCdec/src/drcGainDec_process.cpp

FDK_SRC = \
    libFDK/src/FDK_bitbuffer.cpp \
    libFDK/src/FDK_core.cpp \
    libFDK/src/FDK_crc.cpp \
    libFDK/src/FDK_decorrelate.cpp \
    libFDK/src/FDK_hybrid.cpp \
    libFDK/src/FDK_lpc.cpp \
    libFDK/src/FDK_matrixCalloc.cpp \
    libFDK/src/FDK_qmf_domain.cpp \
    libFDK/src/FDK_tools_rom.cpp \
    libFDK/src/FDK_trigFcts.cpp \
    libFDK/src/autocorr2nd.cpp \
    libFDK/src/dct.cpp \
    libFDK/src/fft.cpp \
    libFDK/src/fft_rad2.cpp \
    libFDK/src/fixpoint_math.cpp \
    libFDK/src/huff_nodes.cpp \
    libFDK/src/mdct.cpp \
    libFDK/src/nlc_dec.cpp \
    libFDK/src/qmf.cpp \
    libFDK/src/scale.cpp

MPEGTPDEC_SRC = \
    libMpegTPDec/src/tpdec_adif.cpp \
    libMpegTPDec/src/tpdec_adts.cpp \
    libMpegTPDec/src/tpdec_asc.cpp \
    libMpegTPDec/src/tpdec_drm.cpp \
    libMpegTPDec/src/tpdec_latm.cpp \
    libMpegTPDec/src/tpdec_lib.cpp

MPEGTPENC_SRC = \
    libMpegTPEnc/src/tpenc_adif.cpp \
    libMpegTPEnc/src/tpenc_adts.cpp \
    libMpegTPEnc/src/tpenc_asc.cpp \
    libMpegTPEnc/src/tpenc_latm.cpp \
    libMpegTPEnc/src/tpenc_lib.cpp

PCMUTILS_SRC = \
    libPCMutils/src/limiter.cpp \
    libPCMutils/src/pcm_utils.cpp \
    libPCMutils/src/pcmdmx_lib.cpp

SACDEC_SRC = \
    libSACdec/src/sac_bitdec.cpp \
    libSACdec/src/sac_calcM1andM2.cpp \
    libSACdec/src/sac_dec.cpp \
    libSACdec/src/sac_dec_conceal.cpp \
    libSACdec/src/sac_dec_lib.cpp \
    libSACdec/src/sac_process.cpp \
    libSACdec/src/sac_qmf.cpp \
    libSACdec/src/sac_reshapeBBEnv.cpp \
    libSACdec/src/sac_rom.cpp \
    libSACdec/src/sac_smoothing.cpp \
    libSACdec/src/sac_stp.cpp \
    libSACdec/src/sac_tsd.cpp

SACENC_SRC = \
    libSACenc/src/sacenc_bitstream.cpp \
    libSACenc/src/sacenc_delay.cpp \
    libSACenc/src/sacenc_dmx_tdom_enh.cpp \
    libSACenc/src/sacenc_filter.cpp \
    libSACenc/src/sacenc_framewindowing.cpp \
    libSACenc/src/sacenc_huff_tab.cpp \
    libSACenc/src/sacenc_lib.cpp \
    libSACenc/src/sacenc_nlc_enc.cpp \
    libSACenc/src/sacenc_onsetdetect.cpp \
    libSACenc/src/sacenc_paramextract.cpp \
    libSACenc/src/sacenc_staticgain.cpp \
    libSACenc/src/sacenc_tree.cpp \
    libSACenc/src/sacenc_vectorfunctions.cpp

SBRDEC_SRC = \
    libSBRdec/src/HFgen_preFlat.cpp \
    libSBRdec/src/env_calc.cpp \
    libSBRdec/src/env_dec.cpp \
    libSBRdec/src/env_extr.cpp \
    libSBRdec/src/hbe.cpp \
    libSBRdec/src/huff_dec.cpp \
    libSBRdec/src/lpp_tran.cpp \
    libSBRdec/src/psbitdec.cpp \
    libSBRdec/src/psdec.cpp \
    libSBRdec/src/psdec_drm.cpp \
    libSBRdec/src/psdecrom_drm.cpp \
    libSBRdec/src/pvc_dec.cpp \
    libSBRdec/src/sbr_deb.cpp \
    libSBRdec/src/sbr_dec.cpp \
    libSBRdec/src/sbr_ram.cpp \
    libSBRdec/src/sbr_rom.cpp \
    libSBRdec/src/sbrdec_drc.cpp \
    libSBRdec/src/sbrdec_freq_sca.cpp \
    libSBRdec/src/sbrdecoder.cpp

SBRENC_SRC = \
    libSBRenc/src/bit_sbr.cpp \
    libSBRenc/src/code_env.cpp \
    libSBRenc/src/env_bit.cpp \
    libSBRenc/src/env_est.cpp \
    libSBRenc/src/fram_gen.cpp \
    libSBRenc/src/invf_est.cpp \
    libSBRenc/src/mh_det.cpp \
    libSBRenc/src/nf_est.cpp \
    libSBRenc/src/ps_bitenc.cpp \
    libSBRenc/src/ps_encode.cpp \
    libSBRenc/src/ps_main.cpp \
    libSBRenc/src/resampler.cpp \
    libSBRenc/src/sbr_encoder.cpp \
    libSBRenc/src/sbr_misc.cpp \
    libSBRenc/src/sbrenc_freq_sca.cpp \
    libSBRenc/src/sbrenc_ram.cpp \
    libSBRenc/src/sbrenc_rom.cpp \
    libSBRenc/src/ton_corr.cpp \
    libSBRenc/src/tran_det.cpp

SYS_SRC = \
    libSYS/src/genericStds.cpp \
    libSYS/src/syslib_channelMapDescr.cpp

libfdk_aac_la_SOURCES = \
    $(AACDEC_SRC) $(AACENC_SRC) \
    $(ARITHCODING_SRC) \
    $(DRCDEC_SRC) \
    $(MPEGTPDEC_SRC) $(MPEGTPENC_SRC) \
    $(SACDEC_SRC) $(SACENC_SRC) \
    $(SBRDEC_SRC) $(SBRENC_SRC) \
    $(PCMUTILS_SRC) $(FDK_SRC) $(SYS_SRC)

EXTRA_DIST = \
    $(top_srcdir)/CMakeLists.txt \
    $(top_srcdir)/.clang-format \
    $(top_srcdir)/autogen.sh \
    $(top_srcdir)/MODULE_LICENSE_FRAUNHOFER \
    $(top_srcdir)/NOTICE \
    $(top_srcdir)/OWNERS \
    $(top_srcdir)/Android.bp \
    $(top_srcdir)/fdk-aac.def \
    $(top_srcdir)/fdk-aac.sym \
    $(top_srcdir)/fdk-aac-config.cmake.in \
    $(top_srcdir)/documentation/*.pdf \
    $(top_srcdir)/fuzzer/* \
    $(top_srcdir)/libAACdec/src/*.h \
    $(top_srcdir)/libAACdec/src/arm/*.cpp \
    $(top_srcdir)/libAACenc/src/*.h \
    $(top_srcdir)/libArithCoding/include/*.h \
    $(top_srcdir)/libDRCdec/include/*.h \
    $(top_srcdir)/libDRCdec/src/*.h \
    $(top_srcdir)/libSACdec/include/*.h \
    $(top_srcdir)/libSACdec/src/*.h \
    $(top_srcdir)/libSACenc/include/*.h \
    $(top_srcdir)/libSACenc/src/*.h \
    $(top_srcdir)/libSBRenc/src/*.h \
    $(top_srcdir)/libSBRenc/include/*.h \
    $(top_srcdir)/libSBRdec/src/*.h \
    $(top_srcdir)/libSBRdec/include/*.h \
    $(top_srcdir)/libSYS/include/*.h \
    $(top_srcdir)/libPCMutils/include/*.h \
    $(top_srcdir)/libPCMutils/src/*.h \
    $(top_srcdir)/libMpegTPEnc/include/*.h \
    $(top_srcdir)/libMpegTPEnc/src/*.h \
    $(top_srcdir)/libMpegTPDec/include/*.h \
    $(top_srcdir)/libMpegTPDec/src/*.h \
    $(top_srcdir)/libFDK/include/*.h \
    $(top_srcdir)/libFDK/include/arm/*.h \
    $(top_srcdir)/libFDK/include/mips/*.h \
    $(top_srcdir)/libFDK/include/ppc/*.h \
    $(top_srcdir)/libFDK/include/x86/*.h \
    $(top_srcdir)/libFDK/src/arm/*.cpp \
    $(top_srcdir)/libFDK/src/mips/*.cpp \
    $(top_srcdir)/METADATA \
    $(top_srcdir)/PREUPLOAD.cfg \
    $(top_srcdir)/sha1.h \
    $(top_srcdir)/test/* \
    $(top_srcdir)/win32/*.h

