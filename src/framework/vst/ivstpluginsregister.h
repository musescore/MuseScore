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

    virtual void registerInstrPlugin(const muse::audio::TrackId trackId, VstPluginPtr pluginPtr) = 0;
    virtual void registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                  const muse::audio::AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr) = 0;
    virtual void registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::AudioFxChainOrder chainOrder,
                                        VstPluginPtr pluginPtr) = 0;

    virtual VstPluginPtr instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const = 0;
    virtual VstPluginPtr fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                  const muse::audio::AudioFxChainOrder chainOrder) const = 0;
    virtual VstPluginPtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                        const muse::audio::AudioFxChainOrder chainOrder) const = 0;

    virtual void unregisterInstrPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) = 0;
    virtual void unregisterFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                    const muse::audio::AudioFxChainOrder chainOrder) = 0;
    virtual void unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                          const muse::audio::AudioFxChainOrder chainOrder) = 0;

    virtual void unregisterAllInstrPlugin() = 0;
    virtual void unregisterAllFx() = 0;
};
}

#endif // MU_VST_IVSTPLUGINSREGISTER_H
