// ovrvision_ds.h
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

//Windows only
#ifdef WIN32

#ifndef __OVRVISION_DS__
#define __OVRVISION_DS__

/////////// INCLUDE ///////////

#include <windows.h>
#include <dshow.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <setjmp.h>

#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>

//for threading
#include <process.h>

#include "ovrvision_device.h"

//OVR Group
namespace OVR {

/////////// CLASS ///////////

//The use OVSampleGrabberCB class
class OVSampleGrabberCB;

//class
class OvrvisionDirectShow: public OvrvisionDevice
{
public:
	//Constructor/Destructor
	OvrvisionDirectShow();
	~OvrvisionDirectShow();

	//Control method
	// Create device
	int OpenDevice(int cam_w, int cam_h, int rate, int skip);

	//Delete device
	int CloseDevice();

	//Transfer status
	int StartTransfer();
	int StopTransfer();

	//Get pixel data
	//In non blocking, when data cannot be acquired, RESULT_FAILED returns. 
	int GetBayer16Image(unsigned char* pimage, bool nonblocking = false);

	//Set camera setting
	int SetCameraSetting(CamSetting proc, int value, bool automode);
	//Get camera setting
	int GetCameraSetting(CamSetting proc, int* value, bool* automode);

	//Property
	DevStatus GetDeviceStatus();
	int GetLatestPixelDataSize();
	int GetMaxPixelDataSize();

	//Callback
	void SetCallback(void(*func)());

private:
	//Device status
	DevStatus		m_devstatus;
	char			m_nDeviceName[OV_DEVICENAMENUM];

	//Pixel Size
	int				m_width;
	int				m_height;
	int				m_rate;
	int				m_latestPixelDataSize;
	int				m_maxPixelDataSize;

	bool			m_comInit;
	
	//Windows DirectShow
	IGraphBuilder*	m_pGraph;
	IBaseFilter*	m_pSrcFilter;
	IBaseFilter*	m_pDestFilter;
	IBaseFilter*	m_pGrabberFilter;
	ISampleGrabber* m_pSGrabber;
	IMediaControl*	m_pMediaControl;

	IAMVideoProcAmp*	m_pAMVideoProcAmp;	//controller
	IAMCameraControl*	m_pIAMCameraControl;

	//Callback class
	OVSampleGrabberCB* m_pSGCallback;

	//Private method

	void GetMediaSubtypeAsString(GUID type, char* typeAsString);
	IBaseFilter* GetSrcFilterFromID(ICreateDevEnum* pDev, usb_id vid, usb_id pid, int skip);
	int UsbidCmp(char* deviceString, usb_id vid, usb_id pid);
};

};

#endif /*__OVRVISION_DS__*/
#endif /*WIN32*/