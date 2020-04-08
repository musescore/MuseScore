#include "doubleinputvalidator.h"

DoubleInputValidator::DoubleInputValidator(QObject *parent) : QValidator(parent)
      {
      }

void DoubleInputValidator::fixup(QString& string) const
      {
      if (!string.contains("."))
            string.append(".00");

      if (string.startsWith("."))
            string.prepend("0");

      if (string.endsWith("."))
            string.append("00");

      QStringList strList = string.split(".", QString::SkipEmptyParts);

      QString intPart = strList.at(0);
      QString floatPart = strList.at(1);

      if (intPart == QString("-"))
            intPart.append("0");
      if (floatPart.length() == 1)
            floatPart.append("0");

      if (intPart.contains(QRegExp("^0{1,3}$")))
            intPart = QString("0");
      else if (intPart.contains(QRegExp("^\\-0{1,3}$")))
            intPart = QString("-0");

      if (intPart == QString("-0") && floatPart == QString("00"))
            intPart = QString("0");

      string = QString("%1.%2").arg(intPart).arg(floatPart);
      }

QValidator::State DoubleInputValidator::validate(QString& inputStr, int& cursorPos) const
      {
      if (inputStr.contains(QRegExp("^\\-?\\d{1,3}\\.\\d{2}$"))) {
            if (inputStr.contains(QRegExp("^\\-?0{2,3}\\."))
               || inputStr == QString("-0.00")) {
                  return Intermediate;
                  }
            return Acceptable;
            }
      else if (inputStr.contains(QRegExp("^\\-?\\d{0,3}\\.?$"))
         || inputStr.contains(QRegExp("^\\-?\\d{0,3}\\.\\d{0,2}$"))) {
            return Intermediate;
            }
      else {
            cursorPos = 0;
            return Invalid;
            }
      }
