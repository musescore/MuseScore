/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "scorecomp.h"

#include <algorithm>
#include <cstdio>
#include <string_view>
#include <vector>

#include "global/io/buffer.h"
#include "global/serialization/textstream.h"

#include "scorerw.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::io;

struct Edit {
    enum class Type {
        Equal,
        Delete,
        Insert,
    };

    Type type = Type::Equal;
    size_t idx1 = 0;
    size_t idx2 = 0;
};

static std::vector<std::string_view> splitLines(const ByteArray& data)
{
    const char* buf = data.constChar();
    const size_t size = data.size();

    std::vector<std::string_view> lines;
    size_t start = 0;

    for (size_t i = 0; i <= size; ++i) {
        if (i == size || buf[i] == '\n') {
            std::string_view line(buf + start, i - start);
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            lines.push_back(line);
            start = i + 1;
        }
    }

    if (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }

    return lines;
}

static std::vector<Edit> computeDiff(const std::vector<std::string_view>& lines1, const std::vector<std::string_view>& lines2)
{
    const size_t m = lines1.size();
    const size_t n = lines2.size();

    std::vector<std::vector<size_t> > dp(m + 1, std::vector<size_t>(n + 1, 0));
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (lines1[i - 1] == lines2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    std::vector<Edit> edits;
    size_t i = m;
    size_t j = n;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && lines1[i - 1] == lines2[j - 1]) {
            edits.push_back({ Edit::Type::Equal, i - 1, j - 1 });
            --i;
            --j;
        } else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j])) {
            edits.push_back({ Edit::Type::Insert, 0, j - 1 });
            --j;
        } else {
            edits.push_back({ Edit::Type::Delete, i - 1, 0 });
            --i;
        }
    }

    std::reverse(edits.begin(), edits.end());
    return edits;
}

bool ScoreComp::saveCompareScore(Score* score, const String& saveName, const String& compareWithLocalPath)
{
    if (!ScoreRW::saveScore(score, saveName)) {
        return false;
    }
    return ScoreComp::compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

bool ScoreComp::saveCompareMimeData(muse::ByteArray mimeData, const muse::String& saveName, const muse::String& compareWithLocalPath)
{
    if (!ScoreRW::saveMimeData(mimeData, saveName)) {
        return false;
    }
    return ScoreComp::compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

bool ScoreComp::compareFiles(const String& fullPath1, const String& fullPath2)
{
    TRACEFUNC;

    RetVal<ByteArray> rv1 = fileSystem()->readFile(fullPath1);
    if (!rv1.ret) {
        LOGE() << "Failed to read: " << fullPath1 << ", err: " << rv1.ret.toString();
        return false;
    }

    RetVal<ByteArray> rv2 = fileSystem()->readFile(fullPath2);
    if (!rv2.ret) {
        LOGE() << "Failed to read: " << fullPath2 << ", err: " << rv2.ret.toString();
        return false;
    }

    const std::vector<std::string_view> lines1 = splitLines(rv1.val);
    const std::vector<std::string_view> lines2 = splitLines(rv2.val);
    if (lines1 == lines2) {
        return true;
    }

    ByteArray outputBuf;
    Buffer buf(&outputBuf);
    buf.open(IODevice::WriteOnly);
    TextStream ts(&buf);

    ts << "--- " << fullPath1 << "\n";
    ts << "+++ " << fullPath2 << "\n";

    const std::vector<Edit> edits = computeDiff(lines1, lines2);
    constexpr size_t CTX = 3;
    const size_t n = edits.size();

    size_t i = 0;
    while (i < n) {
        if (edits[i].type == Edit::Type::Equal) {
            ++i;
            continue;
        }

        const size_t hunkStart = i >= CTX ? i - CTX : 0;

        size_t pos = i;
        while (pos < n) {
            while (pos < n && edits[pos].type != Edit::Type::Equal) {
                ++pos;
            }
            size_t eqEnd = pos;
            while (eqEnd < n && edits[eqEnd].type == Edit::Type::Equal) {
                ++eqEnd;
            }
            if (eqEnd < n && (eqEnd - pos) < 2 * CTX) {
                pos = eqEnd;
            } else {
                break;
            }
        }

        const size_t hunkEnd = std::min(n, pos + CTX);

        size_t start1 = 0;
        size_t start2 = 0;
        size_t count1 = 0;
        size_t count2 = 0;
        bool start1Set = false;
        bool start2Set = false;
        for (size_t k = hunkStart; k < hunkEnd; ++k) {
            const Edit& e = edits[k];
            if (e.type != Edit::Type::Insert) {
                if (!start1Set) {
                    start1 = e.idx1;
                    start1Set = true;
                }
                ++count1;
            }
            if (e.type != Edit::Type::Delete) {
                if (!start2Set) {
                    start2 = e.idx2;
                    start2Set = true;
                }
                ++count2;
            }
        }

        const size_t headerStart1 = start1Set ? start1 + 1 : 0;
        const size_t headerStart2 = start2Set ? start2 + 1 : 0;

        ts << "@@ -" << static_cast<uint64_t>(headerStart1);
        if (count1 != 1) {
            ts << "," << static_cast<uint64_t>(count1);
        }
        ts << " +" << static_cast<uint64_t>(headerStart2);
        if (count2 != 1) {
            ts << "," << static_cast<uint64_t>(count2);
        }
        ts << " @@\n";

        for (size_t k = hunkStart; k < hunkEnd; ++k) {
            const Edit& e = edits[k];
            if (e.type == Edit::Type::Equal) {
                ts << " " << lines1[e.idx1] << "\n";
            } else if (e.type == Edit::Type::Delete) {
                ts << "-" << lines1[e.idx1] << "\n";
            } else {
                ts << "+" << lines2[e.idx2] << "\n";
            }
        }

        i = hunkEnd;
    }

    ts.flush();
    std::fwrite(outputBuf.constChar(), 1, outputBuf.size(), stdout);

    return false;
}
