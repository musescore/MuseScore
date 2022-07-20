#ifndef MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H
#define MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "libmscore/engravingitem.h"

namespace mu::notation {
class AbstractElementPopupModel : public QObject
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, globalContext)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(PopupModelType modelType READ modelType CONSTANT)

public:
    enum class PopupModelType {
        TYPE_UNDEFINED = -1,
        TYPE_HARP_DIAGRAM
    };
    Q_ENUM(PopupModelType)

    explicit AbstractElementPopupModel(QObject* parent = nullptr,
                                       mu::engraving::ElementType elementType = mu::engraving::ElementType::INVALID);
    QString title() const;
    PopupModelType modelType() const;

    static PopupModelType modelTypeFromElement(const mu::engraving::ElementType& elementType);

public slots:
    void setTitle(QString title);
    void setModelType(PopupModelType modelType);

signals:
    void titleChanged();

protected:
    void setElementType(mu::engraving::ElementType type);

private:
    QString m_title;
    PopupModelType m_modelType = PopupModelType::TYPE_UNDEFINED;
    mu::engraving::ElementType m_elementType = mu::engraving::ElementType::INVALID;
};

using PopupModelType = AbstractElementPopupModel::PopupModelType;
} //namespace mu::notation

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::notation::PopupModelType)
#endif

#endif // MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H
