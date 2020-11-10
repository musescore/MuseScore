#ifndef MU_IMPORTEXPORT_EXPORTXML_H
#define MU_IMPORTEXPORT_EXPORTXML_H

class QIODevice;

namespace Ms {
class Score;

bool saveMxl(Score*, QIODevice*);
bool saveXml(Score*, QIODevice*);
}

#endif // MU_IMPORTEXPORT_EXPORTXML_H
