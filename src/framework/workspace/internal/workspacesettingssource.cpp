#include "workspacesettingssource.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::framework;
using namespace mu::async;

void WorkspaceSettingsSource::init()
{
    RetValCh<IWorkspacePtr> currentWorkspace = workspaceManager()->currentWorkspace();

    if (!currentWorkspace.ret) {
        LOGE() << currentWorkspace.ret.toString();
        return;
    }

    m_currentWorkspace = currentWorkspace.val;

    currentWorkspace.ch.onReceive(this, [this](const IWorkspacePtr& workspace) {
        m_currentWorkspace = workspace;
        m_sourceChanged.notify();
    });
}

bool WorkspaceSettingsSource::hasValue(const std::string& key) const
{
    return !value(key).isNull();
}

mu::Val WorkspaceSettingsSource::value(const std::string& key) const
{
    if (!m_currentWorkspace) {
        return Val();
    }

    SettingsDataPtr settings = std::dynamic_pointer_cast<SettingsData>(m_currentWorkspace->data(WorkspaceTag::Settings));
    IF_ASSERT_FAILED(settings) {
        return Val();
    }

    auto it = settings->vals.find(key);
    if (it == settings->vals.end()) {
        return Val();
    }

    return it->second;
}

void WorkspaceSettingsSource::setValue(const std::string &key, const Val& value)
{
    if (!m_currentWorkspace) {
        return;
    }

    SettingsDataPtr settings = std::dynamic_pointer_cast<SettingsData>(m_currentWorkspace->data(WorkspaceTag::Settings));
    IF_ASSERT_FAILED(settings) {
        return;
    }

    settings->vals[key] = value;
    m_currentWorkspace->addData(settings);
}

Notification WorkspaceSettingsSource::sourceChanged() const
{
    return m_sourceChanged;
}
