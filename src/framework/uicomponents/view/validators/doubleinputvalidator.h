#ifndef DOUBLEINPUTVALIDATOR_H
#define DOUBLEINPUTVALIDATOR_H

#include <QString>
#include <QValidator>

class DoubleInputValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qreal top READ top WRITE setTop)
    Q_PROPERTY(qreal bottom READ bottom WRITE setBottom)
    Q_PROPERTY(int decimal READ decimal WRITE setDecimal)

public:
    explicit DoubleInputValidator(QObject* parent = nullptr);

    void fixup(QString& string) const override;
    State validate(QString& inputStr, int& cursorPos) const override;

    qreal top() const;
    qreal bottom() const;
    int decimal() const;

public slots:
    void setTop(qreal);
    void setBottom(qreal);
    void setDecimal(int);

private:
    qreal m_top = 999.0;
    qreal m_bottom = -999.0;
    int m_decimal = 2;
};

#endif // DOUBLEINPUTVALIDATOR_H
