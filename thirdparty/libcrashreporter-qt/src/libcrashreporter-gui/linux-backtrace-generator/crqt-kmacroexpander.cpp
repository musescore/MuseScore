/*
    This file is part of the KDE libraries

    Copyright (c) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "crqt-kmacroexpander_p.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>

KMacroExpanderBase::KMacroExpanderBase(QChar c) : d(new KMacroExpanderBasePrivate(c))
{
}

KMacroExpanderBase::~KMacroExpanderBase()
{
    delete d;
}

void
KMacroExpanderBase::setEscapeChar(QChar c)
{
    d->escapechar = c;
}

QChar
KMacroExpanderBase::escapeChar() const
{
    return d->escapechar;
}

void KMacroExpanderBase::expandMacros(QString &str)
{
    int pos;
    int len;
    ushort ec = d->escapechar.unicode();
    QStringList rst;
    QString rsts;

    for (pos = 0; pos < str.length();) {
        if (ec != 0) {
            if (str.unicode()[pos].unicode() != ec) {
                goto nohit;
            }
            if (!(len = expandEscapedMacro(str, pos, rst))) {
                goto nohit;
            }
        } else {
            if (!(len = expandPlainMacro(str, pos, rst))) {
                goto nohit;
            }
        }
        if (len < 0) {
            pos -= len;
            continue;
        }
        rsts = rst.join(QLatin1Char(' '));
        rst.clear();
        str.replace(pos, len, rsts);
        pos += rsts.length();
        continue;
    nohit:
        pos++;
    }
}

bool KMacroExpanderBase::expandMacrosShellQuote(QString &str)
{
    int pos = 0;
    return expandMacrosShellQuote(str, pos) && pos == str.length();
}

int KMacroExpanderBase::expandPlainMacro(const QString &, int, QStringList &)
{
    qFatal("KMacroExpanderBase::expandPlainMacro called!");
    return 0;
}

int KMacroExpanderBase::expandEscapedMacro(const QString &, int, QStringList &)
{
    qFatal("KMacroExpanderBase::expandEscapedMacro called!");
    return 0;
}

//////////////////////////////////////////////////

template <typename KT, typename VT>
class KMacroMapExpander : public KMacroExpanderBase
{

public:
    KMacroMapExpander(const QHash<KT, VT> &map, QChar c = QLatin1Char('%')) :
        KMacroExpanderBase(c), macromap(map) {}

protected:
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;

private:
    QHash<KT, VT> macromap;
};

static QStringList &operator+=(QStringList &s, const QString &n)
{
    s << n;
    return s;
}

////////

static bool
isIdentifier(ushort c)
{
    return c == '_' ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

////////

template <typename VT>
class KMacroMapExpander<QChar, VT> : public KMacroExpanderBase
{

public:
    KMacroMapExpander(const QHash<QChar, VT> &map, QChar c = QLatin1Char('%')) :
        KMacroExpanderBase(c), macromap(map) {}

protected:
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;

private:
    QHash<QChar, VT> macromap;
};

template <typename VT>
int
KMacroMapExpander<QChar, VT>::expandPlainMacro(const QString &str, int pos, QStringList &ret)
{
    typename QHash<QChar, VT>::const_iterator it = macromap.constFind(str.unicode()[pos]);
    if (it != macromap.constEnd()) {
        ret += it.value();
        return 1;
    }
    return 0;
}

template <typename VT>
int
KMacroMapExpander<QChar, VT>::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    if (str.length() <= pos + 1) {
        return 0;
    }

    if (str.unicode()[pos + 1] == escapeChar()) {
        ret += QString(escapeChar());
        return 2;
    }
    typename QHash<QChar, VT>::const_iterator it = macromap.constFind(str.unicode()[pos + 1]);
    if (it != macromap.constEnd()) {
        ret += it.value();
        return 2;
    }

    return 0;
}

template <typename VT>
class KMacroMapExpander<QString, VT> : public KMacroExpanderBase
{

public:
    KMacroMapExpander(const QHash<QString, VT> &map, QChar c = QLatin1Char('%')) :
        KMacroExpanderBase(c), macromap(map) {}

protected:
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;

private:
    QHash<QString, VT> macromap;
};

template <typename VT>
int
KMacroMapExpander<QString, VT>::expandPlainMacro(const QString &str, int pos, QStringList &ret)
{
    if (pos && isIdentifier(str.unicode()[pos - 1].unicode())) {
        return 0;
    }
    int sl;
    for (sl = 0; isIdentifier(str.unicode()[pos + sl].unicode()); sl++)
        ;
    if (!sl) {
        return 0;
    }
    typename QHash<QString, VT>::const_iterator it =
        macromap.constFind(str.mid(pos, sl));
    if (it != macromap.constEnd()) {
        ret += it.value();
        return sl;
    }
    return 0;
}

template <typename VT>
int
KMacroMapExpander<QString, VT>::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    if (str.length() <= pos + 1) {
        return 0;
    }

    if (str.unicode()[pos + 1] == escapeChar()) {
        ret += QString(escapeChar());
        return 2;
    }
    int sl, rsl, rpos;
    if (str.unicode()[pos + 1].unicode() == '{') {
        rpos = pos + 2;
        if ((sl = str.indexOf(QLatin1Char('}'), rpos)) < 0) {
            return 0;
        }
        sl -= rpos;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier(str.unicode()[rpos + sl].unicode()); ++sl)
            ;
        rsl = sl + 1;
    }
    if (!sl) {
        return 0;
    }
    typename QHash<QString, VT>::const_iterator it =
        macromap.constFind(str.mid(rpos, sl));
    if (it != macromap.constEnd()) {
        ret += it.value();
        return rsl;
    }
    return 0;
}

////////////

int
KCharMacroExpander::expandPlainMacro(const QString &str, int pos, QStringList &ret)
{
    if (expandMacro(str.unicode()[pos], ret)) {
        return 1;
    }
    return 0;
}

int
KCharMacroExpander::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    if (str.length() <= pos + 1) {
        return 0;
    }

    if (str.unicode()[pos + 1] == escapeChar()) {
        ret += QString(escapeChar());
        return 2;
    }
    if (expandMacro(str.unicode()[pos + 1], ret)) {
        return 2;
    }
    return 0;
}

int
KWordMacroExpander::expandPlainMacro(const QString &str, int pos, QStringList &ret)
{
    if (pos && isIdentifier(str.unicode()[pos - 1].unicode())) {
        return 0;
    }
    int sl;
    for (sl = 0; isIdentifier(str.unicode()[pos + sl].unicode()); sl++)
        ;
    if (!sl) {
        return 0;
    }
    if (expandMacro(str.mid(pos, sl), ret)) {
        return sl;
    }
    return 0;
}

int
KWordMacroExpander::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    if (str.length() <= pos + 1) {
        return 0;
    }

    if (str.unicode()[pos + 1] == escapeChar()) {
        ret += QString(escapeChar());
        return 2;
    }
    int sl, rsl, rpos;
    if (str.unicode()[pos + 1].unicode() == '{') {
        rpos = pos + 2;
        if ((sl = str.indexOf(QLatin1Char('}'), rpos)) < 0) {
            return 0;
        }
        sl -= rpos;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier(str.unicode()[rpos + sl].unicode()); ++sl)
            ;
        rsl = sl + 1;
    }
    if (!sl) {
        return 0;
    }
    if (expandMacro(str.mid(rpos, sl), ret)) {
        return rsl;
    }
    return 0;
}

////////////

template <typename KT, typename VT>
inline QString
TexpandMacros(const QString &ostr, const QHash<KT, VT> &map, QChar c)
{
    QString str(ostr);
    KMacroMapExpander<KT, VT> kmx(map, c);
    kmx.expandMacros(str);
    return str;
}

template <typename KT, typename VT>
inline QString
TexpandMacrosShellQuote(const QString &ostr, const QHash<KT, VT> &map, QChar c)
{
    QString str(ostr);
    KMacroMapExpander<KT, VT> kmx(map, c);
    if (!kmx.expandMacrosShellQuote(str)) {
        return QString();
    }
    return str;
}

// public API
namespace KMacroExpander
{

QString expandMacros(const QString &ostr, const QHash<QChar, QString> &map, QChar c)
{
    return TexpandMacros(ostr, map, c);
}
QString expandMacrosShellQuote(const QString &ostr, const QHash<QChar, QString> &map, QChar c)
{
    return TexpandMacrosShellQuote(ostr, map, c);
}
QString expandMacros(const QString &ostr, const QHash<QString, QString> &map, QChar c)
{
    return TexpandMacros(ostr, map, c);
}
QString expandMacrosShellQuote(const QString &ostr, const QHash<QString, QString> &map, QChar c)
{
    return TexpandMacrosShellQuote(ostr, map, c);
}
QString expandMacros(const QString &ostr, const QHash<QChar, QStringList> &map, QChar c)
{
    return TexpandMacros(ostr, map, c);
}
QString expandMacrosShellQuote(const QString &ostr, const QHash<QChar, QStringList> &map, QChar c)
{
    return TexpandMacrosShellQuote(ostr, map, c);
}
QString expandMacros(const QString &ostr, const QHash<QString, QStringList> &map, QChar c)
{
    return TexpandMacros(ostr, map, c);
}
QString expandMacrosShellQuote(const QString &ostr, const QHash<QString, QStringList> &map, QChar c)
{
    return TexpandMacrosShellQuote(ostr, map, c);
}

} // namespace
