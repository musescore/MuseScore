/*
    This file is part of the KDE libraries

    Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

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

#include "crqt-kshell.h"
#include "crqt-kshell_p.h"

#include <QtCore/QChar>
#include <QtCore/QStringList>

static int fromHex(QChar cUnicode)
{
    char c = cUnicode.toLatin1();

    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

inline static bool isQuoteMeta(QChar cUnicode)
{
#if 0 // it's not worth it, especially after seeing gcc's asm output ...
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00
    }; // \'"$

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
#else
    char c = cUnicode.toLatin1();
    return c == '\\' || c == '\'' || c == '"' || c == '$';
#endif
}

inline static bool isMeta(QChar cUnicode)
{
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0xdc, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x38
    }; // \'"$`<>|;&(){}*?#[]

    uint c = cUnicode.unicode();

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

QStringList KShell::splitArgs(const QString &args, Options flags, Errors *err)
{
    QStringList ret;
    bool firstword = flags & AbortOnMeta;

    for (int pos = 0;;) {
        QChar c;
        do {
            if (pos >= args.length()) {
                goto okret;
            }
            c = args.unicode()[pos++];
        } while (c == QLatin1Char(' '));
        QString cret;
        if ((flags & TildeExpand) && c == QLatin1Char('~')) {
            int opos = pos;
            for (;; pos++) {
                if (pos >= args.length()) {
                    break;
                }
                c = args.unicode()[pos];
                if (c == QLatin1Char('/') || c == QLatin1Char(' ')) {
                    break;
                }
                if (isQuoteMeta(c)) {
                    pos = opos;
                    c = QLatin1Char('~');
                    goto notilde;
                }
                if ((flags & AbortOnMeta) && isMeta(c)) {
                    goto metaerr;
                }
            }
            QString ccret = homeDir(args.mid(opos, pos - opos));
            if (ccret.isEmpty()) {
                pos = opos;
                c = QLatin1Char('~');
                goto notilde;
            }
            if (pos >= args.length()) {
                ret += ccret;
                goto okret;
            }
            pos++;
            if (c == QLatin1Char(' ')) {
                ret += ccret;
                firstword = false;
                continue;
            }
            cret = ccret;
        }
        // before the notilde label, as a tilde does not match anyway
        if (firstword) {
            if (c == QLatin1Char('_') ||
                    (c >= QLatin1Char('A') && c <= QLatin1Char('Z')) ||
                    (c >= QLatin1Char('a') && c <= QLatin1Char('z'))) {
                int pos2 = pos;
                QChar cc;
                do {
                    if (pos2 >= args.length()) {
                        // Exactly one word
                        ret += args.mid(pos - 1);
                        goto okret;
                    }
                    cc = args.unicode()[pos2++];
                } while (cc == QLatin1Char('_') ||
                         (cc >= QLatin1Char('A') && cc <= QLatin1Char('Z')) ||
                         (cc >= QLatin1Char('a') && cc <= QLatin1Char('z')) ||
                         (cc >= QLatin1Char('0') && cc <= QLatin1Char('9')));
                if (cc == QLatin1Char('=')) {
                    goto metaerr;
                }
            }
        }
    notilde:
        do {
            if (c == QLatin1Char('\'')) {
                int spos = pos;
                do {
                    if (pos >= args.length()) {
                        goto quoteerr;
                    }
                    c = args.unicode()[pos++];
                } while (c != QLatin1Char('\''));
                cret += args.mid(spos, pos - spos - 1);
            } else if (c == QLatin1Char('"')) {
                for (;;) {
                    if (pos >= args.length()) {
                        goto quoteerr;
                    }
                    c = args.unicode()[pos++];
                    if (c == QLatin1Char('"')) {
                        break;
                    }
                    if (c == QLatin1Char('\\')) {
                        if (pos >= args.length()) {
                            goto quoteerr;
                        }
                        c = args.unicode()[pos++];
                        if (c != QLatin1Char('"') &&
                                c != QLatin1Char('\\') &&
                                !((flags & AbortOnMeta) &&
                                  (c == QLatin1Char('$') ||
                                   c == QLatin1Char('`')))) {
                            cret += QLatin1Char('\\');
                        }
                    } else if ((flags & AbortOnMeta) &&
                               (c == QLatin1Char('$') ||
                                c == QLatin1Char('`'))) {
                        goto metaerr;
                    }
                    cret += c;
                }
            } else if (c == QLatin1Char('$') && pos < args.length() &&
                       args.unicode()[pos] == QLatin1Char('\'')) {
                pos++;
                for (;;) {
                    if (pos >= args.length()) {
                        goto quoteerr;
                    }
                    c = args.unicode()[pos++];
                    if (c == QLatin1Char('\'')) {
                        break;
                    }
                    if (c == QLatin1Char('\\')) {
                        if (pos >= args.length()) {
                            goto quoteerr;
                        }
                        c = args.unicode()[pos++];
                        switch (c.toLatin1()) {
                        case 'a': cret += QLatin1Char('\a'); break;
                        case 'b': cret += QLatin1Char('\b'); break;
                        case 'e': cret += QLatin1Char('\033'); break;
                        case 'f': cret += QLatin1Char('\f'); break;
                        case 'n': cret += QLatin1Char('\n'); break;
                        case 'r': cret += QLatin1Char('\r'); break;
                        case 't': cret += QLatin1Char('\t'); break;
                        case '\\': cret += QLatin1Char('\\'); break;
                        case '\'': cret += QLatin1Char('\''); break;
                        case 'c':
                            if (pos >= args.length()) {
                                goto quoteerr;
                            }
                            cret += args.unicode()[pos++].toLatin1() & 31;
                            break;
                        case 'x': {
                            if (pos >= args.length()) {
                                goto quoteerr;
                            }
                            int hv = fromHex(args.unicode()[pos++]);
                            if (hv < 0) {
                                goto quoteerr;
                            }
                            if (pos < args.length()) {
                                int hhv = fromHex(args.unicode()[pos]);
                                if (hhv > 0) {
                                    hv = hv * 16 + hhv;
                                    pos++;
                                }
                                cret += QChar(hv);
                            }
                            break;
                        }
                        default:
                            if (c.toLatin1() >= '0' && c.toLatin1() <= '7') {
                                char cAscii = c.toLatin1();
                                int hv = cAscii - '0';
                                for (int i = 0; i < 2; i++) {
                                    if (pos >= args.length()) {
                                        break;
                                    }
                                    c = args.unicode()[pos];
                                    if (c.toLatin1() < '0' || c.toLatin1() > '7') {
                                        break;
                                    }
                                    hv = hv * 8 + (c.toLatin1() - '0');
                                    pos++;
                                }
                                cret += QChar(hv);
                            } else {
                                cret += QLatin1Char('\\');
                                cret += c;
                            }
                            break;
                        }
                    } else {
                        cret += c;
                    }
                }
            } else {
                if (c == QLatin1Char('\\')) {
                    if (pos >= args.length()) {
                        goto quoteerr;
                    }
                    c = args.unicode()[pos++];
                } else if ((flags & AbortOnMeta) && isMeta(c)) {
                    goto metaerr;
                }
                cret += c;
            }
            if (pos >= args.length()) {
                break;
            }
            c = args.unicode()[pos++];
        } while (c != QLatin1Char(' '));
        ret += cret;
        firstword = false;
    }

okret:
    if (err) {
        *err = NoError;
    }
    return ret;

quoteerr:
    if (err) {
        *err = BadQuoting;
    }
    return QStringList();

metaerr:
    if (err) {
        *err = FoundMeta;
    }
    return QStringList();
}

inline static bool isSpecial(QChar cUnicode)
{
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xdf, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x38, 0x01, 0x00, 0x00, 0x78
    }; // 0-32 \'"$`<>|;&(){}*?#!~[]

    uint c = cUnicode.unicode();
    return ((c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7))));
}

QString KShell::quoteArg(const QString &arg)
{
    if (!arg.length()) {
        return QString::fromLatin1("''");
    }
    for (int i = 0; i < arg.length(); i++)
        if (isSpecial(arg.unicode()[i])) {
            QChar q(QLatin1Char('\''));
            return QString(arg).replace(q, QLatin1String("'\\''")).prepend(q).append(q);
        }
    return arg;
}
