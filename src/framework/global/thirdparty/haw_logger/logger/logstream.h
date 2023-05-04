#ifndef HAW_LOGSTREAM_H
#define HAW_LOGSTREAM_H

#include <sstream>
#include <thread>

#ifdef HAW_LOGGER_QT_SUPPORT
#include <QDebug>
#endif

namespace haw::logger {
class Stream
{
public:
    Stream() = default;

    inline Stream& operator<<(bool t) { m_ss << t; return *this; }
    inline Stream& operator<<(char t) { m_ss << t; return *this; }
    inline Stream& operator<<(signed short t) { m_ss << t; return *this; }
    inline Stream& operator<<(unsigned short t) { m_ss << t; return *this; }
    inline Stream& operator<<(char16_t t) { m_ss << static_cast<uint16_t>(t); return *this; }
    inline Stream& operator<<(char32_t t) { m_ss << static_cast<uint32_t>(t); return *this; }
    inline Stream& operator<<(signed int t) { m_ss << t; return *this; }
    inline Stream& operator<<(unsigned int t) { m_ss << t; return *this; }
    inline Stream& operator<<(signed long t) { m_ss << t; return *this; }
    inline Stream& operator<<(unsigned long t) { m_ss << t; return *this; }
    inline Stream& operator<<(signed long long t) { m_ss << t; return *this; }
    inline Stream& operator<<(unsigned long long t) { m_ss << t; return *this; }
    inline Stream& operator<<(float t) { m_ss << t; return *this; }
    inline Stream& operator<<(double t) { m_ss << t; return *this; }
    inline Stream& operator<<(const void* t) { m_ss << t; return *this; }
    inline Stream& operator<<(const char* t) { m_ss << t; return *this; }

    inline Stream& operator<<(const std::string& t) { m_ss << t; return *this; }
    inline Stream& operator<<(const std::thread::id& t) { m_ss << t; return *this; }

    template<typename T>
    inline Stream& operator<<(const std::vector<T>& t)
    {
        m_ss << '[';
        for (size_t i = 0; i < t.size(); ++i) {
            m_ss << t[i];
            if (i < t.size() - 1) {
                m_ss << ',';
            }
        }
        m_ss << ']';
        return *this;
    }

#ifdef HAW_LOGGER_QT_SUPPORT
    inline Stream& operator<<(QChar t) { qt_to_ss(t); return *this; }
    inline Stream& operator<<(const QString& t) { qt_to_ss(t); return *this; }
    inline Stream& operator<<(const QStringRef& t) { qt_to_ss(t); return *this; }
    inline Stream& operator<<(QLatin1String t) { qt_to_ss(t); return *this; }
    inline Stream& operator<<(const QByteArray& t) { qt_to_ss(t); return *this; }
    inline Stream& operator<<(const QVariant& t) { qt_to_ss(t); return *this; }

#endif

    inline std::string str() const { return m_ss.str(); }

private:

#ifdef HAW_LOGGER_QT_SUPPORT
    template<typename T>
    inline void qt_to_ss(const T& t)
    {
        QString str;
        QDebug q(&str);
        q << t;
        m_ss << str.toStdString();
    }

#endif

    std::stringstream m_ss;
};
}

#endif //HAW_LOGSTREAM_H
