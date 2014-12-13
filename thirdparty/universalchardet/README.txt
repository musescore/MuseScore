It's a C++ implementation of the universal charset detection library by Mozilla.

universalchardet is an encoding detector library, which takes a sequence of bytes in an unknown character encoding without any additional information, and attempts to determine the encoding of the text.

The original code of universalchardet is available at http://lxr.mozilla.org/seamonkey/source/extensions/universalchardet/

Techniques used by universalchardet are described at http://www.mozilla.org/projects/intl/UniversalCharsetDetection.html

Supported Encodings

Unicode
    UTF-8
    UTF-16BE / UTF-16LE
    UTF-32BE / UTF-32LE / X-ISO-10646-UCS-4-34121 / X-ISO-10646-UCS-4-21431
Chinese
    ISO-2022-CN
    BIG5
    EUC-TW
    GB18030
    HZ-GB-23121
Japanese
    ISO-2022-JP
    SHIFT_JIS
    EUC-JP
Korean
    ISO-2022-KR
    EUC-KR
Cyrillic
    ISO-8859-5
    KOI8-R
    WINDOWS-1251
    MACCYRILLIC
    IBM866
    IBM855
Greek
    ISO-8859-7
    WINDOWS-1253
Hebrew
    ISO-8859-8
    WINDOWS-1255
Others
    WINDOWS-1252


-------------------------------------------------------------------------------
Mozilla notes


For information on how to build Mozilla from the source code, see:

    http://developer.mozilla.org/en/docs/Build_Documentation

To have your bug fix / feature added to Mozilla, you should create a patch and
submit it to Bugzilla (http://bugzilla.mozilla.org). Instructions are at:

    http://developer.mozilla.org/en/docs/Creating_a_patch
    http://developer.mozilla.org/en/docs/Getting_your_patch_in_the_tree

If you have a question about developing Mozilla, and can't find the solution
on http://developer.mozilla.org, you can try asking your question in a
mozilla.* Usenet group, or on IRC at irc.mozilla.org. [The Mozilla news groups
are accessible on Google Groups, or news.mozilla.org with a NNTP reader.]

You can download nightly development builds from the the Mozilla FTP server.
Keep in mind that nightly builds, which are used by Mozilla developers for
testing, may be buggy. Firefox nightlies, for example, can be found at:

    ftp://ftp.mozilla.org/pub/firefox/nightly/latest-trunk/
