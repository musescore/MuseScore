#ifndef IMPORTMIDI_CHARSET_H
#define IMPORTMIDI_CHARSET_H


namespace Ms {
namespace MidiCharset {

QString convertToCharset(const std::string &text);
QString defaultCharset();
std::string fromUchar(const uchar *text);

} // namespace MidiCharset
} // namespace Ms


#endif // IMPORTMIDI_CHARSET_H
