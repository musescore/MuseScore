#ifndef MU_VST_VSTCOMPONENTHANDLER_H
#define MU_VST_VSTCOMPONENTHANDLER_H

#include "async/notification.h"
#include "async/channel.h"

#include "vsttypes.h"

namespace mu::vst {
class VstAdvancedHandler : public IAdvancedComponentHandler
{
    DECLARE_FUNKNOWN_METHODS
public:
    VstAdvancedHandler(async::Notification notifier);
    virtual ~VstAdvancedHandler() = default;

    Steinberg::tresult setDirty(Steinberg::TBool state) override;
    Steinberg::tresult requestOpenEditor(Steinberg::FIDString name) override;
    Steinberg::tresult startGroupEdit() override;
    Steinberg::tresult finishGroupEdit() override;
private:
    async::Notification m_paramsChanged;
};

class VstComponentHandler : public IComponentHandler
{
    DECLARE_FUNKNOWN_METHODS
public:
    VstComponentHandler();
    virtual ~VstComponentHandler() = default;

    async::Channel<PluginParamId, PluginParamValue> pluginParamChanged() const;
    async::Notification pluginParamsChanged() const;

private:
    Steinberg::tresult beginEdit(Steinberg::Vst::ParamID id) override;
    Steinberg::tresult performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized) override;
    Steinberg::tresult endEdit(Steinberg::Vst::ParamID id) override;
    Steinberg::tresult restartComponent(Steinberg::int32 flags) override;

    async::Channel<PluginParamId, PluginParamValue> m_paramChanged;
    async::Notification m_paramsChangedNotify;

    Steinberg::FUnknownPtr<VstAdvancedHandler> m_advancedHandler = nullptr;
};
}

#endif // MU_VST_VSTCOMPONENTHANDLER_H
