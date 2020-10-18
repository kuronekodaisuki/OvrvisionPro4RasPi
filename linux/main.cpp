#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "ovrvision_v4l.h"

using namespace OVR;
using namespace cv;

#define WIDTH 640
#define HEIGHT	480

int main(int argc, char *argv[])
{
	Mat image(HEIGHT, WIDTH, CV_16UC1);
	OvrvisionVideo4Linux device;
	device.OpenDevice(WIDTH, HEIGHT, 30);
	//device.QueryCapability();
	//device.EnumFormats();
	device.StartTransfer();
	for (bool loop = true; loop;)
	{
		if (0 == device.GetBayer16Image(image.data))
		{
			imshow("Bayer", image);
		}
		switch (waitKey(10))
		{case 'q':
			loop = false;
			break;
		case ' ':
			imwrite("frame.png", image);
			break;
		}
	}
	device.StopTransfer();
	device.CloseDevice();
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

