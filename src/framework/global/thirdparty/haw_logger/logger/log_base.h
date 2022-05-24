#ifndef HAW_LOG_H
#define HAW_LOG_H

#include "helpful.h"
#include "logger.h"

#ifndef FUNC_INFO
#if defined(_MSC_VER)
    #define FUNC_INFO __FUNCSIG__
#else
    #define FUNC_INFO __PRETTY_FUNCTION__
#endif
#endif

//! Format
#define CLASSNAME(sig) haw::logger::Helpful::className(sig)
#define FUNCNAME(sig) haw::logger::Helpful::methodName(sig)

//! Log

#ifndef LOG_TAG
#define LOG_TAG CLASSNAME(FUNC_INFO)
#endif

#define IF_LOGLEVEL(level)  if (haw::logger::Logger::instance()->isLevel(level))

#define LOG_STREAM(type, tag, funcInfo) haw::logger::LogInput(type, tag, funcInfo).stream
#define LOG(type, tag)  LOG_STREAM(type, tag, FUNCNAME(FUNC_INFO) + ": ")

#define LOGE_T(tag) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::ERRR, tag)
#define LOGW_T(tag) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::WARN, tag)
#define LOGI_T(tag) IF_LOGLEVEL(haw::logger::Normal) LOG(haw::logger::Logger::INFO, tag)
#define LOGD_T(tag) IF_LOGLEVEL(haw::logger::Debug) LOG(haw::logger::Logger::DEBG, tag)

#define LOGE LOGE_T(LOG_TAG)
#define LOGW LOGW_T(LOG_TAG)
#define LOGI LOGI_T(LOG_TAG)
#define LOGD LOGD_T(LOG_TAG)
#define LOGN if (0) LOGD_T(LOG_TAG) // compiling, but no output

//! Helps
#define DEPRECATED LOGD() << "This function deprecated!!"
#define DEPRECATED_USE(use) LOGD() << "This function deprecated!! Use:" << use
#define NOT_IMPLEMENTED LOGW() << "Not implemented!!"
#define NOT_IMPL_RETURN NOT_IMPLEMENTED; return
#define NOT_SUPPORTED LOGW() << "Not supported!!"
#define NOT_SUPPORTED_USE(use) LOGW() << "Not supported!! Use:" << use

#endif // HAW_LOG_H
