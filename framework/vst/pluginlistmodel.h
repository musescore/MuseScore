#ifndef MU_VST_PLUGINLISTMODEL_H
#define MU_VST_PLUGINLISTMODEL_H

#include "internal/vstscanner.h"

namespace mu {
namespace vst {
class PluginListModel : public QAbstractListModel
{
    Q_OBJECT

    enum {
        UidRole     = Qt::UserRole,
        NameRole    = Qt::UserRole + 1
    };

public:
    PluginListModel(QObject* parent = nullptr);
    PluginListModel(std::shared_ptr<VSTScanner> scaner, QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void update();
    const Plugin& item(unsigned int index);

private:
    std::shared_ptr<VSTScanner> m_scanner;
    QList<Plugin> m_plugins;
};
} // namespace vst
} // namespace mu

#endif // MU_VST_PLUGINLISTMODEL_H
