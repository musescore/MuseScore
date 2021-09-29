#include "vstcomponenthandler.h"

using namespace mu::vst;
using namespace mu::async;

IMPLEMENT_FUNKNOWN_METHODS(VstComponentHandler, IComponentHandler, IComponentHandler::iid)

Channel<PluginParamId, PluginParamValue> VstComponentHandler::pluginParamChanged() const
{
    return m_paramChanged;
}

Notification VstComponentHandler::pluginParamsChanged() const
{
    return m_paramsChangedNotify;
}

Steinberg::tresult VstComponentHandler::beginEdit(Steinberg::Vst::ParamID /*id*/)
{
    return Steinberg::kResultOk;
}

Steinberg::tresult VstComponentHandler::performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized)
{
    m_paramChanged.send(std::move(id), std::move(valueNormalized));

    return Steinberg::kResultOk;
}

Steinberg::tresult VstComponentHandler::endEdit(Steinberg::Vst::ParamID /*id*/)
{
    m_paramsChangedNotify.notify();

    return Steinberg::kResultOk;
}

Steinberg::tresult VstComponentHandler::restartComponent(Steinberg::int32 /*flags*/)
{
    return Steinberg::kResultOk;
}
