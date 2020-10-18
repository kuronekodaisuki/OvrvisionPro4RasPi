// ovrvision_v4l.h
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

#define LINUX
//Linux only
#ifdef LINUX

#ifndef __OVRVISION_V4L__
#define __OVRVISION_V4L__

/////////// INCLUDE ///////////
#include <unistd.h>
#include <asm/types.h>
#include <linux/videodev2.h>	//!Video4Linux
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/v4l2-dv-timings.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>

#include "../src/ovrvision_device.h"

//Group
namespace OVR
{
	/////////// VARS AND DEFS ///////////
	//ID
	typedef unsigned short usb_id;

	typedef struct {
		void *start;
		size_t length;
	} V4L_BUFFER;

	/////////// CLASS ///////////
	//class
	class OvrvisionVideo4Linux: public OvrvisionDevice
	{
	public:
		//Constructor/Destructor
		OvrvisionVideo4Linux();
		~OvrvisionVideo4Linux();

		//Open device
		int OpenDevice(int width, int height, int frame_rate);

		//Delete device
		int CloseDevice();

		//Transfer status
		int StartTransfer();
		int StopTransfer();

		//Get pixel data
		//In non blocking, when data cannot be acquired, RESULT_FAILED returns. 
		int GetBayer16Image(unsigned char* pimage, bool nonblocking = false);
		int GetBayer16Image(unsigned char* pimage, size_t step, bool nonblocking = false);

		//Set camera setting
		int SetCameraSetting(CamSetting proc, int value, bool automode);
		//Get camera setting
		int GetCameraSetting(CamSetting proc, int* value, bool* automode);

		/*! @brief Check capability */
		int CheckCapability();

		/*! @brief Query capability of device */
		void QueryCapability();

		void EnumFormats();

		//Callback
		void SetCallback(void(*func)());

	protected:
		int SearchDevice(const char *name);
		int Init();

	private:
		int	_fd;
		unsigned int	_n_buffers;
		V4L_BUFFER *_buffers;
		bool _cropVertical;
		bool _cropHorizontal;
		struct v4l2_format _format;

		void(*m_get_callback)(void);
	};
}; // namespave OVR

#endif // __OVRVISION_V4L__

#endif // LINUX
