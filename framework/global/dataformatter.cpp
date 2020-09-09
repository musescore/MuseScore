#include "dataformatter.h"

#include <QString>
#include <QDateTime>

#include "translation.h"

using namespace mu;

double DataFormatter::formatDouble(const double& val, const int decimals)
{
    return QString::number(val, 'f', decimals).toDouble();
}

QString DataFormatter::formatTimeSinceCreation(const QDate& creationDate)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // ToDo for Qt 5.15: QDateTime::QDateTime() vs. QDate::startOfDay() (available as of Qt 5.14) ??
    int days = QDateTime(creationDate).daysTo(currentDateTime);

    if (days == 0) {
        return qtrc("global", "Today");
    }

    if (days == 1) {
        return qtrc("global", "Yesterday");
    }

    if (days < 7) {
        return qtrc("global", "%1 days ago").arg(days);
    }

    int weeks = days / 7;

    if (weeks == 1) {
        return qtrc("global", "Last week");
    }

    if (weeks == 2) {
        return qtrc("global", "Two weeks ago");
    }

    if (weeks == 3) {
        return qtrc("global", "Three weeks ago");
    }

    if (weeks == 4) {
        return qtrc("global", "Four weeks ago");
    }

    QDate currentDate = currentDateTime.date();
    constexpr int monthInYear = 12;
    int months = (currentDate.year() - creationDate.year()) * monthInYear + (currentDate.month() - creationDate.month());

    if (months == 1) {
        return qtrc("global", "Last month");
    }

    if (months < monthInYear) {
        return qtrc("global", "%1 months ago").arg(months);
    }

    int years = currentDate.year() - creationDate.year();
    if (years == 1) {
        return qtrc("global", "1 year ago");
    }

    return qtrc("global", "%1 years ago").arg(years);
}
