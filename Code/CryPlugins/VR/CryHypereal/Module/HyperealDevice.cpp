#include "StdAfx.h"

#include "HyperealDevice.h"
#include "HyperealResources.h"
#include "HyperealController.h"

#include "PluginDll.h"

#include <CrySystem/VR/IHMDManager.h>

#include <CryGame/IGameFramework.h>
#include <CryRenderer/IStereoRenderer.h>
#include <Cry3DEngine/IIndexedMesh.h>
#include <CryRenderer/IRenderAuxGeom.h>
#include <CryMath/Cry_Color.h>

#include "..\CryAction\IViewSystem.h"
#include <CryCore/Platform/CryWindows.h>

#define HY_RELEASE(p) {if(p != nullptr) p->Release(); p = nullptr;}
#define DEFAULT_IPD 0.064f;

HyFov ComputeSymmetricalFov(const HyFov& fovLeftEye, const HyFov& fovRightEye)
{
	const float stereoDeviceAR = 1.7777f;

	HyFov fovMax;
	fovMax.m_upTan = max(fovLeftEye.m_upTan, fovRightEye.m_upTan);
	fovMax.m_downTan = max(fovLeftEye.m_downTan, fovRightEye.m_downTan);
	fovMax.m_leftTan = max(fovLeftEye.m_leftTan, fovRightEye.m_leftTan);
	fovMax.m_rightTan = max(fovLeftEye.m_rightTan, fovRightEye.m_rightTan);

	const float combinedTanHalfFovHorizontal = max(fovMax.m_leftTan, fovMax.m_rightTan);
	const float combinedTanHalfFovVertical = max(fovMax.m_upTan, fovMax.m_downTan);

	HyFov fovSym;
	fovSym.m_upTan = fovSym.m_downTan = combinedTanHalfFovVertical * 1.f;
	
	fovSym.m_leftTan = fovSym.m_rightTan = fovSym.m_upTan / (2.f / stereoDeviceAR);

	CryLog("[Hypereal] Fov: Up/Down tans [%f] Left/Right tans [%f]", fovSym.m_upTan, fovSym.m_leftTan);
	return fovSym;
}


// 
// // -------------------------------------------------------------------------
// namespace vr
// {
// // -------------------------------------------------------------------------
// static void SetIdentity(HmdMatrix34_t& in)
// {
// 	memset(in.m, 0, sizeof(HmdMatrix34_t));
// 	in.m[0][0] = 1;
// 	in.m[1][1] = 1;
// 	in.m[2][2] = 1;
// }
// 
// // -------------------------------------------------------------------------
// static void SetZero(HmdVector3_t& in)
// {
// 	memset(in.v, 0, sizeof(HmdVector3_t));
// }
// 
// static Matrix34 RawConvert(HmdMatrix34_t in)
// {
// 	return Matrix34(in.m[0][0], in.m[0][1], in.m[0][2], in.m[0][3],
// 	                in.m[1][0], in.m[1][1], in.m[1][2], in.m[1][3],
// 	                in.m[2][0], in.m[2][1], in.m[2][2], in.m[2][3]);
// }
// 
// static Matrix44 RawConvert(HmdMatrix44_t in)
// {
// 	return Matrix44(in.m[0][0], in.m[0][1], in.m[0][2], in.m[0][3],
// 	                in.m[1][0], in.m[1][1], in.m[1][2], in.m[1][3],
// 	                in.m[2][0], in.m[2][1], in.m[2][2], in.m[2][3],
// 	                in.m[3][0], in.m[3][1], in.m[3][2], in.m[3][3]);
// }
// 
// static HmdMatrix34_t RawConvert(Matrix34 in)
// {
// 	HmdMatrix34_t out;
// 	out.m[0][0] = in.m00;
// 	out.m[0][1] = in.m01;
// 	out.m[0][2] = in.m02;
// 	out.m[0][3] = in.m03;
// 	out.m[1][0] = in.m10;
// 	out.m[1][1] = in.m11;
// 	out.m[1][2] = in.m12;
// 	out.m[1][3] = in.m13;
// 	out.m[2][0] = in.m20;
// 	out.m[2][1] = in.m21;
// 	out.m[2][2] = in.m22;
// 	out.m[2][3] = in.m23;
// 	return out;
// }
// 
// static HmdMatrix44_t RawConvert(Matrix44 in)
// {
// 	HmdMatrix44_t out;
// 	out.m[0][0] = in.m00;
// 	out.m[0][1] = in.m01;
// 	out.m[0][2] = in.m02;
// 	out.m[0][3] = in.m03;
// 	out.m[1][0] = in.m10;
// 	out.m[1][1] = in.m11;
// 	out.m[1][2] = in.m12;
// 	out.m[1][3] = in.m13;
// 	out.m[2][0] = in.m20;
// 	out.m[2][1] = in.m21;
// 	out.m[2][2] = in.m22;
// 	out.m[2][3] = in.m23;
// 	out.m[3][0] = in.m30;
// 	out.m[3][1] = in.m31;
// 	out.m[3][2] = in.m32;
// 	out.m[3][3] = in.m33;
// 	return out;
// }
// }

namespace CryVR {
namespace Hypereal {
// -------------------------------------------------------------------------
Device::RenderModel::RenderModel(/*vr::IVRRenderModels* renderModels, string name*/)
// 	: m_renderModels(renderModels)
// 	, m_name(name)
// 	, m_model(nullptr)
// 	, m_texture(nullptr)
// 	, m_modelState(eRMS_Loading)
// 	, m_textureState(eRMS_Loading)
{

}

Device::RenderModel::~RenderModel()
{
	// Note: we currently do not use the render models, therefore we also do not set them up for rendering!
	/*vertices.clear();
	   normals.clear();
	   uvs.clear();
	   indices.clear();*/

// 	m_model = nullptr; // Object is managed by Hypereal runtime. Since it is possible, that the rendermodel is being accessed by something else, we DO NOT want to delete it
// 	m_modelState = eRMS_Failed;
// 	m_texture = nullptr; // Object is managed by Hypereal runtime. Since it is possible, that the texture is being accessed by something else, we DO NOT want to delete it
// 	m_textureState = eRMS_Failed;
// 
// 	m_renderModels = nullptr;
}

void Device::RenderModel::Update()
{
// 	if (m_modelState == eRMS_Loading) // check model loading progress
// 	{
// 		vr::EVRRenderModelError result = m_renderModels->LoadRenderModel_Async(m_name.c_str(), &m_model);
// 		if (result == vr::VRRenderModelError_Loading)
// 		{
// 			// still loading
// 		}
// 		else if (result == vr::VRRenderModelError_None)
// 		{
// 			m_modelState = eRMS_Loaded;
// 
// 			// Note: we currently do not use the render models, therefore we also do not set them up for rendering!
// 			/*vertices.reserve(m_model->unVertexCount);
// 			   normals.reserve(m_model->unVertexCount);
// 			   uvs.reserve(m_model->unVertexCount);
// 			   indices.reserve(m_model->unVertexCount);
// 
// 			   for (int i = 0; i < m_model->unVertexCount; i++)
// 			   {
// 			   const vr::RenderModel_Vertex_t vrVert = m_model->rVertexData[i];
// 			   vertices.Add(Vec3(vrVert.vPosition.v[0], -vrVert.vPosition.v[2], vrVert.vPosition.v[1]));
// 			   normals.Add(Vec3(vrVert.vNormal.v[0], -vrVert.vNormal.v[2], vrVert.vNormal.v[1]));
// 			   uvs.Add(Vec2(vrVert.rfTextureCoord[0], vrVert.rfTextureCoord[1]));
// 			   indices.Add(m_model->rIndexData[i]);
// 			   }*/
// 		}
// 		else
// 		{
// 			m_modelState = eRMS_Loaded;
// 		}
// 	}
// 
// 	if (m_modelState == eRMS_Loaded && m_textureState == eRMS_Loading) // check texture loading progress
// 	{
// 		vr::EVRRenderModelError result = m_renderModels->LoadTexture_Async(m_model->diffuseTextureId, &m_texture);
// 		if (result == vr::VRRenderModelError_Loading)
// 		{
// 			// still loading
// 		}
// 		else if (result == vr::VRRenderModelError_None)
// 		{
// 			m_textureState = eRMS_Loaded;
// 
// 			// TODO: Setup engine texture
// 
// 			gEnv->pLog->Log("[HMD][Hypereal] Device render model loaded: %s", m_name.c_str());
// 		}
// 		else
// 		{
// 			m_textureState = eRMS_Failed;
// 		}
// 	}
}

static ICVar* pParallax = nullptr;
// -------------------------------------------------------------------------
Device::Device()
	: m_refCount(1)     //OpenVRResources.cpp assumes refcount is 1 on allocation
	, m_controller()
	, m_lastFrameID_UpdateTrackingState(-1)
	, m_devInfo()
	, m_hasInputFocus(false)
	, m_bLoadingScreenActive(false)
	, m_hmdTrackingDisabled(false)
	, m_hmdQuadDistance(CPlugin_Hypereal::s_hmd_quad_distance)
	, m_hmdQuadWidth(CPlugin_Hypereal::s_hmd_quad_width)
	, m_hmdQuadAbsolute(CPlugin_Hypereal::s_hmd_quad_absolute)
	, VrDevice(nullptr)
	, VrGraphicsCxt(nullptr)
	, PlayAreaVertices(nullptr)
	, bPlayAreaValid(false)
	, PixelDensity(1.0f)
	, bVRInitialized(nullptr)
	, bVRSystemValid(nullptr)
	, bIsQuitting(false)
	, InterpupillaryDistance(-1.0f)
	, m_qBaseOrientation(IDENTITY)
	, m_vBaseOffset(IDENTITY)
	, m_fMeterToWorldScale(1.f)
	, m_bPosTrackingEnable(true)
	, m_bResetOrientationKeepPitchAndRoll(false)
{
	m_pHmdInfoCVar = gEnv->pConsole->GetCVar("hmd_info");
	m_pHmdSocialScreenKeepAspectCVar = gEnv->pConsole->GetCVar("hmd_social_screen_keep_aspect");
	m_pHmdSocialScreenCVar = gEnv->pConsole->GetCVar("hmd_social_screen");
	m_pTrackingOriginCVar = gEnv->pConsole->GetCVar("hmd_tracking_origin");

	CreateDevice();

	if (VrDevice == nullptr)
		return;

	gEnv->pSystem->GetHmdManager()->AddEventListener(this);

	pParallax = gEnv->pConsole->GetCVar("sys_flash_stereo_maxparallax");

	// 	memset(m_rTrackedDevicePose, 0, sizeof(vr::TrackedDevicePose_t) * vr::k_unMaxTrackedDeviceCount);
	// 	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
	// 	{
	// 		m_deviceModels[i] = nullptr;
	// 		vr::SetIdentity(m_rTrackedDevicePose[i].mDeviceToAbsoluteTracking);
	// 	}
	// 	for (int i = 0; i < EEyeType::eEyeType_NumEyes; i++)
	// 		m_eyeTargets[i] = nullptr;
	// 
	if (GetISystem()->GetISystemEventDispatcher())
		GetISystem()->GetISystemEventDispatcher()->RegisterListener(this);

	m_controller.Init(VrDevice);

	if(m_controller.IsConnected(EHmdController::eHmdController_Hypereal_1))
		m_controller.OnControllerConnect(HY_SUBDEV_CONTROLLER_LEFT);

	if(m_controller.IsConnected(EHmdController::eHmdController_Hypereal_2))
		m_controller.OnControllerConnect(HY_SUBDEV_CONTROLLER_RIGHT);
	
		
	for (int id = 0; id < RenderLayer::eQuadLayers_Total; id++)
	{
		memset(&m_overlays[id], 0, sizeof(m_overlays[id]));
	}
	
}

// -------------------------------------------------------------------------
Device::~Device()
{
	gEnv->pSystem->GetHmdManager()->RemoveEventListener(this);

	if (GetISystem()->GetISystemEventDispatcher())
		GetISystem()->GetISystemEventDispatcher()->RemoveListener(this);
}
// 
// // -------------------------------------------------------------------------
// string Device::GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* peError)
// {
// 	uint32_t unRequiredBufferLen = m_system->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
// 	if (unRequiredBufferLen == 0)
// 		return "";
// 
// 	char* pchBuffer = new char[unRequiredBufferLen];
// 	unRequiredBufferLen = m_system->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
// 	string result = string(pchBuffer, unRequiredBufferLen);
// 	delete[] pchBuffer;
// 	return result;
// }
// 
// -------------------------------------------------------------------------
const char* Device::GetTrackedDeviceCharPointer(int nProperty)
{
	int realStrLen = 512;
	//VrDevice->GetStringValue(HY_PROPERTY_DEVICE_MANUFACTURER_STRING, 0, 0, &realStrLen);

	//if (realStrLen == 0)
	//	return nullptr;

	char* pBuffer = new char[realStrLen];
	VrDevice->GetStringValue(HY_PROPERTY_DEVICE_MANUFACTURER_STRING, pBuffer, realStrLen, &realStrLen);
	return const_cast<char*>(pBuffer);
}
// 
// // -------------------------------------------------------------------------
// Matrix34 Device::BuildMatrix(const vr::HmdMatrix34_t& in)
// {
// 	return Matrix34(in.m[0][0], in.m[0][1], in.m[0][2], in.m[0][3],
// 	                in.m[1][0], in.m[1][1], in.m[1][2], in.m[1][3],
// 	                in.m[2][0], in.m[2][1], in.m[2][2], in.m[2][3]);
// }
// 
// // -------------------------------------------------------------------------
// Matrix44 Device::BuildMatrix(const vr::HmdMatrix44_t& in)
// {
// 	return Matrix44(in.m[0][0], in.m[0][1], in.m[0][2], in.m[0][3],
// 	                in.m[1][0], in.m[1][1], in.m[1][2], in.m[1][3],
// 	                in.m[2][0], in.m[2][1], in.m[2][2], in.m[2][3],
// 	                in.m[3][0], in.m[3][1], in.m[3][2], in.m[3][3]);
// }
// 
// -------------------------------------------------------------------------
inline Quat HmdQuatToWorldQuat(const Quat& quat)
{
	Matrix33 m33(quat);
	Vec3 column1 = -quat.GetColumn2();
	m33.SetColumn2(m33.GetColumn1());
	m33.SetColumn1(column1);
	return Quat::CreateRotationX(gf_PI * 0.5f) * Quat(m33);
}
 
//  // -------------------------------------------------------------------------
//  Vec3 Device::HmdVec3ToWorldVec3(const Vec3& vec)
//  {
//  	return Vec3(vec.x, -vec.z, vec.y);
//  }

 inline Quat HYQuatToQuat(const HyQuat& q) {
	  return HmdQuatToWorldQuat(Quat(q.w, q.x, q.y, q.z));
 }

 inline Vec3 HYVec3ToVec3(const HyVec3& v) {
	 return Vec3(v.x,-v.z,v.y);
 }
 inline HyQuat QuatToHYQuat(const Quat &q) {
	 Quat invQuat = HmdQuatToWorldQuat(q).GetInverted();
	 HyQuat hyQuat;
	 hyQuat.w = invQuat.w;
	 hyQuat.x = invQuat.v.x;
	 hyQuat.y = invQuat.v.y;
	 hyQuat.z = invQuat.v.z;
	 return hyQuat;
 }
 inline HyVec3 Vec3ToHYVec3(const Vec3& v) {
	 HyVec3 hyVec3;
	 hyVec3.x = v.x;
	 hyVec3.y = v.z;
	 hyVec3.z = -v.y;
	 return hyVec3;
 }
// -------------------------------------------------------------------------
void Device::CopyPoseState(HmdPoseState& world, HmdPoseState& hmd, HyTrackingState& src)
{
	Quat srcQuat = HYQuatToQuat(src.m_pose.m_rotation);
	Vec3 srcPos = HYVec3ToVec3(src.m_pose.m_position);
	Vec3 srcAngVel = HYVec3ToVec3(src.m_angularVelocity);
	Vec3 srcAngAcc = HYVec3ToVec3(src.m_angularAcceleration);
	Vec3 srcLinVel = HYVec3ToVec3(src.m_linearVelocity);
	Vec3 srcLinAcc = HYVec3ToVec3(src.m_linearAcceleration);

	hmd.orientation = srcQuat;
	hmd.position = srcPos;

	hmd.linearVelocity = srcLinVel;
	hmd.angularVelocity = srcAngVel;
 
	world.position = /*HmdVec3ToWorldVec3*/(hmd.position);
	world.orientation = /*HmdQuatToWorldQuat*/(hmd.orientation);
	world.linearVelocity = /*HmdVec3ToWorldVec3*/(hmd.linearVelocity);
	world.angularVelocity = /*HmdVec3ToWorldVec3*/(hmd.angularVelocity);
}

// -------------------------------------------------------------------------
void Device::AddRef()
{
	CryInterlockedIncrement(&m_refCount);
}

// -------------------------------------------------------------------------
void Device::Release()
{
	long refCount = CryInterlockedDecrement(&m_refCount);
#if !defined(_RELEASE)
	IF (refCount < 0, 0)
		__debugbreak();
#endif
	IF (refCount == 0, 0)
	{
		delete this;
	}
}

// -------------------------------------------------------------------------
void Device::GetPreferredRenderResolution(unsigned int& width, unsigned int& height)
{
	GetRenderTargetSize(width, height);
}

// -------------------------------------------------------------------------
void Device::DisableHMDTracking(bool disable)
{
	m_hmdTrackingDisabled = disable;
}

// -------------------------------------------------------------------------
Device* Device::CreateInstance()
{
	return new Device();
}

// -------------------------------------------------------------------------
void Device::GetCameraSetupInfo(float& fov, float& aspectRatioFactor) const
{
// 	float fNear = gEnv->pRenderer->GetCamera().GetNearPlane();
// 	float fFar = gEnv->pRenderer->GetCamera().GetFarPlane();
// 
// 	HyMat4 proj;
// 	VrGraphicsCxt->GetProjectionMatrix(VrDeviceInfo.Fov[HY_EYE_LEFT], fNear, fFar, true, proj);
// 	fov = 2.0f * atan(1.0f / proj.m[1][1]);
// 	aspectRatioFactor = 2.0f;
	fov = m_devInfo.fovV;
	aspectRatioFactor = 2.0f * eyeFovSym.m_leftTan;
// 	vr::HmdMatrix44_t proj = m_system->GetProjectionMatrix(vr::EVREye::Eye_Left, fNear, fFar, vr::API_DirectX);
// 
// 	fov = 2.0f * atan(1.0f / proj.m[1][1]);
// 	aspectRatioFactor = 2.0f;
}

// -------------------------------------------------------------------------
void Device::GetAsymmetricCameraSetupInfo(int nEye, float& fov, float& aspectRatio, float& asymH, float& asymV, float& eyeDist) const
{
	if (VrGraphicsCxt == nullptr)
		return;

	float fNear = gEnv->pRenderer->GetCamera().GetNearPlane();
	float fFar = gEnv->pRenderer->GetCamera().GetFarPlane();
	HyMat4 proj;
	VrGraphicsCxt->GetProjectionMatrix(VrDeviceInfo.Fov[nEye], fNear, fFar, true, proj);
	fov = 2.0f * atan(1.0f / proj.m[1][1]);
	aspectRatio = proj.m[1][1] / proj.m[0][0];
	asymH = proj.m[0][2] / proj.m[1][1] * aspectRatio;
	asymV = proj.m[1][2] / proj.m[1][1];

	eyeDist = GetInterpupillaryDistance();
}

// -------------------------------------------------------------------------
void Device::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_LOAD_START_LOADINGSCREEN:
		m_bLoadingScreenActive = true;
		break;

	// Intentional fall through
	case ESYSTEM_EVENT_LEVEL_LOAD_END:
	case ESYSTEM_EVENT_LEVEL_LOAD_ERROR:
		m_bLoadingScreenActive = false;
		m_controller.ClearState();
		break;

	case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
		m_controller.ClearState();
		break;

	default:
		break;
	}
}
// -------------------------------------------------------------------------
float Device::GetInterpupillaryDistance() const
{
	if (InterpupillaryDistance > 0.01f)
		return InterpupillaryDistance;
	if (bVRSystemValid)
	{
		bool isConnected = false;
		HyResult hr = VrDevice->GetBoolValue(HY_PROPERTY_HMD_CONNECTED_BOOL, isConnected);
		if (hySucceeded(hr) && isConnected)
		{
			float ipd = DEFAULT_IPD;
			hr = VrDevice->GetFloatValue(HY_PROPERTY_IPD_FLOAT, ipd);
			if (hySucceeded(hr))
				return ipd;
		}
	}
	return DEFAULT_IPD;
}

void Device::ResetOrientationAndPosition(float Yaw)
{
	ResetOrientation(Yaw);
	ResetPosition();
}

void Device::ResetOrientation(float yaw)
{
	Quat qBaseOrientation = HYQuatToQuat(m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_rotation);

	Ang3 currentAng = Ang3(qBaseOrientation);
	if (!m_bResetOrientationKeepPitchAndRoll)
	{
		currentAng.x = 0;//Pitch
		currentAng.y = 0;//Roll
	}

	if (fabs(yaw )>FLT_EPSILON)
	{
		currentAng.z -= yaw;
		currentAng.Normalize();
	}
	m_qBaseOrientation =Quat(currentAng);
}

void Device::ResetPosition()
{
	m_vBaseOffset = HYVec3ToVec3(m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_position);
}

// -------------------------------------------------------------------------
void Device::UpdateTrackingState(EVRComponent type)
{
	IRenderer* pRenderer = gEnv->pRenderer;

	const int frameID = pRenderer->GetFrameID(false);

#if !defined(_RELEASE)
	if (!gEnv->IsEditor())// we currently assume one system update per frame rendered, which is not always the case in editor (i.e. no level)
	{
		if (((type & eVRComponent_Hmd) != 0) && (CryGetCurrentThreadId() != gEnv->mMainThreadId) && (m_bLoadingScreenActive == false))
		{
			gEnv->pLog->LogError("[HMD][Hypereal] Device::UpdateTrackingState() should be called from main thread only!");
		}

		if (frameID == m_lastFrameID_UpdateTrackingState)
		{
			gEnv->pLog->LogError("[HMD][Hypereal] Device::UpdateTrackingState() should be called only once per frame!");
		}
	}
	m_lastFrameID_UpdateTrackingState = frameID;
#endif


	if (VrDevice)
	{
		VrDevice->ConfigureTrackingOrigin(m_pTrackingOriginCVar->GetIVal() == (int)EHmdTrackingOrigin::Floor == 1 ? HY_TRACKING_ORIGIN_FLOOR : HY_TRACKING_ORIGIN_EYE);

		static const HySubDevice Devices[EDevice::Total_Count] =
		{ HY_SUBDEV_HMD, HY_SUBDEV_CONTROLLER_LEFT, HY_SUBDEV_CONTROLLER_RIGHT };

		HyTrackingState trackingState;
		for (uint32_t i = 0; i < EDevice::Total_Count; i++)
		{
			HyResult r = VrDevice->GetTrackingState(Devices[i], frameID, trackingState);
			if (hySucceeded(r))
			{
				m_rTrackedDevicePose[i] = trackingState;
				m_IsDevicePositionTracked[i] = ((HY_TRACKING_POSITION_TRACKED & trackingState.m_flags) != 0);
				m_IsDeviceRotationTracked[i] = ((HY_TRACKING_ROTATION_TRACKED & trackingState.m_flags) != 0);


				m_localStates[i].statusFlags = m_nativeStates[i].statusFlags = ((m_IsDeviceRotationTracked[EDevice::Hmd]) ? eHmdStatus_OrientationTracked : 0) |
					((m_IsDevicePositionTracked[EDevice::Hmd]) ? eHmdStatus_PositionTracked : 0);


				if (m_IsDevicePositionTracked[i]||m_IsDeviceRotationTracked[i])
				{
					CopyPoseState(m_localStates[i].pose, m_nativeStates[i].pose, m_rTrackedDevicePose[i]);
				}

			}
			else
			{
				m_IsDevicePositionTracked[i] = false;
				m_IsDeviceRotationTracked[i] = false;
			}

			if(i!= Hmd)
			{
				HyResult res;
				HyInputState controllerState;

				HySubDevice sid = static_cast<HySubDevice>(i);
				res = VrDevice->GetControllerInputState(sid, controllerState);
				if (hySuccess==res)
					m_controller.Update(sid, m_nativeStates[i], m_localStates[i], controllerState);
			}
			else//HMD
			{
				//need retransform
				float* ipdptr = (InterpupillaryDistance > 0.01f ? &InterpupillaryDistance : nullptr);
				HyPose hyEyeRenderPose[HY_EYE_MAX];
				VrGraphicsCxt->GetEyePoses(m_rTrackedDevicePose[EDevice::Hmd].m_pose, ipdptr, hyEyeRenderPose);
				
				memcpy(&m_nativeEyePoseStates, &m_nativeStates[EDevice::Hmd], sizeof(HmdTrackingState));
				memcpy(&m_localEyePoseStates, &m_localStates[EDevice::Hmd], sizeof(HmdTrackingState));

// 				m_baseTrans.q = m_qBaseOrientation;
// 				m_baseTrans.t = m_vBaseOffset;
// 				QuatT qtInverted = m_baseTrans.GetInverted();
// 				QuatT qtNew(HYQuatToQuat(hyEyeRenderPose[HY_EYE_LEFT].m_rotation), HYVec3ToVec3(hyEyeRenderPose[HY_EYE_LEFT].m_position));
// 				QuatT qtRecentered = qtNew*qtInverted;
				// compute centered transformation
				Quat eyeRotation = HYQuatToQuat(hyEyeRenderPose[HY_EYE_LEFT].m_rotation);
				Vec3 eyePosition = HYVec3ToVec3(hyEyeRenderPose[HY_EYE_LEFT].m_position);

				Quat qRecenterRotation = m_qBaseOrientation.GetInverted()*eyeRotation;
				qRecenterRotation.Normalize();
				Vec3 vRecenterPosition = (eyePosition - m_vBaseOffset) * m_fMeterToWorldScale;
				vRecenterPosition = m_qBaseOrientation.GetInverted()*vRecenterPosition;
				if (!m_bPosTrackingEnable)
				{
					vRecenterPosition.x = 0.f;
					vRecenterPosition.y = 0.f;
				}

				m_nativeEyePoseStates.pose.orientation = m_localEyePoseStates.pose.orientation = qRecenterRotation;
				m_nativeEyePoseStates.pose.position = m_localEyePoseStates.pose.position = vRecenterPosition;
			}
		}

	}
}

// -------------------------------------------------------------------------
void Device::UpdateInternal(EInternalUpdate type)
{
	if (!VrDevice || !VrGraphicsCxt)
	{
		return;
	}
	const HyMsgHeader *msg;
	while (true)
	{
		VrDevice->RetrieveMsg(&msg);
		if (msg->m_type == HY_MSG_NONE)
			break;
		switch (msg->m_type)
		{
		case HY_MSG_PENDING_QUIT:
			bIsQuitting = true;
			break;
		case HY_MSG_INPUT_FOCUS_CHANGED:
// 			bIsPausing = !(((HyMsgFocusChange*)msg)->m_id == HY_ID_SELF_IN_MSG);
// 			if (bIsPausing)
// 				FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Broadcast();
// 			else
// 				FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Broadcast();
 			break;
		case HY_MSG_VIEW_FOCUS_CHANGED:
// 			bIsVisible = ((HyMsgFocusChange*)msg)->m_id == HY_ID_SELF_IN_MSG;
// 			if (bIsVisible)
// 				FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Broadcast();
// 			else
// 				FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Broadcast();
			break;
		case HY_MSG_SUBDEVICE_STATUS_CHANGED:
		{
			HyMsgSubdeviceChange* pData = ((HyMsgSubdeviceChange*)msg);
			HySubDevice sid = static_cast<HySubDevice>(pData->m_subdevice);
			if (0 != pData->m_value)
				m_controller.OnControllerConnect(sid);
			else
				m_controller.OnControllerDisconnect(sid);
		}
			break;
		default:
			VrDevice->DefaultMsgFunction(msg);
			break;
		}
	}

	if (bIsQuitting)
	{
		//EnableStereo(false);
#if WITH_EDITOR
		if (GIsEditor)
		{
			FSceneViewport* SceneVP = FindSceneViewport();
			if (SceneVP && SceneVP->IsStereoRenderingAllowed())
			{
				TSharedPtr<SWindow> Window = SceneVP->FindWindow();
				Window->RequestDestroyWindow();
			}
		}
		else
#endif	//WITH_EDITOR
		{
			gEnv->pSystem->Quit();
		}
		bIsQuitting = false;
	}

}

// -------------------------------------------------------------------------
void Device::CreateDevice()
{
	for (int i = 0; i < 2; ++i)
	{
		RTDesc[i].m_uvSize = HyVec2{ 1.f, 1.f };
		RTDesc[i].m_uvOffset = HyVec2{ 0.f, 0.f };
	}

	HyResult startResult = HyStartup();
	bVRInitialized = hySucceeded(startResult);
	if (!bVRInitialized)
	{
		gEnv->pLog->Log("[HMD][Hypereal] HyperealVR Failed to Startup.");
		return;
	}

	gEnv->pLog->Log("[HMD][Hypereal] HyperealVR Startup sucessfully.");
	
	bPlayAreaValid = false;
	PlayAreaVertexCount = 0;

	
	HyResult hr = HyCreateInterface(sch_HyDevice_Version, 0, (void**)&VrDevice);

	if (!hySucceeded(hr))
	{
		gEnv->pLog->Log("[HMD][Hypereal] HyCreateInterface failed.");
		return ;
	}

	

	memset(&VrDeviceInfo,0, sizeof(DeviceInfo));

	VrDevice->GetIntValue(HY_PROPERTY_DEVICE_RESOLUTION_X_INT, VrDeviceInfo.DeviceResolutionX);
	VrDevice->GetIntValue(HY_PROPERTY_DEVICE_RESOLUTION_Y_INT, VrDeviceInfo.DeviceResolutionY);
	VrDevice->GetFloatArray(HY_PROPERTY_DEVICE_LEFT_EYE_FOV_FLOAT4_ARRAY, VrDeviceInfo.Fov[HY_EYE_LEFT].val, 4);
	VrDevice->GetFloatArray(HY_PROPERTY_DEVICE_RIGHT_EYE_FOV_FLOAT4_ARRAY, VrDeviceInfo.Fov[HY_EYE_RIGHT].val, 4);



	eyeFovSym = ComputeSymmetricalFov( VrDeviceInfo.Fov[HY_EYE_LEFT], VrDeviceInfo.Fov[HY_EYE_RIGHT]);
	//set for  generic HMD device
	m_devInfo.screenWidth = (uint)VrDeviceInfo.DeviceResolutionX;
	m_devInfo.screenHeight = (uint)VrDeviceInfo.DeviceResolutionY;
	
	m_devInfo.manufacturer = GetTrackedDeviceCharPointer(HY_PROPERTY_DEVICE_MANUFACTURER_STRING);
	m_devInfo.productName = GetTrackedDeviceCharPointer(HY_PROPERTY_DEVICE_PRODUCT_NAME_STRING);
	m_devInfo.fovH = 2.0f * atanf(eyeFovSym.m_leftTan);
	m_devInfo.fovV = 2.0f * atanf(eyeFovSym.m_upTan);


	bool isConnected = false;
	hr = VrDevice->GetBoolValue(HY_PROPERTY_HMD_CONNECTED_BOOL, isConnected);
	if (hySucceeded(hr) && isConnected)
	{
	//	CustomPresent = new FHyperealCustomPresent(nullptr, this, VrGraphicsCxt);
		
		VrDevice->ConfigureTrackingOrigin(m_pTrackingOriginCVar->GetIVal() == (int)EHmdTrackingOrigin::Floor == 1 ? HY_TRACKING_ORIGIN_FLOOR : HY_TRACKING_ORIGIN_EYE);

		RebuildPlayArea();
		gEnv->pLog->Log("[HMD][Hypereal] EnableStereo successfully.");
	}
	else
	{
		gEnv->pLog->Log("[HMD][Hypereal] HyperealVR HMD is Disconnected.");
	}

	 #if CRY_PLATFORM_WINDOWS
	 	// the following is (hopefully just) a (temporary) hack to shift focus back to the CryEngine window, after (potentially) spawning the SteamVR Compositor
	 	if (!gEnv->IsEditor())
	 	{
	 		LockSetForegroundWindow(LSFW_UNLOCK);
	 		SetForegroundWindow((HWND)gEnv->pSystem->GetHWND());
	 	}
	 #endif
}

void Device::ReleaseDevice() 
{
	HY_RELEASE(VrDevice);

	bVRSystemValid = false;

	if (bVRInitialized)
	{
		bVRInitialized = false;
		HyShutdown();
	}
}
// -------------------------------------------------------------------------
void Device::DebugDraw(float& xPosLabel, float& yPosLabel) const
{
	// Basic info
	const float yPos = yPosLabel, xPosData = xPosLabel, yDelta = 20.f;
	float y = yPos;
	const ColorF fColorLabel(1.0f, 1.0f, 1.0f, 1.0f);
	const ColorF fColorIdInfo(1.0f, 1.0f, 0.0f, 1.0f);
	const ColorF fColorInfo(0.0f, 1.0f, 0.0f, 1.0f);
	const ColorF fColorDataPose(0.0f, 1.0f, 1.0f, 1.0f);

	IRenderAuxText::Draw2dLabel(xPosLabel, y, 1.3f, fColorLabel, false, "Hypereal HMD Info:");
	y += yDelta;

	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorIdInfo, false, "Device:%ss", m_devInfo.productName ? m_devInfo.productName : "unknown");
	y += yDelta;
	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorIdInfo, false, "Manufacturer:%s", m_devInfo.manufacturer ? m_devInfo.manufacturer : "unknown");
	y += yDelta;
	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorInfo, false, "Resolution: %dx%d", m_devInfo.screenWidth, m_devInfo.screenHeight);
	y += yDelta;
	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorInfo, false, "FOV (degrees): H:%.2f - V:%.2f", m_devInfo.fovH * 180.0f / gf_PI, m_devInfo.fovV * 180.0f / gf_PI);
	y += yDelta;

 	const Vec3 hmdPos = m_localStates[EDevice::Hmd].pose.position;
 	const Ang3 hmdRotAng(m_localStates[EDevice::Hmd].pose.orientation);
 	const Vec3 hmdRot(RAD2DEG(hmdRotAng));
 	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorDataPose, false, "HmdPos(LS):[%.f,%f,%f]", hmdPos.x, hmdPos.y, hmdPos.z);
 	y += yDelta;
 	IRenderAuxText::Draw2dLabel(xPosData, y, 1.3f, fColorDataPose, false, "HmdRot(LS):[%.f,%f,%f] (PRY) degrees", hmdRot.x, hmdRot.y, hmdRot.z);
 	y += yDelta;
 
 	yPosLabel = y;
}

// -------------------------------------------------------------------------
void Device::OnRecentered()
{
	RecenterPose();
}

// -------------------------------------------------------------------------
void Device::RecenterPose()
{
	ResetOrientationAndPosition(0.0f);
}

// -------------------------------------------------------------------------
const HmdTrackingState& Device::GetLocalTrackingState() const
{
	return m_hmdTrackingDisabled ? m_disabledTrackingState : m_localEyePoseStates;// m_localStates[EDevice::Hmd];
}

// -------------------------------------------------------------------------
Quad Device::GetPlayArea() const
{
	Quad result;
	result.vCorners[0] = /*HmdVec3ToWorldVec3*/(Vec3(PlayAreaVertices[0].x, PlayAreaVertices[0].y, 0));
	result.vCorners[1] = /*HmdVec3ToWorldVec3*/(Vec3(PlayAreaVertices[1].x, PlayAreaVertices[1].y, 0));
	result.vCorners[2] = /*HmdVec3ToWorldVec3*/(Vec3(PlayAreaVertices[2].x, PlayAreaVertices[2].y, 0));
	result.vCorners[3] = /*HmdVec3ToWorldVec3*/(Vec3(PlayAreaVertices[3].x, PlayAreaVertices[3].y, 0));
	return result;

}

// -------------------------------------------------------------------------
Vec2 Device::GetPlayAreaSize() const
{
// 	if (auto* pChaperone = vr::VRChaperone())
// 	{
// 		Vec2 result;
// 		if (pChaperone->GetPlayAreaSize(&result.x, &result.y))
// 		{
// 			return result;
// 		}
// 	}

	return Vec2(ZERO);
}

// -------------------------------------------------------------------------
const HmdTrackingState& Device::GetNativeTrackingState() const
{
	return m_hmdTrackingDisabled ? m_disabledTrackingState : m_nativeEyePoseStates;// m_nativeStates[EDevice::Hmd];
}

// -------------------------------------------------------------------------
const EHmdSocialScreen Device::GetSocialScreenType(bool* pKeepAspect) const
{

	const int kFirstInvalidIndex = static_cast<int>(EHmdSocialScreen::FirstInvalidIndex);

	if (pKeepAspect)
	{
		*pKeepAspect = m_pHmdSocialScreenKeepAspectCVar->GetIVal() != 0;
	}

	if (m_pHmdSocialScreenCVar->GetIVal() >= -1 && m_pHmdSocialScreenCVar->GetIVal() < kFirstInvalidIndex)
	{
		const EHmdSocialScreen socialScreenType = static_cast<EHmdSocialScreen>(m_pHmdSocialScreenCVar->GetIVal());
		return socialScreenType;
	}

	return EHmdSocialScreen::UndistortedDualImage; // default to dual distorted image
}

// -------------------------------------------------------------------------
void Device::PrintHmdInfo()
{
	// nada
}

// -------------------------------------------------------------------------
void Device::SubmitOverlay(int id)
{
	if (m_mapOverlayers.size()==0)
		return;

	if (m_overlays[id].overlayTexture)
	{
		m_overlays[id].submitted = true;
	}
	
}
Vec3 sPos = Vec3(0, 3000, 0);
Ang3 sAng = Ang3(0, 0, 0);
bool sNoRelative = true;
// -------------------------------------------------------------------------
void Device::SubmitFrame(const CryVR::Hypereal::SHmdSubmitFrameData& submitData)
{
	if (VrGraphicsCxt)
	{
		//update overlay pose
		for (int id = 0; id < RenderLayer::eQuadLayers_Total; id++)
		{
			if (m_overlays[id].submitted && m_overlays[id].visible)
			{
				MapOverlayer::iterator itFind = m_mapOverlayers.find(id);
				if (itFind != m_mapOverlayers.end())
				{
					HyViewLayer* pLayer = itFind->second.layerHandle;
					if (pLayer != nullptr)
					{
						SHmdRenderLayerInfo & _data = submitData.pQuadLayersArray[id];

						
						HyPose pose;

			 
						HyVec2 hySize;
						hySize.x = (float)_data.viewportSize.y;
						hySize.y = (float)_data.viewportSize.y;
						pLayer->SetSize(hySize);
						
						pLayer->SetTexture(itFind->second.textureDesc);

						Quat qut = Quat(sAng);

						if (/*sNoRelative*/_data.layerType== RenderLayer::ELayerType::eLayer_Quad|| _data.layerType == RenderLayer::ELayerType::eLayer_Scene3D)
						{
							QuatT qtLayer(_data.pose.q, _data.pose.t);
							//QuatT qtLayer(qut, sPos);
							Quat hmdRotation = HYQuatToQuat(m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_rotation);
							Vec3 hmdPosition = HYVec3ToVec3(m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_position);

							Quat qRecenterRotation = m_qBaseOrientation.GetInverted()*hmdRotation;
						
							QuatT qtRecentered(qRecenterRotation, hmdPosition);
							qtRecentered = qtRecentered.GetInverted()*qtLayer;
							//Vec3 vRecenterPosition = (sPos + m_localEyePoseStates.pose.position);
						
							pose.m_position = Vec3ToHYVec3(qtRecentered.t);
							pose.m_rotation = QuatToHYQuat(qtRecentered.q);
							//						pose.m_position = Vec3ToHYVec3(_data.pose.t);
							//						pose.m_rotation = QuatToHYQuat(_data.pose.q);// /*m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_rotation;*/ QuatToHYQuat(qRot);
							pLayer->SetPose(pose);

							pLayer->SetFlags(0);
						}
						else
						{
							pose.m_position = Vec3ToHYVec3(sPos);
							pose.m_rotation = QuatToHYQuat(qut);
							//						pose.m_position = Vec3ToHYVec3(_data.pose.t);
							//						pose.m_rotation = QuatToHYQuat(_data.pose.q);// /*m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_rotation;*/ QuatToHYQuat(qRot);
							pLayer->SetPose(pose);

							pLayer->SetFlags(HY_LAYER_FLAG_LOCK_TO_HELMET);
						}
					}
				}

			}
		}
		
		HyResult hr = hySuccess;

		// 		RTDesc[0].m_texture = resL->RenderTex;
		// 		RTDesc[1].m_texture = resR->RenderTex;
		hr = VrGraphicsCxt->Submit(m_lastFrameID_UpdateTrackingState, RTDesc, 2);
	}

}

// -------------------------------------------------------------------------
void Device::GetRenderTargetSize(uint& w, uint& h)
{
 	if (nullptr == VrGraphicsCxt)
 	{
 		w = 1200;
 		h = 1080;
 		return;
 	}
 	VrGraphicsCxt->GetRenderTargetSize(HY_EYE_LEFT, w, h);
//  	w = 1200;
//  	h = 1080;
}

// -------------------------------------------------------------------------
void Device::GetMirrorImageView(EEyeType eye, void* resource, void** mirrorTextureView)
{
	/*VrGraphicsCxt->CopyMirrorTexture(resource,(uint) VrDeviceInfo.DeviceResolutionX/2, (uint)VrDeviceInfo.DeviceResolutionY);*/
	//vr::VRCompositor()->GetMirrorTextureD3D11(static_cast<vr::EVREye>(eye), resource, mirrorTextureView);
}

// -------------------------------------------------------------------------
void Device::OnSetupEyeTargets(ERenderAPI api, ERenderColorSpace colorSpace, void* leftEyeHandle, void* rightEyeHandle)
{
	RTDesc[EEyeType::eEyeType_LeftEye].m_texture = leftEyeHandle;
	RTDesc[EEyeType::eEyeType_RightEye].m_texture = rightEyeHandle;
}

// -------------------------------------------------------------------------
void Device::OnSetupOverlay(int id,void* overlayTextureHandle)
{
	CreateLayer(id, overlayTextureHandle);
}

// -------------------------------------------------------------------------
void Device::OnDeleteOverlay(int id)
{
	DestroyLayer(id);
}


void Device::RebuildPlayArea()
{
	if (PlayAreaVertices)
	{
		delete[] PlayAreaVertices;
		PlayAreaVertices = nullptr;
	}

	if (hySucceeded(VrDevice->GetIntValue(HY_PROPERTY_CHAPERONE_VERTEX_COUNT_INT, PlayAreaVertexCount)) && PlayAreaVertexCount != 0)
	{
		PlayAreaVertices = new HyVec2[PlayAreaVertexCount];
		HyResult r = VrDevice->GetFloatArray(HY_PROPERTY_CHAPERONE_VERTEX_VEC2_ARRAY, reinterpret_cast<float*>(PlayAreaVertices),(uint) PlayAreaVertexCount * 2);

		bPlayAreaValid = hySucceeded(r);

		if (!bPlayAreaValid)
		{
			delete[] PlayAreaVertices;
			PlayAreaVertices = nullptr;
		}
	}
}

float Device::GetDistance(const HyVec2& P, const HyVec2& PA, const HyVec2& PB)
{
	float xx = PB.x - PA.x;
	float yy = PB.y - PA.y;
	return (-(yy * P.x - xx * P.y + PB.x * PA.y - PB.y * PA.x) / sqrt(xx * xx + yy * yy));
}

void Device::CreateGraphicsContext(void* graphicsDevice)
{
	if (nullptr == VrDevice)
		return;

	//graphic ctx should be ready
	HyGraphicsAPI graphicsAPI = HY_GRAPHICS_UNKNOWN;

	graphicsAPI = HY_GRAPHICS_D3D11;

	{
		VrGraphicsCxtDesc.m_mirrorWidth = gEnv->pRenderer->GetWidth();
		VrGraphicsCxtDesc.m_mirrorHeight = gEnv->pRenderer->GetHeight();
	}

	VrGraphicsCxtDesc.m_graphicsDevice = graphicsDevice;
	VrGraphicsCxtDesc.m_graphicsAPI = graphicsAPI;
	VrGraphicsCxtDesc.m_pixelFormat = HY_TEXTURE_R8G8B8A8_UNORM_SRGB;
	VrGraphicsCxtDesc.m_pixelDensity = PixelDensity;
	VrGraphicsCxtDesc.m_flags = 0;

	HyResult hr = VrDevice->CreateGraphicsContext(VrGraphicsCxtDesc, &VrGraphicsCxt);
	if (!hySucceeded(hr))
	{
		gEnv->pLog->Log("[HMD][Hypereal] CreateGraphicsContext failed.");
		return;
	}
}

void Device::ReleaseGraphicsContext()
{
	HY_RELEASE(VrGraphicsCxt);
}

void Device::CopyMirrorImage(void* pDstResource,uint nWidth,uint nHeight)
{
	if (VrGraphicsCxt)
	{
		VrGraphicsCxt->CopyMirrorTexture(pDstResource, nWidth, nHeight);
	}
}


void Device::CreateLayer(int id, void* overlayTextureHandle,bool bRecrate)
{
	if (nullptr == VrDevice || nullptr == VrGraphicsCxt)
		return;

	//remove old one with same id
	MapOverlayer::iterator itFind = m_mapOverlayers.find(id);
	if (itFind != m_mapOverlayers.end())
	{
		HyViewLayer* pLayer = itFind->second.layerHandle;
		pLayer->Release();
		m_mapOverlayers.erase(itFind);
	}

	HyViewLayer* pLayer = (HyViewLayer*)VrDevice->CreateClassInstance(HY_CLASS_VIEW_LAYER);

	if (nullptr == pLayer)
	{
		gEnv->pLog->Log("[HMD][Hypereal] Error creating overlay %i", id);
		return;
	}

	SOverlay newOverlayer;
	newOverlayer.layerHandle = pLayer;

	Matrix34 matPose = Matrix34::CreateTranslationMat(Vec3(0, 0, -m_hmdQuadDistance));

	//Quat qRot( );

	HyPose pose;
	pose.m_position = Vec3ToHYVec3(matPose.GetTranslation());
	pose.m_rotation = m_rTrackedDevicePose[EDevice::Hmd].m_pose.m_rotation;
	pLayer->SetPose(pose);
	HyVec2 hySize;
	hySize.x = 1000.0f;
	hySize.y = 1000.0f;
	pLayer->SetSize(hySize);


	HyTextureDesc hyTextureDesc;
	hyTextureDesc.m_texture = overlayTextureHandle;
	hyTextureDesc.m_uvOffset = HyVec2{ 0.0f , 0.0f };
	hyTextureDesc.m_uvSize = HyVec2{ 1.0f , 1.0f };
	hyTextureDesc.m_flags = 0;
	pLayer->SetTexture(hyTextureDesc);

	if (m_hmdQuadAbsolute)
		pLayer->SetFlags(0);
	else
		pLayer->SetFlags(HY_LAYER_FLAG_LOCK_TO_HELMET);

	pLayer->SetPriority(id);
	if (!bRecrate)
	{
		m_overlays[id].visible = true;
		m_overlays[id].submitted = true;
	}
	
	m_overlays[id].layerHandle = pLayer;
	m_overlays[id].overlayTexture = overlayTextureHandle;
	m_overlays[id].textureDesc = hyTextureDesc;

	m_mapOverlayers[id] = m_overlays[id];

}
void Device::DestroyLayer(int id)
{
	if (!VrDevice&&VrGraphicsCxt)
		return;

	//remove old one with same id
	MapOverlayer::iterator itFind = m_mapOverlayers.find(id);
	if (itFind != m_mapOverlayers.end())
	{
		HyViewLayer* pLayer = itFind->second.layerHandle;
		pLayer->Release();
		m_mapOverlayers.erase(itFind);

		memset(&m_overlays[id], 0, sizeof(m_overlays[id]));
	}
}
void Device::SetShowLayer(int id,bool bShow)
{
	if (!VrDevice&&VrGraphicsCxt)
		return;

	MapOverlayer::iterator itFind = m_mapOverlayers.find(id);
	if (itFind != m_mapOverlayers.end())
	{
		HyViewLayer* pLayer = itFind->second.layerHandle;
		if (pLayer!=nullptr)
		{
			if (!bShow) //remove from HMD
			{
				pLayer->Release();
				itFind->second.layerHandle = nullptr;
			}
			else //create from HMD
			{
				CreateLayer(id, itFind->second.overlayTexture, true);
			}
		}
	}
}

} // namespace Hypereal
} // namespace CryVR
