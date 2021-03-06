#pragma once

#include "../Interface/IHyperealPlugin.h"

namespace CryVR
{
namespace Hypereal {
class CPlugin_Hypereal : public IHyperealPlugin, public ISystemEventListener
{
	CRYINTERFACE_BEGIN()
	CRYINTERFACE_ADD(IHyperealPlugin)
	CRYINTERFACE_ADD(ICryPlugin)
	CRYINTERFACE_END()

	CRYGENERATE_SINGLETONCLASS(CPlugin_Hypereal, "Plugin_Hypereal", 0x2F80BAC2A4B94114, 0x94999C8D93D76B39)

	virtual ~CPlugin_Hypereal();

	//! Retrieve name of plugin.
	virtual const char* GetName() const override { return "CryHypereal"; }

	//! Retrieve category for the plugin.
	virtual const char* GetCategory() const override { return "Plugin"; }

	//! This is called to initialize the new plugin.
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;

public:
	virtual IHyperealDevice* CreateDevice() override;
	virtual IHyperealDevice* GetDevice() const override;

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// Start CVars
public:
	static float s_hmd_quad_distance;
	static float s_hmd_quad_width;
	static int s_hmd_quad_absolute;

protected:
	virtual void OnPluginUpdate(EPluginUpdateType updateType) override {}
};

}      // namespace Hypereal
}      // namespace CryVR