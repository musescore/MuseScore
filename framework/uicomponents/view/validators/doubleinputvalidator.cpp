#include "doubleinputvalidator.h"

DoubleInputValidator::DoubleInputValidator(QObject* parent)
    : QValidator(parent)
{
}

void DoubleInputValidator::fixup(QString& string) const
{
    auto zeros = [](int num)->QString {
                     QString s = QString();
                     for (int i = 0; i < num; i++) {
                         s.append("0");
                     }
                     return s;
                 };

    if (!string.contains(".")) {
        string.append("." + zeros(m_decimal));
    }

    if (string.startsWith(".")) {
        string.prepend("0");
    }

    if (string.endsWith(".")) {
        string.append(zeros(m_decimal));
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList strList = string.split(".", Qt::SkipEmptyParts);
#else
    QStringList strList = string.split(".", QString::SkipEmptyParts);
#endif

    QString intPart = strList.at(0);
    QString floatPart = strList.at(1);

    if (floatPart.length() < m_decimal) {
        floatPart.append(zeros(m_decimal - floatPart.length()));
    }

    if (intPart.contains(QRegExp("^0{1,3}$"))) {
        intPart = QString("0");
    } else if (intPart.contains(QRegExp("^\\-0{0,3}$"))) {
        intPart = QString("-0");
    }

    if (intPart == QString("-0") && floatPart == zeros(m_decimal)) {
        intPart = QString("0");
    }

    string = QString("%1.%2").arg(intPart).arg(floatPart);

    if (string.toDouble() > m_top) {
        string = QString::number(m_top, 'f', m_decimal);
    } else if (string.toDouble() < m_bottom) {
        string = QString::number(m_bottom, 'f', m_decimal);
    }
}

QValidator::State DoubleInputValidator::validate(QString& inputStr, int& cursorPos) const
{
    QValidator::State state = Invalid;

    if (inputStr.contains(QRegExp(QString("^\\-?\\d{1,3}\\.\\d{%1}$").arg(m_decimal)))) {
        if (inputStr.contains(QRegExp("^\\-?0{2,3}\\."))
            || (inputStr.startsWith("-") && inputStr.toDouble() == 0.0)) {
            state = Intermediate;
        } else {
            state = Acceptable;
        }
    } else if (inputStr.contains(QRegExp("^\\-?\\d{0,3}\\.?$"))
               || inputStr.contains(QRegExp(QString("^\\-?\\d{0,3}\\.\\d{0,%1}$").arg(m_decimal)))) {
        state = Intermediate;
    } else {
        cursorPos = 0;
        return Invalid;
    }

    if (inputStr.toDouble() > m_top || inputStr.toDouble() < m_bottom) {
        state = Intermediate;
    }

    return state;
}

qreal DoubleInputValidator::top() const
{
    return m_top;
}

qreal DoubleInputValidator::bottom() const
{
    return m_bottom;
}

int DoubleInputValidator::decimal() const
{
    return m_decimal;
}

void DoubleInputValidator::setTop(qreal top)
{
    m_top = top;
}

void DoubleInputValidator::setBottom(qreal bottom)
{
    m_bottom = bottom;
}

void DoubleInputValidator::setDecimal(int decimal)
{
    m_decimal = decimal;
}
