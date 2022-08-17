#include "vstcomponenthandler.h"

using namespace mu::vst;
using namespace mu::async;

IMPLEMENT_FUNKNOWN_METHODS(VstAdvancedHandler, Steinberg::Vst::IComponentHandler2, Steinberg::Vst::IComponentHandler2::iid)

IMPLEMENT_REFCOUNT(VstComponentHandler)

::Steinberg::tresult PLUGIN_API VstComponentHandler::queryInterface(const ::Steinberg::TUID _iid, void** obj)
{
    QUERY_INTERFACE(_iid, obj, ::Steinberg::FUnknown::iid, IComponentHandler)
    QUERY_INTERFACE(_iid, obj, IComponentHandler::iid, IComponentHandler)

    if (::Steinberg::FUnknownPrivate::iidEqual(_iid, IAdvancedComponentHandler::iid)) {
        return m_advancedHandler->queryInterface(_iid, obj);
    }
    *obj = nullptr;
    return ::Steinberg::kNoInterface;
}

VstComponentHandler::VstComponentHandler()
    : m_advancedHandler(new VstAdvancedHandler(m_paramsChangedNotify))
{
}

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

VstAdvancedHandler::VstAdvancedHandler(async::Notification notifier)
    : m_paramsChanged(notifier)
{
}

Steinberg::tresult VstAdvancedHandler::setDirty(Steinberg::TBool /*state*/)
{
    m_paramsChanged.notify();

    return Steinberg::kResultOk;
}

Steinberg::tresult VstAdvancedHandler::requestOpenEditor(Steinberg::FIDString /*name*/)
{
    return Steinberg::kResultOk;
}

Steinberg::tresult VstAdvancedHandler::startGroupEdit()
{
    return Steinberg::kResultOk;
}

Steinberg::tresult VstAdvancedHandler::finishGroupEdit()
{
    m_paramsChanged.notify();

    return Steinberg::kResultOk;
}
