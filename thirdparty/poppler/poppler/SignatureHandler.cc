//========================================================================
//
// SignatureHandler.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015, 2016 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2016 Albert Astals Cid <aacid@kde.org>
// Copyright 2015 Markus Kilås <digital@markuspage.com>
//
//========================================================================

#include <config.h>

#include "SignatureHandler.h"
#include "goo/gmem.h"
#include <secmod.h>

#include <dirent.h>
#include <Error.h>

unsigned int SignatureHandler::digestLength(SECOidTag digestAlgId)
{
  switch(digestAlgId){
    case SEC_OID_SHA1:
      return 20;
    case SEC_OID_SHA256:
      return 32;
    case SEC_OID_SHA384:
      return 48;
    case SEC_OID_SHA512:
      return 64;
    default:
      printf("ERROR: Unrecognized Hash ID\n");
      return 0;
  }
}

char *SignatureHandler::getSignerName()
{
  if (!CMSSignerInfo)
      return NULL;

  CERTCertificate *cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB());
  return CERT_GetCommonName(&cert->subject);
}

time_t SignatureHandler::getSigningTime()
{
  PRTime sTime; // time in microseconds since the epoch

  if (NSS_CMSSignerInfo_GetSigningTime (CMSSignerInfo, &sTime) != SECSuccess)
    return 0;

  return (time_t) sTime/1000000;
}


GooString *SignatureHandler::getDefaultFirefoxCertDB_Linux()
{
  GooString * finalPath = NULL;
  DIR *toSearchIn;
  struct dirent *subFolder;

  GooString * homePath = new GooString(getenv("HOME"));
  homePath = homePath->append("/.mozilla/firefox/");

  if ((toSearchIn = opendir(homePath->getCString())) == NULL) {
    error(errInternal, 0, "couldn't find default Firefox Folder");
    delete homePath;
    return NULL;
  }
  do {
    if ((subFolder = readdir(toSearchIn)) != NULL) {
      if (strstr(subFolder->d_name, "default") != NULL) {
	finalPath = homePath->append(subFolder->d_name);
	closedir(toSearchIn);
	return finalPath;
      }
    }
  } while (subFolder != NULL);

  closedir(toSearchIn);
  delete homePath;
  return NULL;
}

/**
 * Initialise NSS
 */
void SignatureHandler::init_nss() 
{
  GooString *certDBPath = getDefaultFirefoxCertDB_Linux();
  if (certDBPath == NULL) {
    NSS_Init("sql:/etc/pki/nssdb");
  } else {
    NSS_Init(certDBPath->getCString());
  }
  //Make sure NSS root certificates module is loaded
  SECMOD_AddNewModule("Root Certs", "libnssckbi.so", 0, 0);

  delete certDBPath;
}


SignatureHandler::SignatureHandler(unsigned char *p7, int p7_length)
 : CMSMessage(NULL),
   CMSSignedData(NULL),
   CMSSignerInfo(NULL),
   temp_certs(NULL)
{
  init_nss();
  CMSitem.data = p7;
  CMSitem.len = p7_length;
  CMSMessage = CMS_MessageCreate(&CMSitem);
  CMSSignedData = CMS_SignedDataCreate(CMSMessage);
  CMSSignerInfo = CMS_SignerInfoCreate(CMSSignedData);
  hash_context = initHashContext();
}

HASHContext * SignatureHandler::initHashContext()
{

  SECItem usedAlgorithm = NSS_CMSSignedData_GetDigestAlgs(CMSSignedData)[0]->algorithm;
  hash_length = digestLength(SECOID_FindOIDTag(&usedAlgorithm));
  HASH_HashType hashType;
  hashType    = HASH_GetHashTypeByOidTag(SECOID_FindOIDTag(&usedAlgorithm));
  return HASH_Create(hashType);
}

void SignatureHandler::updateHash(unsigned char * data_block, int data_len)
{
  HASH_Update(hash_context, data_block, data_len);
}

SignatureHandler::~SignatureHandler()
{
  SECITEM_FreeItem(&CMSitem, PR_FALSE);
  if (CMSSignerInfo)
    NSS_CMSSignerInfo_Destroy(CMSSignerInfo);
  if (CMSSignedData)
    NSS_CMSSignedData_Destroy(CMSSignedData);
  if (CMSMessage)
    NSS_CMSMessage_Destroy(CMSMessage);

  if (hash_context)
    HASH_Destroy(hash_context);

  free(temp_certs);

  if (NSS_Shutdown()!=SECSuccess)
    fprintf(stderr, "Detail: %s\n", PR_ErrorToString(PORT_GetError(), PR_LANGUAGE_I_DEFAULT));
}

NSSCMSMessage *SignatureHandler::CMS_MessageCreate(SECItem * cms_item)
{
  if (cms_item->data){
    return NSS_CMSMessage_CreateFromDER(cms_item, NULL, NULL /* Content callback */
                        , NULL, NULL /*Password callback*/
                        , NULL, NULL /*Decrypt callback*/);
  } else {
    return NULL;
  }
}

NSSCMSSignedData *SignatureHandler::CMS_SignedDataCreate(NSSCMSMessage * cms_msg)
{
  if (!NSS_CMSMessage_IsSigned(cms_msg)) {
    error(errInternal, 0, "Input couldn't be parsed as a CMS signature");
    return NULL;
  }

  NSSCMSContentInfo *cinfo = NSS_CMSMessage_ContentLevel(cms_msg, 0);
  if (!cinfo) {
    error(errInternal, 0, "Error in NSS_CMSMessage_ContentLevel");
    return NULL;
  }

  NSSCMSSignedData *signedData = (NSSCMSSignedData*) NSS_CMSContentInfo_GetContent(cinfo);
  if (!signedData) {
    error(errInternal, 0, "CError in NSS_CMSContentInfo_GetContent()");
    return NULL;
  }

  if (signedData->rawCerts)
  {
    size_t i;
    for (i = 0; signedData->rawCerts[i]; ++i) {} // just count the length of the certificate chain

    // tempCerts field needs to be filled for complete memory release by NSSCMSSignedData_Destroy
    signedData->tempCerts = (CERTCertificate **) gmallocn( i+1, sizeof(CERTCertificate *));
    memset(signedData->tempCerts, 0, (i+1) * sizeof(CERTCertificate *));
    // store the adresses of these temporary certificates for future release
    for (i = 0; signedData->rawCerts[i]; ++i)
      signedData->tempCerts[i] = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), signedData->rawCerts[i], NULL, 0, 0);

    temp_certs = signedData->tempCerts;
    return signedData;
  } else {
    return NULL;
  }
}

NSSCMSSignerInfo *SignatureHandler::CMS_SignerInfoCreate(NSSCMSSignedData * cms_sig_data)
{
  NSSCMSSignerInfo *signerInfo = NSS_CMSSignedData_GetSignerInfo(cms_sig_data, 0);
  if (!signerInfo) {
    printf("Error in NSS_CMSSignedData_GetSignerInfo()\n");
    return NULL;
  } else {
    return signerInfo;
  }
}

NSSCMSVerificationStatus SignatureHandler::validateSignature()
{
  unsigned char *digest_buffer = NULL;

  if (!CMSSignedData)
    return NSSCMSVS_MalformedSignature;

  digest_buffer = (unsigned char *)PORT_Alloc(hash_length);
  unsigned int result_len = 0;

  HASH_End(hash_context, digest_buffer, &result_len, hash_length);

  SECItem digest;
  digest.data = digest_buffer;
  digest.len = hash_length;

  if ((NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB())) == NULL)
    CMSSignerInfo->verificationStatus = NSSCMSVS_SigningCertNotFound;

  SECItem * content_info_data = CMSSignedData->contentInfo.content.data;
  if (content_info_data != NULL && content_info_data->data != NULL)
  {
    /*
      This means it's not a detached type signature
      so the digest is contained in SignedData->contentInfo
    */
    if (memcmp(digest.data, content_info_data->data, hash_length) == 0
        && digest.len == content_info_data->len)
    {
      PORT_Free(digest_buffer);
      return NSSCMSVS_GoodSignature;
    }
    else
    {
      PORT_Free(digest_buffer);
      return NSSCMSVS_DigestMismatch;
    }

  }
  else if (NSS_CMSSignerInfo_Verify(CMSSignerInfo, &digest, NULL) != SECSuccess)
  {

    PORT_Free(digest_buffer);
    return CMSSignerInfo->verificationStatus;
  }
  else
  {
    PORT_Free(digest_buffer);
    return NSSCMSVS_GoodSignature;
  }
}

SECErrorCodes SignatureHandler::validateCertificate()
{
  SECErrorCodes retVal;
  CERTCertificate *cert;

  if (!CMSSignerInfo)
    return (SECErrorCodes) -1; //error code to avoid matching error codes defined in SECErrorCodes

  if ((cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB())) == NULL)
    CMSSignerInfo->verificationStatus = NSSCMSVS_SigningCertNotFound;

  CERTValInParam inParams[2];
  inParams[0].type = cert_pi_revocationFlags;
  inParams[0].value.pointer.revocation = CERT_GetClassicOCSPEnabledSoftFailurePolicy();
  inParams[1].type = cert_pi_end;

  CERT_PKIXVerifyCert(cert, certificateUsageEmailSigner, inParams, NULL,
                CMSSignerInfo->cmsg->pwfn_arg);

  retVal = (SECErrorCodes) PORT_GetError();

  if (cert)
    CERT_DestroyCertificate(cert);

  return retVal;
}


SignatureValidationStatus SignatureHandler::NSS_SigTranslate(NSSCMSVerificationStatus nss_code)
{
  switch(nss_code)
  {
    case NSSCMSVS_GoodSignature:
      return SIGNATURE_VALID;

    case NSSCMSVS_BadSignature:
      return SIGNATURE_INVALID;

      case NSSCMSVS_DigestMismatch:
      return SIGNATURE_DIGEST_MISMATCH;

    case NSSCMSVS_ProcessingError:
      return SIGNATURE_DECODING_ERROR;

    default:
      return SIGNATURE_GENERIC_ERROR;
  }
}

CertificateValidationStatus SignatureHandler::NSS_CertTranslate(SECErrorCodes nss_code)
{
  // 0 not defined in SECErrorCodes, it means success for this purpose.
  if (nss_code == (SECErrorCodes) 0)
    return CERTIFICATE_TRUSTED;

  switch(nss_code)
  {
    case SEC_ERROR_UNKNOWN_ISSUER:
      return CERTIFICATE_UNKNOWN_ISSUER;

    case SEC_ERROR_UNTRUSTED_ISSUER:
      return CERTIFICATE_UNTRUSTED_ISSUER;

    case SEC_ERROR_REVOKED_CERTIFICATE:
      return CERTIFICATE_REVOKED;

    case SEC_ERROR_EXPIRED_CERTIFICATE:
      return CERTIFICATE_EXPIRED;

    default:
      return CERTIFICATE_GENERIC_ERROR;
  }
}
