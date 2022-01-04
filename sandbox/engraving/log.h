#include <QDebug>
#include <string>

#define LOGE() qDebug() << Q_FUNC_INFO
#define LOGW() qDebug() << Q_FUNC_INFO
#define LOGI() qDebug() << Q_FUNC_INFO
#define LOGD() qDebug() << Q_FUNC_INFO

inline QDebug operator<<(QDebug debug, const std::string& s)
{
    debug << s.c_str();
    return debug;
}

#define DEPRECATED LOGD() << "This function deprecated!!"
#define DEPRECATED_USE(use) LOGD() << "This function deprecated!! Use:" << use
#define NOT_IMPLEMENTED LOGW() << "Not implemented!!"
#define NOT_IMPL_RETURN NOT_IMPLEMENTED return
#define NOT_SUPPORTED LOGW() << "Not supported!!"
#define NOT_SUPPORTED_USE(use) LOGW() << "Not supported!! Use:" << use

#define TRACEFUNC

#define IF_ASSERT_FAILED_X(cond, msg) if (!(cond)) { \
        LOGE() << "\"ASSERT FAILED!\":" << msg << __FILE__ << __LINE__; \
        Q_ASSERT(cond); \
} \
    if (!(cond)) \

#define IF_ASSERT_FAILED(cond) IF_ASSERT_FAILED_X(cond, #cond)

#define IF_FAILED(cond) if (!(cond)) { \
        LOGE() << "\"FAILED!\":" << #cond << __FILE__ << __LINE__; \
} \
    if (!(cond)) \

#define UNUSED(x) (void)x;
#define UNREACHABLE
