#ifndef MU_VST_VSTCOMPONENTHANDLER_H
#define MU_VST_VSTCOMPONENTHANDLER_H

#include "async/notification.h"
#include "async/channel.h"

#include "vsttypes.h"

namespace mu::vst {
class VstComponentHandler : public IComponentHandler
{
    DECLARE_FUNKNOWN_METHODS
public:
    VstComponentHandler() = default;
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
};
}

#endif // MU_VST_VSTCOMPONENTHANDLER_H
