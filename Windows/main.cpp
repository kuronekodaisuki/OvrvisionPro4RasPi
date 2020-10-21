#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "ovrvision_pro.h"

using namespace OVR;
using namespace cv;

#define WIDTH 640
#define HEIGHT	480

int main(int argc, char *argv[])
{
	Mat image(HEIGHT, WIDTH, CV_16UC1);
	OvrvisionPro camera;
	if (0 < camera.Open(0, Camprop::OV_CAM20VR_VGA))
	{
		//device.OpenDevice(WIDTH, HEIGHT, 30);
		//device.QueryCapability();
		//device.EnumFormats();
		//device.StartTransfer();
		unsigned char* left;
		unsigned char* right;

		for (bool loop = true; loop && camera.isOpen();)
		{
			camera.Capture(Camqt::OV_CAMQT_DMSRMP);
			left = camera.GetCamImageBGRA(Cameye::OV_CAMEYE_LEFT);
			right = camera.GetCamImageBGRA(Cameye::OV_CAMEYE_RIGHT);
			//if (0 == device.GetBayer16Image(image.data))
			//{
			//	imshow("Bayer", image);
			//}
			switch (waitKey(10))
			{
			case 'q':
				loop = false;
				break;
			case ' ':
				imwrite("frame.png", image);
				break;
			}
		}
		//device.StopTransfer();
		//device.CloseDevice();
		camera.Close();
	}
}

/* Result
Driver name     : uvcvideo
Driver Version  : 3.10.40
Device name     : OvrvisionPro
Bus information : usb-tegra-ehci.0-1
Capabilities    : 84000001h
V4L2_CAP_VIDEO_CAPTURE (Video Capture)
V4L2_CAP_STREAMING (Streaming I/O method)
*/

