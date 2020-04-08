#ifndef DOUBLEINPUTVALIDATOR_H
#define DOUBLEINPUTVALIDATOR_H

#include <QString>
#include <QValidator>

class DoubleInputValidator : public QValidator
{
    Q_OBJECT

public:
    explicit DoubleInputValidator(QObject* parent = nullptr);

    void fixup(QString& string) const override;
    State validate(QString &inputStr, int &cursorPos) const override;
    };

#endif // DOUBLEINPUTVALIDATOR_H
