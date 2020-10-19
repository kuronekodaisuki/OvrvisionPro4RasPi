// ovrvisionPro.cpp
//
//MIT License
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
// Oculus Rift : TM & Copyright Oculus VR, Inc. All Rights Reserved
// Unity : TM & Copyright Unity Technologies. All Rights Reserved


/////////// INCLUDE ///////////

#include "ovrvision_pro.h"

#include "ovrvision_setting.h"
//#include "OvrvisionProCL.h"

#if defined(WIN32)
#include "../Windows/ovrvision_ds.h"
#elif defined(LINUX)
#include "../linux/ovrvision_v4l.h"
#elif defined(MACOSX)
#endif // WIN32

/////////// VARS AND DEFS ///////////

//OVRVISION USB SETTING
//Vender id
#define OV_USB_VENDERID		(0x2C33)
//Product id
#define OV_USB_PRODUCTID	(0x0001)	//Reversal

//PixelSize
#define OV_PIXELSIZE_RGB	(4)
#define OV_PIXELSIZE_BY8	(1)

//Challenge num
#define OV_CHALLENGENUM		(3)	//3

/////////// CLASS ///////////

//Group
namespace OVR
{
	//Constructor/Destructor
	OvrvisionPro::OvrvisionPro()
	{
	#if defined(WIN32)
		m_pDevice = new OvrvisionDirectShow();
	#elif defined(LINUX)
		m_pDevice = new OvrvisionVideo4Linux();
	#elif defined(MACOSX)
		m_pDevice = [[OvrvisionAVFoundation alloc] init];
	#endif

		m_pFrame = NULL;
		m_pPixels[0] = m_pPixels[1] = NULL;
		m_pOpenCL = NULL;

		m_width = 960;
		m_height = 950;
		m_framerate = 60;
		m_expo_f = 937156.80f;

		m_focalpoint = 1.0f;
		m_rightgap[0] = m_rightgap[1] = m_rightgap[2] = 0.0f;

		m_isOpen = false;
		m_isCameraSync = false;
	}

	OvrvisionPro::~OvrvisionPro()
	{
		Close();

	#if defined(WIN32)
		delete m_pDevice;
	#elif defined(LINUX)
		delete m_pDevice;
	#elif defined(MACOSX)
		[m_pDevice dealloc];
	#endif
	}

	//Initialize
	//Open the Ovrvision
	int OvrvisionPro::Open(int locationID, OVR::Camprop prop, const char *pVendorName, int deviceType, void *pDevice)
	{
		int objs = 0;
		int challenge;

		int cam_width;
		int cam_height;
		int cam_framerate;
		float cam_expo_f;

		if (m_isOpen)
			return 0;

		switch (prop) {
		case OV_CAM5MP_FULL:
			cam_width = 2560;
			cam_height = 1920;
			cam_framerate = 15;
			cam_expo_f = 480000.00f;
			break;
		case OV_CAM5MP_FHD:
			cam_width = 1920;
			cam_height = 1080;
			cam_framerate = 30;
			cam_expo_f = 570579.49f;
			break;
		case OV_CAMHD_FULL:
			cam_width = 1280;
			cam_height = 960;
			cam_framerate = 45;
			cam_expo_f = 720112.52f;
			break;
		case OV_CAMVR_FULL:
			cam_width = 960;
			cam_height = 950;
			cam_framerate = 60;
			cam_expo_f = 937156.80f;
			break;
		case OV_CAMVR_WIDE:
			cam_width = 1280;
			cam_height = 800;
			cam_framerate = 60;
			cam_expo_f = 783273.84f;
			break;
		case OV_CAMVR_VGA:
			cam_width = 640;
			cam_height = 480;
			cam_framerate = 90;
			cam_expo_f = 720112.52f;
			break;
		case OV_CAMVR_QVGA:
			cam_width = 320;
			cam_height = 240;
			cam_framerate = 120;
			cam_expo_f = 491520.00f;
			break;
		case OV_CAM20HD_FULL:
			cam_width = 1280;
			cam_height = 960;
			cam_framerate = 15;
			cam_expo_f = 240000.00f;
			break;
		case OV_CAM20VR_VGA:
			cam_width = 640;
			cam_height = 480;
			cam_framerate = 30;
			cam_expo_f = 240000.00f;
			break;
		default:
			cam_width = 960;
			cam_height = 950;
			cam_framerate = 60;
			cam_expo_f = 937156.80f;
			return 0;
		};

		//Open
		for(challenge = 0; challenge < OV_CHALLENGENUM; challenge++) {	//CHALLENGEN
			if (m_pDevice->OpenDevice(locationID, cam_width, cam_height, cam_framerate) == 0) {
				objs++;
				break;
			}
#ifdef WIN32
			Sleep(150);	//150ms wait
#else
			usleep(150000);	//150ms wait
#endif // WIN32

		}

		//Error
		if (objs == 0)
			return 0;

		m_pFrame = new ushort[cam_width*cam_height];
		m_pPixels[0] = new byte[cam_width*cam_height*OV_PIXELSIZE_RGB];
		m_pPixels[1] = new byte[cam_width*cam_height*OV_PIXELSIZE_RGB];

		//Initialize OpenCL system
		try {
			if (deviceType == 1 && pDevice != NULL)
			{
				m_pOpenCL = new OvrvisionProOpenCL(cam_width, cam_height, SHARING_MODE::D3D11, pDevice, pVendorName); // When use D3D11 sharing texture
			}
			else if (deviceType == 0)
			{
				m_pOpenCL = new OvrvisionProOpenCL(cam_width, cam_height, SHARING_MODE::OPENGL, NULL, pVendorName);    // When use OpenGL sharing texture
			}
			else
			{
				m_pOpenCL = new OvrvisionProOpenCL(cam_width, cam_height, SHARING_MODE::NONE, NULL, pVendorName);
			}
		}
		catch (std::exception ex)
		{
			printf("This OvrvisionSDK is the GPU necessity which is supporting OpenCL1.2 or more.\n");
			return 0;
		}
		//Opened
		m_isOpen = true;
		m_pDevice->StartTransfer();

		m_width = cam_width;
		m_height = cam_height;
		m_framerate = cam_framerate;
		m_expo_f = cam_expo_f;

		//Initialize Camera system
		InitCameraSetting();
		
		return objs;
	}

	//Close the Ovrvision
	void OvrvisionPro::Close()
	{
		if (!m_isOpen)
			return;
		
		m_pDevice->CloseDevice();

		if (m_pOpenCL) {
			delete m_pOpenCL;
			m_pOpenCL = NULL;
		}
		if (m_pFrame) {
			delete[] m_pFrame;
			m_pFrame = NULL;
		}
		if (m_pPixels[0]) {
			delete[] m_pPixels[0];
			delete[] m_pPixels[1];
			m_pPixels[0] = m_pPixels[1] = NULL;
		}

		m_isOpen = false;
	}

	// Capture frame
	void OvrvisionPro::Capture(OVR::Camqt qt)
	{
		if (!m_isOpen)
			return;

		if (qt == OV_CAMQT_NONE)
			return;

		if (m_pDevice->GetBayer16Image((uchar *)m_pFrame, !m_isCameraSync) == RESULT_OK)
		{
			if (qt == OV_CAMQT_DMSRMP)
				m_pOpenCL->DemosaicRemap(m_pFrame);	//OpenCL
			else if (qt == OV_CAMQT_DMS)
				m_pOpenCL->Demosaic(m_pFrame);		//OpenCL
		}
	}

	//Get camera image region of interest
	void  OvrvisionPro::GetStereoImageBGRA(unsigned char* pLeft, unsigned char* pRight, ROI roi)
	{
		m_pOpenCL->Read(pLeft, pRight, roi.offsetX, roi.offsetY, roi.width, roi.height);
	}

	void OvrvisionPro::InspectTextures(unsigned char *left, unsigned char *right, unsigned int type)
	{
		m_pOpenCL->InspectTextures(left, right, type);
	}

	//Get Camera data pre-store.
	void OvrvisionPro::PreStoreCamData(OVR::Camqt qt)
	{
		if (!m_isOpen)
			return;

		if (m_pDevice->GetBayer16Image((uchar *)m_pFrame, !m_isCameraSync) == RESULT_OK)
		{
			if (qt == OV_CAMQT_DMSRMP)
				m_pOpenCL->DemosaicRemap(m_pFrame, m_pPixels[0], m_pPixels[1]);	//OpenCL
			else if (qt == OV_CAMQT_DMS)
				m_pOpenCL->Demosaic(m_pFrame, m_pPixels[0], m_pPixels[1]);		//OpenCL
			else {
				byte* p_left = m_pPixels[0];
				byte* p_right = m_pPixels[1];
				for (int y = 0; y < m_height; y++) {
					for (int x = 0; x < m_width; x++) {
						int ps = (y * m_width) + x;
						int dest_ps = ps * 4;
						p_left[dest_ps + 0] = p_left[dest_ps + 1] = p_left[dest_ps + 2] = p_left[dest_ps + 3] = (m_pFrame[ps] & 0x00FF);
						p_right[dest_ps + 0] = p_right[dest_ps + 1] = p_right[dest_ps + 2] = p_right[dest_ps + 3] = (m_pFrame[ps] >> 8);
					}
				}

			}
		}
	}

	unsigned char* OvrvisionPro::GetCamImageBGRA(OVR::Cameye eye)
	{
		return m_pPixels[(int)eye];
	}

	void OvrvisionPro::GetCamImageBGRA(unsigned char* pImageBuf, OVR::Cameye eye)
	{
		memcpy(pImageBuf, m_pPixels[(int)eye], m_width*m_height*OV_PIXELSIZE_RGB);
	}

	void OvrvisionPro::SetCallbackImageFunction(void(*func)())
	{
		m_pDevice->SetCallback(func);
	}

	//Private method
	void OvrvisionPro::InitCameraSetting()
	{
		//Read files.
		OvrvisionSetting ovrset(this);
		if (ovrset.ReadEEPROM()) {
			
			//Read Set
			SetCameraExposure(ovrset.m_propExposure);
			SetCameraGain(ovrset.m_propGain);
			SetCameraBLC(ovrset.m_propBLC);
			SetCameraWhiteBalanceAuto((bool)ovrset.m_propWhiteBalanceAuto);
			if (ovrset.m_propWhiteBalanceAuto == 0) {
				SetCameraWhiteBalanceR(ovrset.m_propWhiteBalanceR);
				SetCameraWhiteBalanceG(ovrset.m_propWhiteBalanceG);
				SetCameraWhiteBalanceB(ovrset.m_propWhiteBalanceB);
			}
			//50ms wait
#ifdef WIN32
			Sleep(50);
#else
			usleep(50000);
#endif // WIN32

			//calc to Pixel
			ovrset.m_focalPoint.at<float>(0) *= ((ovrset.m_pixelSize.width / SensorSizeWidth) * SensorSizeScale);
			m_focalpoint = ovrset.m_focalPoint.at<float>(0);
			m_rightgap[0] = (float)-ovrset.m_trans.at<double>(0);	//T:X
			m_rightgap[1] = (float)ovrset.m_trans.at<double>(1);	//T:Y
			m_rightgap[2] = (float)ovrset.m_trans.at<double>(2);	//T:Z
		}

		if (m_pOpenCL)
			m_pOpenCL->LoadCameraParams(&ovrset);
	}

	bool OvrvisionPro::isOpen(){
		return m_isOpen;
	}

	int OvrvisionPro::GetCamWidth(){
		return m_width;
	}
	int OvrvisionPro::GetCamHeight(){
		return m_height;
	}
	int OvrvisionPro::GetCamFramerate(){
		return m_framerate;
	}
	int OvrvisionPro::GetCamBuffersize(){
		return m_width*m_height*OV_PIXELSIZE_RGB;
	}
	int OvrvisionPro::GetCamPixelsize(){
		return OV_PIXELSIZE_RGB;
	}

	float OvrvisionPro::GetCamFocalPoint(){
		return m_focalpoint;
	}
	float OvrvisionPro::GetHMDRightGap(int at){
		return m_rightgap[at];
	}

	//Thread sync
	void OvrvisionPro::SetCameraSyncMode(bool value){
		m_isCameraSync = value;
	}

	//Camera Propaty
	int OvrvisionPro::GetCameraExposure(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		return value;
	}

	void OvrvisionPro::SetCameraExposure(int value){
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 32767)	//high
			value = 32767;

		// Number is divided by 8
		value -= value % 8;

		m_pDevice->SetCameraSetting(OV_CAMSET_EXPOSURE, value, false);
	}
	
	bool OvrvisionPro::SetCameraExposurePerSec(float fps){
		if (!m_isOpen)
			return false;

		//The range specification
		if (fps < 25.0f)	//low
			fps = 25.0f;
		if (fps > 240.0f)	//high
			fps = 240.0f;

		fps += 0.5f;
		if ((float)m_framerate > fps) {
			return false;
		}

		int fpsint = (int)(m_expo_f / fps);
		SetCameraExposure(fpsint);
		return true;
	}

	int OvrvisionPro::GetCameraGain(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		m_pDevice->GetCameraSetting(OV_CAMSET_GAIN, &value, &automode);
		return value;
	}

	void OvrvisionPro::SetCameraGain(int value){
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 47)	//high
			value = 47;

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_GAIN, value, false);
	}

	/*!	@brief Set white balance gain of the Ovrvision.
		@param value gain. Range of 1000K - 12000K */
	void OvrvisionPro::SetCameraWhiteBalanceKelvin(int kelvin)
	{
		if (!m_isOpen)
			return;

		int temp = kelvin / 100;
		int red, green, blue;

		if (temp <= 66) {
			red = 255;
			green = temp;
			green = 99.4708025861 * std::log(green) - 161.1195681661;

			if (temp <= 19) {
				blue = 0;
			}
			else {
				blue = temp - 10;
				blue = 138.5177312231 * std::log(blue) - 305.0447927307;
			}

		}
		else {
			red = temp - 60;
			red = 329.698727446 * std::pow(red, -0.1332047592);
			green = temp - 60;
			green = 288.1221695283 * std::pow(green, -0.0755148492);
			blue = 255;
		}

		SetCameraWhiteBalanceR(red * 16);
		SetCameraWhiteBalanceG(green * 16);
		SetCameraWhiteBalanceB(blue * 16);
	}

	int OvrvisionPro::GetCameraWhiteBalanceR(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		m_pDevice->GetCameraSetting(OV_CAMSET_WHITEBALANCER, &value, &automode);
		return value;
	}

	void OvrvisionPro::SetCameraWhiteBalanceR(int value){
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 4095)	//high
			value = 4095;

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_WHITEBALANCER, value, false);
	}

	int OvrvisionPro::GetCameraWhiteBalanceG(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		m_pDevice->GetCameraSetting(OV_CAMSET_WHITEBALANCEG, &value, &automode);
		return value;
	}
	
	void OvrvisionPro::SetCameraWhiteBalanceG(int value){
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 4095)	//high
			value = 4095;

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_WHITEBALANCEG, value, false);
	}
	
	int OvrvisionPro::GetCameraWhiteBalanceB(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		m_pDevice->GetCameraSetting(OV_CAMSET_WHITEBALANCEB, &value, &automode);
		return value;
	}
	
	void OvrvisionPro::SetCameraWhiteBalanceB(int value) {
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 4095)	//high
			value = 4095;

		bool curval = GetCameraWhiteBalanceAuto();

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_WHITEBALANCEB, value, curval);
	}

	int OvrvisionPro::GetCameraBLC(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return (-1);

		m_pDevice->GetCameraSetting(OV_CAMSET_BLC, &value, &automode);
		return value;
	}
	
	void OvrvisionPro::SetCameraBLC(int value){
		if (!m_isOpen)
			return;

		//The range specification
		if (value < 0)	//low
			value = 0;
		if (value > 1023)	//high
			value = 1023;

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_BLC, value, false);
	}

	bool OvrvisionPro::GetCameraWhiteBalanceAuto(){
		int value = 0;
		bool automode = false;

		if (!m_isOpen)
			return false;

		m_pDevice->GetCameraSetting(OV_CAMSET_WHITEBALANCEB, &value, &automode);
		return automode;
	}

	void OvrvisionPro::SetCameraWhiteBalanceAuto(bool value){
		if (!m_isOpen)
			return;

		int curval = GetCameraWhiteBalanceB();

		//set
		m_pDevice->SetCameraSetting(OV_CAMSET_WHITEBALANCEB, curval, value);
	}

	//EEPROM
	void OvrvisionPro::UserDataAccessUnlock(){
		if (!m_isOpen)
			return;

		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00007000, false);
		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00006000, false);
		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00007000, false);
	}

	void OvrvisionPro::UserDataAccessLock() {
		if (!m_isOpen)
			return;
		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00000000, false);
	}
	
	void OvrvisionPro::UserDataAccessSelectAddress(unsigned int addr) {
		if (!m_isOpen)
			return;

		addr &= 0x000001FF;
		addr |= 0x00001000;	//cmd

		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, addr, false);
	}
	
	unsigned char OvrvisionPro::UserDataAccessGetData() {

		int value = 0;
		bool automode_none = false;

		if (!m_isOpen)
			return 0x00;

		m_pDevice->GetCameraSetting(OV_CAMSET_DATA, &value, &automode_none);
		return (unsigned char)value;
	}
	
	void OvrvisionPro::UserDataAccessSetData(unsigned char value){
		if (!m_isOpen)
			return;

		int value_int = (int)value;
		value_int |= 0x00002000;	//cmd

		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, value_int, false);

	}
	
	void OvrvisionPro::UserDataAccessSave(){
		if (!m_isOpen)
			return;

		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00003000, false);
	}

	void OvrvisionPro::UserDataAccessCheckSumAddress(){
		if (!m_isOpen)
			return;

		m_pDevice->SetCameraSetting(OV_CAMSET_DATA, 0x00005000, false);
	}

	bool OvrvisionPro::CameraParamSaveEEPROM(){
		OvrvisionSetting ovrset(this);
		bool rt;
		
		//Distort param is readed.
		ovrset.m_propExposure = GetCameraExposure();
		ovrset.m_propGain = GetCameraGain();
		ovrset.m_propBLC = GetCameraBLC();
		ovrset.m_propWhiteBalanceR = GetCameraWhiteBalanceR();
		ovrset.m_propWhiteBalanceG = GetCameraWhiteBalanceG();
		ovrset.m_propWhiteBalanceB = GetCameraWhiteBalanceB();
		ovrset.m_propWhiteBalanceAuto = (char)GetCameraWhiteBalanceAuto();

		//Write
		rt = ovrset.WriteEEPROM(WRITE_EEPROM_FLAG_CAMERASETWR);
		//50ms wait
#ifdef WIN32
		Sleep(50);
#else
		usleep(50000);
#endif // WIN32
		return rt;
	}

	bool OvrvisionPro::CameraParamResetEEPROM(){
		OvrvisionSetting ovrset(this);
		bool rt;

		//Write
		rt = ovrset.ResetEEPROM();

		//50ms wait
#ifdef WIN32
		Sleep(50);
#else
		usleep(50000);
#endif // WIN32
		return rt;
	}

#ifdef ENABLE_HAND_CAPTURE
	// Create textures(OpenGL)
	void OvrvisionPro::CreateSkinTextures(int width, int height, unsigned int left, unsigned int right)
	{
		m_pOpenCL->CreateSkinTextures(width, height, (TEXTURE)left, (TEXTURE)right);
	}

	// Update textures(OpenGL)
	void OvrvisionPro::UpdateSkinTextures(unsigned int left, unsigned int right)
	{
		m_pOpenCL->UpdateSkinTextures((TEXTURE)left, (TEXTURE)right);
	}

	// Update scaled image texture
	void OvrvisionPro::UpdateImageTextures(unsigned int left, unsigned int right)
	{
		m_pOpenCL->UpdateImageTextures((TEXTURE)left, (TEXTURE)right);
	}
#endif // ENABLE_HAND_CAPTURE


};
