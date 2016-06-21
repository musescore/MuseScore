//========================================================================
//
// SignatureInfo.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef SIGNATUREINFO_H
#define SIGNATUREINFO_H

#include <time.h>

enum SignatureValidationStatus
{
  SIGNATURE_VALID,
  SIGNATURE_INVALID,
  SIGNATURE_DIGEST_MISMATCH,
  SIGNATURE_DECODING_ERROR,
  SIGNATURE_GENERIC_ERROR,
  SIGNATURE_NOT_FOUND,
  SIGNATURE_NOT_VERIFIED
};

enum CertificateValidationStatus
{
  CERTIFICATE_TRUSTED,
  CERTIFICATE_UNTRUSTED_ISSUER,
  CERTIFICATE_UNKNOWN_ISSUER,
  CERTIFICATE_REVOKED,
  CERTIFICATE_EXPIRED,
  CERTIFICATE_GENERIC_ERROR,
  CERTIFICATE_NOT_VERIFIED
};

class SignatureInfo {
public:
  SignatureInfo();
  SignatureInfo(SignatureValidationStatus, CertificateValidationStatus);
  ~SignatureInfo();

  /* GETTERS */
  SignatureValidationStatus getSignatureValStatus();
  CertificateValidationStatus getCertificateValStatus();
  char *getSignerName();
  time_t getSigningTime();
  bool isSubfilterSupported() { return sig_subfilter_supported; }

  /* SETTERS */
  void setSignatureValStatus(enum SignatureValidationStatus );
  void setCertificateValStatus(enum CertificateValidationStatus );
  void setSignerName(char *);
  void setSigningTime(time_t);
  void setSubFilterSupport(bool isSupported) { sig_subfilter_supported = isSupported; }

private:
  SignatureInfo(const SignatureInfo &);
  SignatureInfo& operator=(const SignatureInfo &);

  SignatureValidationStatus sig_status;
  CertificateValidationStatus cert_status;
  char *signer_name;
  time_t signing_time;
  bool sig_subfilter_supported;
};

#endif
