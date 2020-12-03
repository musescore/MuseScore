#ifndef INTINPUTVALIDATOR_H
#define INTINPUTVALIDATOR_H

#include <QString>
#include <QValidator>

class IntInputValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(int top READ top WRITE setTop)
    Q_PROPERTY(int bottom READ bottom WRITE setBottom)

public:
    explicit IntInputValidator(QObject* parent = nullptr);

    void fixup(QString& string) const override;
    State validate(QString& inputStr, int& cursorPos) const override;

    int top() const;
    int bottom() const;

public slots:
    void setTop(int);
    void setBottom(int);

private:
    int m_top = 999;
    int m_bottom = -999;
};

#endif // INTINPUTVALIDATOR_H
