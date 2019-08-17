/**
 * KQOAuth - An OAuth authentication library for Qt.
 *
 * Author: Johan Paul (johan.paul@gmail.com)
 *         http://www.johanpaul.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  KQOAuth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with KQOAuth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QString>
#include <QCryptographicHash>
#include <QByteArray>

#include <QtDebug>
#include "kqoauthutils.h"

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>


QString KQOAuthUtils::hmac_sha1(const QString &message, const QString &key)
{
    QByteArray keyBytes = key.toLatin1();
    int keyLength;              // Length of key word
    const int blockSize = 64;   // Both MD5 and SHA-1 have a block size of 64.

    keyLength = keyBytes.size();
    // If key is longer than block size, we need to hash the key
    if (keyLength > blockSize) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(keyBytes);
        keyBytes = hash.result();
    }

    /* http://tools.ietf.org/html/rfc2104  - (1) */
    // Create the opad and ipad for the hash function.
    QByteArray ipad;
    QByteArray opad;

    ipad.fill( 0, blockSize);
    opad.fill( 0, blockSize);

    ipad.replace(0, keyBytes.length(), keyBytes);
    opad.replace(0, keyBytes.length(), keyBytes);

    /* http://tools.ietf.org/html/rfc2104 - (2) & (5) */
    for (int i=0; i<64; i++) {
        ipad[i] = ipad[i] ^ 0x36;
        opad[i] = opad[i] ^ 0x5c;
    }

    QByteArray workArray;
    workArray.clear();

    workArray.append(ipad, 64);
    /* http://tools.ietf.org/html/rfc2104 - (3) */
    workArray.append(message.toLatin1());


    /* http://tools.ietf.org/html/rfc2104 - (4) */
    QByteArray sha1 = QCryptographicHash::hash(workArray, QCryptographicHash::Sha1);

    /* http://tools.ietf.org/html/rfc2104 - (6) */
    workArray.clear();
    workArray.append(opad, 64);
    workArray.append(sha1);

    sha1.clear();

    /* http://tools.ietf.org/html/rfc2104 - (7) */
    sha1 = QCryptographicHash::hash(workArray, QCryptographicHash::Sha1);
    return QString(sha1.toBase64());
}
