#ifndef MU_VST_IVSTPLUGINSREGISTER_H
#define MU_VST_IVSTPLUGINSREGISTER_H

#include "modularity/imoduleinterface.h"
#include "audio/audiotypes.h"

#include "vsttypes.h"

namespace mu::vst {
class IVstPluginsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVstPluginsRegister)
public:
    virtual ~IVstPluginsRegister() = default;

    virtual void registerInstrPlugin(const audio::TrackId trackId, VstPluginPtr pluginPtr) = 0;
    virtual void registerFxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
                                  const audio::AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr) = 0;
    virtual void registerMasterFxPlugin(const audio::AudioResourceId& resourceId, const audio::AudioFxChainOrder chainOrder,
                                        VstPluginPtr pluginPtr) = 0;

    virtual VstPluginPtr instrumentPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId) const = 0;
    virtual VstPluginPtr fxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
                                  const audio::AudioFxChainOrder chainOrder) const = 0;
    virtual VstPluginPtr masterFxPlugin(const audio::AudioResourceId& resourceId, const audio::AudioFxChainOrder chainOrder) const = 0;

    virtual void unregisterInstrPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId) = 0;
    virtual void unregisterFxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
                                    const audio::AudioFxChainOrder chainOrder) = 0;
    virtual void unregisterMasterFxPlugin(const audio::AudioResourceId& resourceId, const audio::AudioFxChainOrder chainOrder) = 0;

    virtual void unregisterAllInstrPlugin() = 0;
    virtual void unregisterAllFx() = 0;
};
}

#endif // MU_VST_IVSTPLUGINSREGISTER_H
