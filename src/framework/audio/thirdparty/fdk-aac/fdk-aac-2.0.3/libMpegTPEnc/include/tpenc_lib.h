/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2018 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.

 1.    INTRODUCTION
The Fraunhofer FDK AAC Codec Library for Android ("FDK AAC Codec") is software
that implements the MPEG Advanced Audio Coding ("AAC") encoding and decoding
scheme for digital audio. This FDK AAC Codec software is intended to be used on
a wide variety of Android devices.

AAC's HE-AAC and HE-AAC v2 versions are regarded as today's most efficient
general perceptual audio codecs. AAC-ELD is considered the best-performing
full-bandwidth communications codec by independent studies and is widely
deployed. AAC has been standardized by ISO and IEC as part of the MPEG
specifications.

Patent licenses for necessary patent claims for the FDK AAC Codec (including
those of Fraunhofer) may be obtained through Via Licensing
(www.vialicensing.com) or through the respective patent owners individually for
the purpose of encoding or decoding bit streams in products that are compliant
with the ISO/IEC MPEG audio standards. Please note that most manufacturers of
Android devices already license these patent claims through Via Licensing or
directly from the patent owners, and therefore FDK AAC Codec software may
already be covered under those patent licenses when it is used for those
licensed purposes only.

Commercially-licensed AAC software libraries, including floating-point versions
with enhanced sound quality, are also available from Fraunhofer. Users are
encouraged to check the Fraunhofer website for additional applications
information and documentation.

2.    COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

You must retain the complete text of this software license in redistributions of
the FDK AAC Codec or your modifications thereto in source code form.

You must retain the complete text of this software license in the documentation
and/or other materials provided with redistributions of the FDK AAC Codec or
your modifications thereto in binary form. You must make available free of
charge copies of the complete source code of the FDK AAC Codec and your
modifications thereto to recipients of copies in binary form.

The name of Fraunhofer may not be used to endorse or promote products derived
from this library without prior written permission.

You may not charge copyright license fees for anyone to use, copy or distribute
the FDK AAC Codec software or your modifications thereto.

Your modified versions of the FDK AAC Codec must carry prominent notices stating
that you changed the software and the date of any change. For modified versions
of the FDK AAC Codec, the term "Fraunhofer FDK AAC Codec Library for Android"
must be replaced by the term "Third-Party Modified Version of the Fraunhofer FDK
AAC Codec Library for Android."

3.    NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software.

You may use this FDK AAC Codec software or modifications thereto only for
purposes that are authorized by appropriate patent licenses.

4.    DISCLAIMER

This FDK AAC Codec software is provided by Fraunhofer on behalf of the copyright
holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
including but not limited to the implied warranties of merchantability and
fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary,
or consequential damages, including but not limited to procurement of substitute
goods or services; loss of use, data, or profits, or business interruption,
however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of
this software, even if advised of the possibility of such damage.

5.    CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Audio and Multimedia Departments - FDK AAC LL
Am Wolfsmantel 33
91058 Erlangen, Germany

www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
----------------------------------------------------------------------------- */

/******************* MPEG transport format encoder library *********************

   Author(s):   Manuel Jander

   Description: MPEG Transport encode

*******************************************************************************/

#ifndef TPENC_LIB_H
#define TPENC_LIB_H

#include "tp_data.h"
#include "FDK_bitstream.h"

#define TRANSPORTENC_INBUF_SIZE 8192

typedef enum {
  TRANSPORTENC_OK = 0,             /*!< All fine.             */
  TRANSPORTENC_NO_MEM,             /*!< Out of memory.             */
  TRANSPORTENC_UNKOWN_ERROR = 1,   /*!< Unknown error (embarrasing).   */
  TRANSPORTENC_INVALID_PARAMETER,  /*!< An invalid parameter was passed to a
                                      function .          */
  TRANSPORTENC_UNSUPPORTED_FORMAT, /*!< Unsupported transport format. */
  TRANSPORTENC_NOT_ENOUGH_BITS,    /*!< Out of bits. Provide more bits and try
                                      again.            */

  TRANSPORTENC_INVALID_CONFIG, /*!< Error in configuration.               */
  TRANSPORTENC_LATM_INVALID_NR_OF_SUBFRAMES, /*!< LATM: number of subframes out
                                                of range.                  */
  TRANSPORTENC_LOAS_NOT_AVAILABLE, /*!< LOAS format not supported.           */
  TRANSPORTENC_INVALID_LATM_ALIGNMENT, /*!< AudioMuxElement length not aligned
                                          to 1 byte.            */

  TRANSPORTENC_INVALID_TRANSMISSION_FRAME_LENGTH, /*!< Invalid transmission
                                                     frame length (< 0). */
  TRANSPORTENC_INVALID_CELP_FRAME_LENGTH, /*!< Invalid CELP frame length found
                                             (>= 62).                 */
  TRANSPORTENC_INVALID_FRAME_BITS, /*!< Frame bits is not 40 and not 80. */
  TRANSPORTENC_INVALID_AOT,        /*!< Unknown AOT found.        */
  TRANSPORTENC_INVALID_AU_LENGTH   /*!< Invalid Access Unit length (not
                                      byte-aligned).           */

} TRANSPORTENC_ERROR;

typedef struct TRANSPORTENC *HANDLE_TRANSPORTENC;

/**
 * \brief             Determine a reasonable channel configuration on the basis
 * of channel_mode.
 * \param noChannels  Number of audio channels.
 * \return            CHANNEL_MODE value that matches the given amount of audio
 * channels.
 */
CHANNEL_MODE transportEnc_GetChannelMode(int noChannels);

/**
 * \brief                Register SBR heaqder writer callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle SBR header
 * writing.
 * \param user_data      void pointer for user data passed to the callback as
 * first parameter.
 * \return               0 on success.
 */
int transportEnc_RegisterSbrCallback(HANDLE_TRANSPORTENC hTpEnc,
                                     const cbSbr_t cbSbr, void *user_data);

/**
 * \brief                Register USAC SC writer callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle USAC
 * SCwriting.
 * \param user_data      void pointer for user data passed to the callback as
 * first parameter.
 * \return               0 on success.
 */
int transportEnc_RegisterUsacCallback(HANDLE_TRANSPORTENC hTpEnc,
                                      const cbUsac_t cbUsac, void *user_data);

/**
 * \brief                Register SSC writer callback.
 * \param hTp            Handle of transport decoder.
 * \param cbUpdateConfig Pointer to a callback function to handle SSC writing.
 * \param user_data      void pointer for user data passed to the callback as
 * first parameter.
 * \return               0 on success.
 */
int transportEnc_RegisterSscCallback(HANDLE_TRANSPORTENC hTpEnc,
                                     const cbSsc_t cbSsc, void *user_data);

/**
 * \brief         Write ASC from given parameters.
 * \param asc     A HANDLE_FDK_BITSTREAM where the ASC is written to.
 * \param config  Structure containing the codec configuration settings.
 * \param cb callback information structure.
 * \return        0 on success.
 */
int transportEnc_writeASC(HANDLE_FDK_BITSTREAM asc, CODER_CONFIG *config,
                          CSTpCallBacks *cb);

/* Defintion of flags that can be passed to transportEnc_Open() */
#define TP_FLAG_MPEG4 1     /** MPEG4 (instead of MPEG2) */
#define TP_FLAG_LATM_AMV 2  /** LATM AudioMuxVersion  */
#define TP_FLAG_LATM_AMVA 4 /** LATM AudioMuxVersionA */

/**
 * \brief               Allocate transport encoder.
 * \param phTpEnc       Pointer to transport encoder handle.
 * \return              Error code.
 */
TRANSPORTENC_ERROR transportEnc_Open(HANDLE_TRANSPORTENC *phTpEnc);

/**
 * \brief               Init transport encoder.
 * \param bsBuffer      Pointer to transport encoder.
 * \param bsBuffer      Pointer to bitstream buffer.
 * \param bsBufferSize  Size in bytes of bsBuffer.
 * \param transportFmt  Format of the transport to be written.
 * \param config        Pointer to a valid CODER_CONFIG struct.
 * \param flags         Transport encoder flags.
 * \return              Error code.
 */
TRANSPORTENC_ERROR transportEnc_Init(HANDLE_TRANSPORTENC hTpEnc,
                                     UCHAR *bsBuffer, INT bsBufferSize,
                                     TRANSPORT_TYPE transportFmt,
                                     CODER_CONFIG *config, UINT flags);

/**
 * \brief               Write additional bits in transport encoder.
 * \param config        Pointer to a valid CODER_CONFIG struct.
 * \param nBits         Number of additional bits.
 * \return              Error code.
 */
TRANSPORTENC_ERROR transportEnc_AddOtherDataBits(HANDLE_TRANSPORTENC hTpEnc,
                                                 const int nBits);

/**
 * \brief      Get transport encoder bitstream.
 * \param hTp  Pointer to a transport encoder handle.
 * \return     The handle to the requested FDK bitstream.
 */
HANDLE_FDK_BITSTREAM transportEnc_GetBitstream(HANDLE_TRANSPORTENC hTp);

/**
 * \brief         Get amount of bits required by the transport headers.
 * \param hTp     Handle of transport encoder.
 * \param auBits  Amount of payload bits required for the current subframe.
 * \return        Error code.
 */
INT transportEnc_GetStaticBits(HANDLE_TRANSPORTENC hTp, int auBits);

/**
 * \brief       Close transport encoder. This function assures that all
 * allocated memory is freed.
 * \param phTp  Pointer to a previously allocated transport encoder handle.
 */
void transportEnc_Close(HANDLE_TRANSPORTENC *phTp);

/**
 * \brief                       Write one access unit.
 * \param hTp                   Handle of transport encoder.
 * \param total_bits            Amount of total access unit bits.
 * \param bufferFullness        Value of current buffer fullness in bits.
 * \param noConsideredChannels  Number of bitrate wise considered channels (all
 * minus LFE channels).
 * \return                      Error code.
 */
TRANSPORTENC_ERROR transportEnc_WriteAccessUnit(HANDLE_TRANSPORTENC hTp,
                                                INT total_bits,
                                                int bufferFullness,
                                                int noConsideredChannels);

/**
 * \brief        Inform the transportEnc layer that writing of access unit has
 * finished. This function is required to be called when the encoder has
 * finished writing one Access one Access Unit for bitstream
 * housekeeping.
 * \param hTp    Transport handle.
 * \param pBits  Pointer to an int, where the current amount of frame bits is
 * passed and where the current amount of subframe bits is returned.
 *
 * OR:  This integer is modified by the amount of extra bit alignment that may
 * occurr.
 *
 * \return       Error code.
 */
TRANSPORTENC_ERROR transportEnc_EndAccessUnit(HANDLE_TRANSPORTENC hTp,
                                              int *pBits);

/*
 * \brief         Get a payload frame.
 * \param hTpEnc  Transport encoder handle.
 * \param nBytes  Pointer to an int to hold the frame size in bytes. Returns
 * zero if currently there is no complete frame for output (number of sub frames
 * > 1).
 * \return        Error code.
 */
TRANSPORTENC_ERROR transportEnc_GetFrame(HANDLE_TRANSPORTENC hTpEnc,
                                         int *nbytes);

/* ADTS CRC support */

/**
 * \brief         Set current bitstream position as start of a new data region.
 * \param hTpEnc  Transport encoder handle.
 * \param mBits   Size in bits of the data region. Set to 0 if it should not be
 * of a fixed size.
 * \return        Data region ID, which should be used when calling
 * transportEnc_CrcEndReg().
 */
int transportEnc_CrcStartReg(HANDLE_TRANSPORTENC hTpEnc, int mBits);

/**
 * \brief         Set end of data region.
 * \param hTpEnc  Transport encoder handle.
 * \param reg     Data region ID, opbtained from transportEnc_CrcStartReg().
 * \return        void
 */
void transportEnc_CrcEndReg(HANDLE_TRANSPORTENC hTpEnc, int reg);

/**
 * \brief             Get AudioSpecificConfig or StreamMuxConfig from transport
 * encoder handle and write it to dataBuffer.
 * \param hTpEnc      Transport encoder handle.
 * \param cc          Pointer to the current and valid configuration contained
 * in a CODER_CONFIG struct.
 * \param dataBuffer  Bitbuffer holding binary configuration.
 * \param confType    Pointer to an UINT where the configuration type is
 * returned (0:ASC, 1:SMC).
 * \return            Error code.
 */
TRANSPORTENC_ERROR transportEnc_GetConf(HANDLE_TRANSPORTENC hTpEnc,
                                        CODER_CONFIG *cc,
                                        FDK_BITSTREAM *dataBuffer,
                                        UINT *confType);

/**
 * \brief       Get information (version among other things) of the transport
 * encoder library.
 * \param info  Pointer to an allocated LIB_INFO struct.
 * \return      Error code.
 */
TRANSPORTENC_ERROR transportEnc_GetLibInfo(LIB_INFO *info);

#endif /* #ifndef TPENC_LIB_H */
