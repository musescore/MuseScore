#include "intinputvalidator.h"

IntInputValidator::IntInputValidator(QObject* parent)
    : QValidator(parent)
{
}

void IntInputValidator::fixup(QString& string) const
{
    if (string.isEmpty() || string.endsWith("-")) {
        string.append("0");
    }
    if (string.toInt() == 0) {
        string = "0";
    }

    if (string.toInt() > m_top) {
        string = QString::number(m_top);
    } else if (string.toInt() < m_bottom) {
        string = QString::number(m_bottom);
    }
}

QValidator::State IntInputValidator::validate(QString& inputStr, int& cursorPos) const
{
    QValidator::State state = Invalid;

    if (inputStr.contains(QRegExp("^\\-?\\d{1,3}$"))) {
        if (inputStr.contains(QRegExp("^\\-?0{2,3}"))
            || (inputStr.startsWith("-") && inputStr.toDouble() == 0.0)) {
            state = Intermediate;
        } else {
            state = Acceptable;
        }
    } else if (inputStr.contains(QRegExp("^\\-?$"))) {
        state = Intermediate;
    } else {
        cursorPos = 0;
        return Invalid;
    }

    if (inputStr.toInt() > m_top || inputStr.toInt() < m_bottom) {
        state = Intermediate;
    }

    return state;
}

int IntInputValidator::top() const
{
    return m_top;
}

int IntInputValidator::bottom() const
{
    return m_bottom;
}

void IntInputValidator::setTop(int top)
{
    m_top = top;
}

void IntInputValidator::setBottom(int bottom)
{
    m_bottom = bottom;
}
