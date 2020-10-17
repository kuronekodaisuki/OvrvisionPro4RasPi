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

#pragma once

namespace OVR
{
	/////////// VARS AND DEFS ///////////

	//Function status
	#define RESULT_OK		(0)
	#define RESULT_FAILED	(1)

	//RGB color data pixel byte
	#define OV_RGB_COLOR	(3)

	//Device name buffer size
	#define OV_DEVICENAMENUM	(256)

	//Blocking timeout
	#define OV_BLOCKTIMEOUT		(3000)	//3s

	//ID
	typedef unsigned short usb_id;

	//Device Status
	typedef enum ov_devstatus {
		OV_DEVNONE = 0,
		OV_DEVCREATTING,
		OV_DEVSTOP,
		OV_DEVRUNNING,
	} DevStatus;

	//Camera Setting enum
	typedef enum ov_camseet {
		OV_CAMSET_EXPOSURE = 0,		//Exposure
		OV_CAMSET_GAIN,				//Gain
		OV_CAMSET_WHITEBALANCER,	//Saturation
		OV_CAMSET_WHITEBALANCEG,	//Brightness
		OV_CAMSET_WHITEBALANCEB,	//Sharpness
		OV_CAMSET_BLC,				//Backlight Compensation
		OV_CAMSET_DATA,				//EEPROM Data Access
	} CamSetting;

	// 
	class OvrvisionDevice
	{
	public:
		// Open device
		virtual int Open(int width, int height, int fps) = 0;

		// Close device
		virtual void Close() = 0;

		//Transfer status
		virtual int StartTransfer() = 0;
		virtual int StopTransfer() = 0;

		//Get pixel data
		//In non blocking, when data cannot be acquired, RESULT_FAILED returns. 
		virtual int GetBayer16Image(unsigned char* pimage, bool nonblocking = false) = 0;

		//Set camera setting
		virtual int SetCameraSetting(CamSetting proc, int value, bool automode) = 0;
		//Get camera setting
		virtual int GetCameraSetting(CamSetting proc, int* value, bool* automode) = 0;

		//Callback
		virtual void SetCallback(void(*func)()) = 0;

	protected:
		//Device status
		DevStatus		m_devstatus;
		char m_nDeviceName[OV_DEVICENAMENUM];

		//Pixel Size
		int				m_width;
		int				m_height;
		int				m_rate;
		int				m_latestPixelDataSize;
		int				m_maxPixelDataSize;
	};
}



