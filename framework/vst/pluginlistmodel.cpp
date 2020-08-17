#include "pluginlistmodel.h"
#include "log.h"

using namespace mu::vst;

PluginListModel::PluginListModel(QObject* parent)
    : QAbstractListModel(parent), m_scanner(), m_plugins()
{
}

PluginListModel::PluginListModel(std::shared_ptr<VSTScanner> scaner, QObject* parent)
    : QAbstractListModel(parent), m_scanner(scaner), m_plugins()
{
    update();
}

Qt::ItemFlags PluginListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !m_scanner) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable;
}

int PluginListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_scanner) {
        return 0;
    }

    return m_scanner->getPlugins().size();
}

QHash<int, QByteArray> PluginListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[NameRole] = "name";
    names[UidRole]  = "uid";
    return names;
}

QVariant PluginListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_scanner) {
        return QVariant();
    }

    if (index.row() >= m_plugins.size()) {
        LOGE() << "wrong plugin index";
        return QVariant();
    }

    auto plugin = m_plugins.at(index.row());
    QVariant r;
    switch (role) {
    case NameRole:
        r.setValue(QString::fromStdString(plugin.getName()));
        break;
    case UidRole:
        r.setValue(QString::fromStdString(plugin.getId()));
        break;
    }

    return r;
}

void PluginListModel::update()
{
    m_plugins.clear();
    m_scanner->scan();
    for (auto p : m_scanner->getPlugins()) {
        m_plugins.push_back(p.second);
    }
}

static const Plugin nullPlugin;
const Plugin& PluginListModel::item(unsigned int index)
{
    auto size = static_cast<unsigned int>(m_plugins.size());
    IF_ASSERT_FAILED(index < size) {
        LOGE() << "index out of range";
        return nullPlugin;
    }
    return m_plugins.at(index);
}
