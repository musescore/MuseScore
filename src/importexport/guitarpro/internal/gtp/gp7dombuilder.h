#ifndef GP7DOMBUILDER_H
#define GP7DOMBUILDER_H

#include "gpdommodel.h"
#include "gp67dombuilder.h"

namespace Ms {
class GP7DomBuilder : public GP67DomBuilder
{
public:
    GP7DomBuilder() = default;

private:
    std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(QDomNode* trackNode) override;

    int readMidiChannel(QDomNode* trackChildNode) const;
    int readMidiProgramm(QDomNode* trackChildNode) const;
};
} //end Ms namespace
#endif // GP7DOMBUILDER_H
