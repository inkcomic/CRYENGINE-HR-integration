#pragma once

#include <CrySystem/VR/IHMDDevice.h>
#include <CrySystem/VR/IHMDManager.h>

#include "../Interface/IHmdHyperealDevice.h"
#include "HyperealController.h"

#include <Cry3DEngine/IIndexedMesh.h>
#include <CryRenderer/IStereoRenderer.h>

struct IConsoleCmdArgs;
struct IRenderer;

namespace CryVR
{
namespace Hypereal
{
class Controller;
class Device : public IHyperealDevice, public IHmdEventListener, public ISystemEventListener
{
public:
	// IHmdDevice
	virtual void                    AddRef() override;
	virtual void                    Release() override;

	virtual EHmdClass               GetClass() const override                         { return eHmdClass_Hypereal; }
	virtual void                    GetDeviceInfo(HmdDeviceInfo& info) const override { info = m_devInfo; }

	virtual void                    GetCameraSetupInfo(float& fov, float& aspectRatioFactor) const override;
	virtual void                    GetAsymmetricCameraSetupInfo(int nEye, float& fov, float& aspectRatio, float& asymH, float& asymV, float& eyeDist) const;
	virtual void                    UpdateInternal(EInternalUpdate type) override;
	virtual void                    RecenterPose() override;
	virtual void                    UpdateTrackingState(EVRComponent type) override;
	virtual const HmdTrackingState& GetNativeTrackingState() const override;
	virtual const HmdTrackingState& GetLocalTrackingState() const override;
	virtual Quad                    GetPlayArea() const override;
	virtual Vec2                    GetPlayAreaSize() const override;
	virtual const IHmdController*   GetController() const override      { return &m_controller; }
	virtual const EHmdSocialScreen  GetSocialScreenType(bool* pKeepAspect = nullptr) const override;
	virtual int                     GetControllerCount() const override { __debugbreak(); return 2; /* Hypereal_TODO */ }
	virtual void                    GetPreferredRenderResolution(unsigned int& width, unsigned int& height) override;
	virtual void                    DisableHMDTracking(bool disable) override;
	// ~IHmdDevice

	// IHyperealDevice
	virtual void SubmitOverlay(int id);
	virtual void SubmitFrame();
	virtual void OnSetupEyeTargets(ERenderAPI api, ERenderColorSpace colorSpace, void* leftEyeHandle, void* rightEyeHandle);
	virtual void OnSetupOverlay(int id, ERenderAPI api, ERenderColorSpace colorSpace, void* overlayTextureHandle);
	virtual void OnDeleteOverlay(int id);
	virtual void GetRenderTargetSize(uint& w, uint& h);
	virtual void GetMirrorImageView(EEyeType eye, void* resource, void** mirrorTextureView) override;
	virtual void CopyMirrorImage(void* pDstResource, uint nWidth, uint nHeight)override;
	// ~IHyperealDevice

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	// IHmdEventListener
	virtual void OnRecentered() override;
	// ~IHmdEventListener

public:
	int GetRefCount() const { return m_refCount; }

public:
	static Device* CreateInstance();
	void           SetupRenderModels();
	void           CaptureInputFocus(bool capture);
	bool           HasInputFocus() { return m_hasInputFocus; }

private:
	Device();
	virtual ~Device();

	void                   CreateDevice();
	void                   PrintHmdInfo();
	void                   DebugDraw(float& xPosLabel, float& yPosLabel) const;

// 	string                 GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* peError = NULL);
 	const char*            GetTrackedDeviceCharPointer(int nProperty);
// 	static inline Matrix34 BuildMatrix(const vr::HmdMatrix34_t& in);
// 	static inline Matrix44 BuildMatrix(const vr::HmdMatrix44_t& in);
 	static inline Quat     HmdQuatToWorldQuat(const Quat& quat);
 	static inline Vec3     HmdVec3ToWorldVec3(const Vec3& vec);
 	inline void            CopyPoseState(HmdPoseState& world, HmdPoseState& hmd, HyTrackingState& source);
// 	void                   LoadDeviceRenderModel(int deviceIndex);
// 	void                   DumpDeviceRenderModel(int deviceIndex);

private:
	enum ERenderModelState
	{
		eRMS_Loading,
		eRMS_Loaded,
		eRMS_Failed
	};
	struct RenderModel
	{
		/*PodArray<Vec3> vertices;
		   PodArray<Vec3> normals;
		   PodArray<Vec2> uvs;
		   PodArray<vtx_idx> indices;*/

		RenderModel(/*vr::IVRRenderModels* renderModels, string name*/);
		~RenderModel();
		void Update();
		bool IsValid()   { return m_modelState != eRMS_Failed && m_textureState != eRMS_Failed; }
		bool IsLoading() { return m_modelState == eRMS_Loading || m_textureState == eRMS_Loading; }

	private:
		string                        m_name;
// 		vr::IVRRenderModels*          m_renderModels;
// 		vr::RenderModel_t*            m_model;
		ERenderModelState             m_modelState;
/*		vr::RenderModel_TextureMap_t* m_texture;*/
		ERenderModelState             m_textureState;
	};
	struct SOverlay
	{
		bool                  visible;
		bool                  submitted;
		HyViewLayer			* layerHandle;
		void				* overlayTexture;
		HyTextureDesc		textureDesc;
// 		vr::VROverlayHandle_t handle;
// 		
// 		vr::HmdMatrix34_t     pos;
	};
	// OpenVR Pointers
// 	vr::IVRSystem*          m_system;
// 	vr::IVRCompositor*      m_compositor;
// 	vr::IVRRenderModels*    m_renderModels;
// 	vr::IVROverlay*         m_overlay;
// 	vr::Texture_t*          m_eyeTargets[EEyeType::eEyeType_NumEyes];

	typedef std::map<uint, SOverlay> MapOverlayer;
	MapOverlayer			m_mapOverlayers;
	SOverlay                m_overlays[RenderLayer::eQuadLayers_Total];
	// General device fields:
	bool                    m_bLoadingScreenActive;
	volatile int            m_refCount;
	int                     m_lastFrameID_UpdateTrackingState; // we could remove this. at some point we may want to sample more than once the tracking state per frame.
	HmdDeviceInfo           m_devInfo;
	EHmdSocialScreen        m_defaultSocialScreenBehavior;
	// Tracking related:
	enum EDevice
	{
		Hmd,
		Left_Controller,
		Right_Controller,
		Total_Count
	};
	HyTrackingState			m_rTrackedDevicePose[EDevice::Total_Count];
	HmdTrackingState        m_nativeStates[EDevice::Total_Count];
	HmdTrackingState        m_localStates[EDevice::Total_Count];
	HmdTrackingState        m_nativeEyePoseStates;
	HmdTrackingState        m_localEyePoseStates;

	HmdTrackingState        m_disabledTrackingState;

	Quat					m_qBaseOrientation;
	Vec3					m_vBaseOffset;
	float					m_fMeterToWorldScale;
	bool					m_bPosTrackingEnable;
	bool					m_bResetOrientationKeepPitchAndRoll;

	// Controller related:
	Controller              m_controller;
	RenderModel*            m_deviceModels[1/*vr::k_unMaxTrackedDeviceCount*/];
	bool                    m_hasInputFocus;
	bool                    m_hmdTrackingDisabled;
	float                   m_hmdQuadDistance;
	float                   m_hmdQuadWidth;
	int                     m_hmdQuadAbsolute;

	ICVar*                  m_pHmdInfoCVar;
	ICVar*                  m_pHmdSocialScreenKeepAspectCVar;
	ICVar*                  m_pHmdSocialScreenCVar;
	ICVar*                  m_pTrackingOriginCVar;



	//////////////////////////////////////////////////////////////////////////
	//HVR device member
	struct DeviceInfo
	{
		int64 DeviceResolutionX;
		int64 DeviceResolutionY;
		HyFov Fov[HY_EYE_MAX];
	};



	HyDevice *VrDevice;
	DeviceInfo VrDeviceInfo;
	HyGraphicsContext *VrGraphicsCxt;
	HyGraphicsContextDesc VrGraphicsCxtDesc;
	HyVec2 *PlayAreaVertices;
	int64 PlayAreaVertexCount;
	bool bPlayAreaValid;
	HyFov eyeFovSym;
	float PixelDensity;
	bool bVRInitialized;
	bool bVRSystemValid;
	bool bIsQuitting;
	HyTextureDesc RTDesc[2];
	float InterpupillaryDistance;

	HyPose	m_CurDevicePose[EDevice::Total_Count];
	bool	m_IsDevicePositionTracked[EDevice::Total_Count];
	bool	m_IsDeviceRotationTracked[EDevice::Total_Count];
	//////////////////////////////////////////////////////////////////////////

	//member func
	void RebuildPlayArea();
	float GetDistance(const HyVec2& P, const HyVec2& PA, const HyVec2& PB);
	void ReleaseDevice();
	inline float GetInterpupillaryDistance() const;
	void ResetOrientationAndPosition(float Yaw);
	void ResetOrientation(float Yaw);
	void ResetPosition();

public:
	virtual void CreateGraphicsContext(void* graphicsDevice);
	virtual void ReleaseGraphicsContext();
};
} // namespace Hypereal
} // namespace CryVR
