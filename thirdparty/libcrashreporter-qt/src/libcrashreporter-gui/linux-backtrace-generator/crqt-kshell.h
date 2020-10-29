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
#ifndef KSHELL_H
#define KSHELL_H

#include <qglobal.h>

class QStringList;
class QString;

/**
 * \namespace KShell
 * Emulates some basic system shell functionality.
 * @see KStringHandler
 */
namespace KShell
{

/**
 * Flags for splitArgs().
 */
enum Option {
    NoOptions = 0,

    /**
     * Perform tilde expansion.
     * On Windows, this flag is ignored, as the Windows shell has no
     * equivalent functionality.
     */
    TildeExpand = 1,

    /**
     * Put the parser into full shell mode and bail out if a too complex
     * construct is encountered.
     * A particular purpose of this flag is finding out whether the
     * command line being split would be executable directly (via
     * KProcess::setProgram()) or whether it needs to be run through
     * a real shell (via KProcess::setShellCommand()). Note, however,
     * that shell builtins are @em not recognized - you need to do that
     * yourself (compare with a list of known commands or verify that an
     * executable exists for the named command).
     *
     * Meta characters that cause a bail-out are the command separators
     * @c semicolon and @c ampersand, the redirection symbols @c less-than,
     * @c greater-than and the @c pipe @c symbol and the grouping symbols
     * opening and closing @c parentheses.
     *
     * Further meta characters on *NIX are the grouping symbols
     * opening and closing @c braces, the command substitution symbol
     * @c backquote, the generic substitution symbol @c dollar (if
     * not followed by an apostrophe), the wildcards @c asterisk,
     * @c question @c mark and opening and closing @c square @c brackets
     * and the comment symbol @c hash @c mark.
     * Additionally, a variable assignment in the first word is recognized.
     *
     * A further meta character on Windows is the environment variable
     * expansion symbol @c percent. Occurrences of @c \%PERCENT_SIGN% as
     * inserted by quoteArg() are converted back and cause no bail-out,
     * though.
     */
    AbortOnMeta = 2
};
Q_DECLARE_FLAGS(Options, Option)

/**
 * Status codes from splitArgs()
 */
enum Errors {
    /**
     * Success.
     */
    NoError = 0,

    /**
     * Indicates a parsing error, like an unterminated quoted string.
     */
    BadQuoting,

    /**
     * The AbortOnMeta flag was set and an unhandled shell meta character
     * was encountered.
     */
    FoundMeta
};

/**
 * Splits @p cmd according to system shell word splitting and quoting rules.
 * Can optionally perform tilde expansion and/or abort if it finds shell
 * meta characters it cannot process.
 *
 * On *NIX the behavior is based on the POSIX shell and bash:
 * - Whitespace splits tokens
 * - The backslash quotes the following character
 * - A string enclosed in single quotes is not split. No shell meta
 *   characters are interpreted.
 * - A string enclosed in double quotes is not split. Within the string,
 *   the backslash quotes shell meta characters - if it is followed
 *   by a "meaningless" character, the backslash is output verbatim.
 * - A string enclosed in $'' is not split. Within the string, the
 *   backslash has a similar meaning to the one in C strings. Consult
 *   the bash manual for more information.
 *
 * On Windows, the behavior is defined by the Microsoft C runtime. Qt and
 * many other implementations comply with this standard, but many do not.
 * - Whitespace splits tokens
 * - A string enclosed in double quotes is not split
 *   - 2N double quotes within a quoted string yield N literal quotes.
 *     This is not documented on MSDN.
 * - Backslashes have special semantics iff they are followed by a double
 *   quote:
 *   - 2N backslashes + double quote => N backslashes and begin/end quoting
 *   - 2N+1 backslashes + double quote => N backslashes + literal quote
 *
 * If AbortOnMeta is used on Windows, this function applies cmd shell
 * semantics before proceeding with word splitting:
 * - Cmd ignores @em all special chars between double quotes.
 *   Note that the quotes are @em not removed at this stage - the
 *   tokenization rules described above still apply.
 * - The @c circumflex is the escape char for everything including
 *   itself.
 *
 * @param cmd the command to split
 * @param flags operation flags, see \ref Option
 * @param err if not NULL, a status code will be stored at the pointer
 *  target, see \ref Errors
 * @return a list of unquoted words or an empty list if an error occurred
 */
QStringList splitArgs(const QString &cmd, Options flags = NoOptions, Errors *err = 0);

/**
 * Quotes and joins @p args together according to system shell rules.
 *
 * If the output is fed back into splitArgs(), the AbortOnMeta flag
 * needs to be used on Windows. On *NIX, no such requirement exists.
 *
 * See quoteArg() for more info.
 *
 * @param args a list of strings to quote and join
 * @return a command suitable for shell execution
 */
QString joinArgs(const QStringList &args);

/**
 * Quotes @p arg according to system shell rules.
 *
 * This function can be used to quote an argument string such that
 * the shell processes it properly. This is e.g. necessary for
 * user-provided file names which may contain spaces or quotes.
 * It also prevents expansion of wild cards and environment variables.
 *
 * On *NIX, the output is POSIX shell compliant.
 * On Windows, it is compliant with the argument splitting code of the
 * Microsoft C runtime and the cmd shell used together.
 * Occurrences of the @c percent @c sign are replaced with
 * @c \%PERCENT_SIGN% to prevent spurious variable expansion;
 * related KDE functions are prepared for this.
 *
 * @param arg the argument to quote
 * @return the quoted argument
 */
QString quoteArg(const QString &arg);

/**
 * Performs tilde expansion on @p path. Interprets "~/path" and
 * "~user/path". If the path starts with an escaped tilde ("\~" on UNIX,
 * "^~" on Windows), the escape char is removed and the path is returned
 * as is.
 *
 * Note that if @p path starts with a tilde but cannot be properly expanded,
 * this function will return an empty string.
 *
 * @param path the path to tilde-expand
 * @return the expanded path
 */
QString tildeExpand(const QString &path);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KShell::Options)

#endif /* KSHELL_H */
