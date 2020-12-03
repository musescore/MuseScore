#ifndef ELEMENTREPOSITORYSERVICE_H
#define ELEMENTREPOSITORYSERVICE_H

#include "internal/interfaces/ielementrepositoryservice.h"

#include <QObject>

class ElementRepositoryService : public QObject, public IElementRepositoryService
{
    Q_OBJECT
public:
    explicit ElementRepositoryService(QObject* parent);

    QObject* getQObject() override;

    void updateElementList(const QList<Ms::Element*>& newRawElementList) override;
    QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType) const override;
    QList<Ms::Element*> findElementsByType(const Ms::ElementType elementType,
                                           std::function<bool(const Ms::Element*)> filterFunc) const override;
    QList<Ms::Element*> takeAllElements() const override;

signals:
    void elementsUpdated() override;

private:
    QList<Ms::Element*> m_elementList;

    QList<Ms::Element*> exposeRawElements(const QList<Ms::Element*>& rawElementList) const;

    QList<Ms::Element*> findChords() const;
    QList<Ms::Element*> findNotes() const;
    QList<Ms::Element*> findStems() const;
    QList<Ms::Element*> findHooks() const;
    QList<Ms::Element*> findBeams() const;
    QList<Ms::Element*> findGlissandos() const;
    QList<Ms::Element*> findHairpins() const;
    QList<Ms::Element*> findStaffs() const;
    QList<Ms::Element*> findSectionBreaks() const;
    QList<Ms::Element*> findPedals() const;
    QList<Ms::Element*> findPairedClefs() const;
    QList<Ms::Element*> findTexts() const;
    QList<Ms::Element*> findTremolos() const;
};

#endif // ELEMENTREPOSITORYSERVICE_H
