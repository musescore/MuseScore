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
#ifndef KMACROEXPANDER_H
#define KMACROEXPANDER_H

#include <QtCore/QChar>

class QString;
class QStringList;
template <typename KT, typename VT> class QHash;
class KMacroExpanderBasePrivate;

/**
 * \class KMacroExpanderBase kmacroexpander.h <KMacroExpanderBase>
 *
 * Abstract base class for the worker classes behind the KMacroExpander namespace
 * and the KCharMacroExpander and KWordMacroExpander classes.
 *
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class KMacroExpanderBase
{

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or QChar::null for none
     */
    explicit KMacroExpanderBase(QChar c = QLatin1Char('%'));

    /**
     * Destructor.
     */
    virtual ~KMacroExpanderBase();

    /**
     * Perform safe macro expansion (substitution) on a string.
     *
     * @param str the string in which macros are expanded in-place
     */
    void expandMacros(QString &str);

    // TODO: This documentation is relevant for end-users. Where to put it?
    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * <h3>*NIX notes</h3>
     *
     * Explicitly supported shell constructs:
     *   \ '' "" $'' $"" {} () $(()) ${} $() ``
     *
     * Implicitly supported shell constructs:
     *   (())
     *
     * Unsupported shell constructs that will cause problems:
     *  Shortened &quot;<tt>case $v in pat)</tt>&quot; syntax. Use
     *   &quot;<tt>case $v in (pat)</tt>&quot; instead.
     *
     * The rest of the shell (incl. bash) syntax is simply ignored,
     * as it is not expected to cause problems.
     *
     * Note that bash contains a bug which makes macro expansion within
     * double quoted substitutions (<tt>"${VAR:-%macro}"</tt>) inherently
     * insecure.
     *
     * For security reasons, @em never put expandos in command line arguments
     * that are shell commands by themselves -
     * &quot;<tt>sh -c 'foo \%f'</tt>&quot; is taboo.
     * &quot;<tt>file=\%f sh -c 'foo "$file"'</tt>&quot; is OK.
     *
     * <h3>Windows notes</h3>
     *
     * All quoting syntax supported by KShell is supported here as well.
     * Additionally, command grouping via parentheses is recognized - note
     * however, that the parser is much stricter about unquoted parentheses
     * than cmd itself.
     * The rest of the cmd syntax is simply ignored, as it is not expected
     * to cause problems - do not use commands that embed other commands,
     * though - &quot;<tt>for /f ...</tt>&quot; is taboo.
     *
     * @param str the string in which macros are expanded in-place
     * @param pos the position inside the string at which parsing/substitution
     *  should start, and upon exit where processing stopped
     * @return false if the string could not be parsed and therefore no safe
     *  substitution was possible. Note that macros will have been processed
     *  up to the point where the error occurred. An unmatched closing paren
     *  or brace outside any shell construct is @em not an error (unlike in
     *  the function below), but still prematurely terminates processing.
     */
    bool expandMacrosShellQuote(QString &str, int &pos);

    /**
     * Same as above, but always starts at position 0, and unmatched closing
     * parens and braces are treated as errors.
     */
    bool expandMacrosShellQuote(QString &str);

    /**
     * Set the macro escape character.
     * @param c escape char indicating start of macros, or QChar::null if none
     */
    void setEscapeChar(QChar c);

    /**
     * Obtain the macro escape character.
     * @return escape char indicating start of macros, or QChar::null if none
     */
    QChar escapeChar() const;

protected:
    /**
     * This function is called for every single char within the string if
     * the escape char is QChar::null. It should determine whether the
     * string starting at @p pos within @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str
     * @param ret return value: the string to substitute for the macro
     * @return If greater than zero, the number of chars at @p pos in @p str
     *  to substitute with @p ret (i.e., a valid macro was found). If less
     *  than zero, subtract this value from @p pos (to skip a macro, i.e.,
     *  substitute it with itself). If zero, no macro starts at @p pos.
     */
    virtual int expandPlainMacro(const QString &str, int pos, QStringList &ret);

    /**
     * This function is called every time the escape char is found if it is
     * not QChar::null. It should determine whether the
     * string starting at @p pos witin @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str. Note that this is the position of
     *  the occurrence of the escape char
     * @param ret return value: the string to substitute for the macro
     * @return If greater than zero, the number of chars at @p pos in @p str
     *  to substitute with @p ret (i.e., a valid macro was found). If less
     *  than zero, subtract this value from @p pos (to skip a macro, i.e.,
     *  substitute it with itself). If zero, scanning continues as if no
     *  escape char was encountered at all.
     */
    virtual int expandEscapedMacro(const QString &str, int pos, QStringList &ret);

private:
    KMacroExpanderBasePrivate *const d;
};

/**
 * \class KWordMacroExpander kmacroexpander.h <KMacroExpanderBase>
 *
 * Abstract base class for simple word macro substitutors. Use this instead of
 * the functions in the KMacroExpander namespace if speculatively pre-filling
 * the substitution map would be too expensive.
 *
 * A typical application:
 *
 * \code
 * class MyClass {
 * ...
 *   private:
 *     QString m_str;
 * ...
 *   friend class MyExpander;
 * };
 *
 * class MyExpander : public KWordMacroExpander {
 *   public:
 *     MyExpander( MyClass *_that ) : KWordMacroExpander(), that( _that ) {}
 *   protected:
 *     virtual bool expandMacro( const QString &str, QStringList &ret );
 *   private:
 *     MyClass *that;
 * };
 *
 * bool MyExpander::expandMacro( const QString &str, QStringList &ret )
 * {
 *   if (str == "macro") {
 *     ret += complexOperation( that->m_str );
 *     return true;
 *   }
 *   return false;
 * }
 *
 * ... MyClass::...(...)
 * {
 *   QString str;
 *   ...
 *   MyExpander mx( this );
 *   mx.expandMacrosShellQuote( str );
 *   ...
 * }
 * \endcode
 *
 * Alternatively MyClass could inherit from KWordMacroExpander directly.
 *
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class KWordMacroExpander : public KMacroExpanderBase
{

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or QChar::null for none
     */
    explicit KWordMacroExpander(QChar c = QLatin1Char('%')) : KMacroExpanderBase(c) {}

protected:
    /** \internal Not to be called or reimplemented. */
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;
    /** \internal Not to be called or reimplemented. */
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;

    /**
     * Return substitution list @p ret for string macro @p str.
     * @param str the macro to expand
     * @param ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     * @return @c true iff @p chr was a recognized macro name
     */
    virtual bool expandMacro(const QString &str, QStringList &ret) = 0;
};

/**
 * \class KCharMacroExpander kmacroexpander.h <KMacroExpanderBase>
 *
 * Abstract base class for single char macro substitutors. Use this instead of
 * the functions in the KMacroExpander namespace if speculatively pre-filling
 * the substitution map would be too expensive.
 *
 * See KWordMacroExpander for a sample application.
 *
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class KCharMacroExpander : public KMacroExpanderBase
{

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or QChar::null for none
     */
    explicit KCharMacroExpander(QChar c = QLatin1Char('%')) : KMacroExpanderBase(c) {}

protected:
    /** \internal Not to be called or reimplemented. */
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;
    /** \internal Not to be called or reimplemented. */
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) Q_DECL_OVERRIDE;

    /**
     * Return substitution list @p ret for single-character macro @p chr.
     * @param chr the macro to expand
     * @param ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     * @return @c true iff @p chr was a recognized macro name
     */
    virtual bool expandMacro(QChar chr, QStringList &ret) = 0;
};

/**
 * A group of functions providing macro expansion (substitution) in strings,
 * optionally with quoting appropriate for shell execution.
 */
namespace KMacroExpander
{
/**
 * Perform safe macro expansion (substitution) on a string.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 *
 * @param str The string to expand
 * @param map map with substitutions
 * @param c escape char indicating start of macro, or QChar::null if none
 * @return the string with all valid macros expanded
 *
 * \code
 * // Code example
 * QHash<QChar,QString> map;
 * map.insert('u', "/tmp/myfile.txt");
 * map.insert('n', "My File");
 * QString s = "%% Title: %u:%n";
 * s = KMacroExpander::expandMacros(s, map);
 * // s is now "% Title: /tmp/myfile.txt:My File";
 * \endcode
 */
QString expandMacros(const QString &str, const QHash<QChar, QString> &map, QChar c = QLatin1Char('%'));

/**
 * Perform safe macro expansion (substitution) on a string for use
 * in shell commands.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 *
 * @param str The string to expand
 * @param map map with substitutions
 * @param c escape char indicating start of macro, or QChar::null if none
 * @return the string with all valid macros expanded, or a null string
 *  if a shell syntax error was detected in the command
 *
 * \code
 * // Code example
 * QHash<QChar,QString> map;
 * map.insert('u', "/tmp/myfile.txt");
 * map.insert('n', "My File");
 * QString s = "kedit --caption %n %u";
 * s = KMacroExpander::expandMacrosShellQuote(s, map);
 * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
 * system(QFile::encodeName(s));
 * \endcode
 */
QString expandMacrosShellQuote(const QString &str, const QHash<QChar, QString> &map,
        QChar c = QLatin1Char('%'));

/**
 * Perform safe macro expansion (substitution) on a string.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 * Macro names can consist of chars in the range [A-Za-z0-9_];
 * use braces to delimit macros from following words starting
 * with these chars, or to use other chars for macro names.
 *
 * @param str The string to expand
 * @param map map with substitutions
 * @param c escape char indicating start of macro, or QChar::null if none
 * @return the string with all valid macros expanded
 *
 * \code
 * // Code example
 * QHash<QString,QString> map;
 * map.insert("url", "/tmp/myfile.txt");
 * map.insert("name", "My File");
 * QString s = "Title: %{url}-%name";
 * s = KMacroExpander::expandMacros(s, map);
 * // s is now "Title: /tmp/myfile.txt-My File";
 * \endcode
 */
QString expandMacros(const QString &str, const QHash<QString, QString> &map,
                                        QChar c = QLatin1Char('%'));

/**
 * Perform safe macro expansion (substitution) on a string for use
 * in shell commands. See KMacroExpanderBase::expandMacrosShellQuote()
 * for the exact semantics.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 * Macro names can consist of chars in the range [A-Za-z0-9_];
 * use braces to delimit macros from following words starting
 * with these chars, or to use other chars for macro names.
 *
 * @param str The string to expand
 * @param map map with substitutions
 * @param c escape char indicating start of macro, or QChar::null if none
 * @return the string with all valid macros expanded, or a null string
 *  if a shell syntax error was detected in the command
 *
 * \code
 * // Code example
 * QHash<QString,QString> map;
 * map.insert("url", "/tmp/myfile.txt");
 * map.insert("name", "My File");
 * QString s = "kedit --caption %name %{url}";
 * s = KMacroExpander::expandMacrosShellQuote(s, map);
 * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
 * system(QFile::encodeName(s));
 * \endcode
 */
QString expandMacrosShellQuote(const QString &str, const QHash<QString, QString> &map,
        QChar c = QLatin1Char('%'));

/**
 * Same as above, except that the macros expand to string lists that
 * are simply join(" ")ed together.
 */
QString expandMacros(const QString &str, const QHash<QChar, QStringList> &map,
                                        QChar c = QLatin1Char('%'));
QString expandMacros(const QString &str, const QHash<QString, QStringList> &map,
                                        QChar c = QLatin1Char('%'));

/**
 * Same as above, except that the macros expand to string lists.
 * If the macro appears inside a quoted string, the list is simply
 * join(" ")ed together; otherwise every element expands to a separate
 * quoted string.
 */
QString expandMacrosShellQuote(const QString &str, const QHash<QChar, QStringList> &map,
        QChar c = QLatin1Char('%'));
QString expandMacrosShellQuote(const QString &str, const QHash<QString, QStringList> &map,
        QChar c = QLatin1Char('%'));
}

#endif /* KMACROEXPANDER_H */
