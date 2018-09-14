/*****    Measurement of height by kinect            ******/
/*****    VisualStudio 2012 (开发工具)
OpenCV2.4.8 (显示界面库 vc11库)
KinectSDK-v2.0-PublicPreview1409-Setup (Kinect SDK驱动版本)
Windows 8.1 (操作系统)                   ******/
/*****    shihz                                    ******/
/*****    2015-2-7                                ******/

#include"stdafx.h"  
#include "opencv2/opencv.hpp"  

#define Y 160  
using namespace cv;


//定义Kinect方法类  
class Kinect
{
public:
	static const int        cDepthWidth = 512;   //深度图的大小  
	static const int        cDepthHeight = 424;

	static const int        cColorWidth = 1920;   //彩色图的大小  
	static const int        cColorHeight = 1080;
	Mat showImageDepth;

	HRESULT                 InitKinect();//初始化Kinect  
	void                    UpdateDepth();//更新深度数据  
	void                    UpdateColor();//更新深度数据  
	void                    ProcessDepth(const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth);   //处理得到的深度图数据  
	void                    ProcessColor(RGBQUAD* pBuffer, int nWidth, int nHeight);   //处理得到的彩色图数据  

	Kinect();                                     //构造函数  
	~Kinect();                                     //析构函数  

private:

	IKinectSensor*          m_pKinectSensor;// Current Kinect  
	IDepthFrameReader*      m_pDepthFrameReader;// Depth reader    在需要的时候可以再添加IColorFrameReader,进行color reader  
	RGBQUAD*                m_pDepthRGBX;
	IColorFrameReader*      m_pColorFrameReader;// Color reader  
	RGBQUAD*                m_pColorRGBX;
};

int main2()
{
	Kinect kinect;
	Mat showImageColor;
	kinect.InitKinect();
	while (1)
	{
		kinect.UpdateColor();                          //程序的中心内容，获取数据并显示  
		kinect.UpdateDepth();
		if (waitKey(1) >= 0)//按下任意键退出  
		{
			break;
		}
	}


	return 0;
}

Kinect::Kinect()
{
	m_pKinectSensor = NULL;
	m_pColorFrameReader = NULL;
	m_pDepthFrameReader = NULL;

	m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight];// create heap storage for color pixel data in RGBX format  ，开辟一个动态存储区域  
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];// create heap storage for color pixel data in RGBX format  
}

Kinect::~Kinect()                        //定义析构函数  
{
	if (m_pDepthRGBX)
	{
		delete[] m_pDepthRGBX;            //删除动态存储区域  
		m_pDepthRGBX = NULL;
	}

	SafeRelease(m_pDepthFrameReader);// done with depth frame reader  

	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}

	SafeRelease(m_pColorFrameReader);// done with color frame reader  

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();// close the Kinect Sensor  
	}
	SafeRelease(m_pKinectSensor);
}

HRESULT Kinect::InitKinect()            //定义初始化kinect函数  
{
	HRESULT hr;                           //typedef long HRESULT  

	hr = GetDefaultKinectSensor(&m_pKinectSensor);      //获取默认的kinect，一般来说只有用一个kinect，所以默认的也就是唯一的那个  
	if (FAILED(hr))                                //Failed这个函数的参数小于0的时候返回true else 返回false  
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the depth reader  
		IDepthFrameSource* pDepthFrameSource = NULL;
		IColorFrameSource* pColorFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);      //初始化Depth reader，传入该IDepthReader的地址，让该指针指向深度数据流  
		}

		SafeRelease(pColorFrameSource);
		SafeRelease(pDepthFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		printf("No ready Kinect found! \n");
		return E_FAIL;
	}

	return hr;
}

void Kinect::UpdateDepth()
{
	if (!m_pDepthFrameReader)
	{
		return;
	}

	IDepthFrame* pDepthFrame = NULL;

	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	if (SUCCEEDED(hr))
	{
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			// In order to see the full range of depth (including the less reliable far field depth)  
			// we are setting nDepthMaxDistance to the extreme potential depth threshold  
			nDepthMaxDistance = USHRT_MAX;

			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.  
			// hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);  
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			ProcessDepth(pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pDepthFrame);
}

void Kinect::UpdateColor()
{
	if (!m_pColorFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = NULL;

	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = NULL;

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			ProcessColor(pBuffer, nWidth, nHeight);
		}

		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}

void Kinect::ProcessDepth(const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{
	// Make sure we've received valid data  
	if (m_pDepthRGBX && pBuffer && (nWidth == cDepthWidth) && (nHeight == cDepthHeight))
	{
		RGBQUAD* pRGBX = m_pDepthRGBX;

		// end pixel is start + width*height - 1  
		const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

		while (pBuffer < pBufferEnd)
		{
			USHORT depth = *pBuffer;

			// To convert to a byte, we're discarding the most-significant  
			// rather than least-significant bits.  
			// We're preserving detail, although the intensity will "wrap."  
			// Values outside the reliable depth range are mapped to 0 (black).  

			// Note: Using conditionals in this loop could degrade performance.  
			// Consider using a lookup table instead when writing production code.  
			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);     //深度数据由黑白图像显示//，每256个单位是一个有黑到白的周期  


			pRGBX->rgbRed = intensity;
			pRGBX->rgbGreen = intensity;
			pRGBX->rgbBlue = intensity;

			++pRGBX;
			++pBuffer;
		}

		// Draw the data with OpenCV  
		Mat DepthImage(nHeight, nWidth, CV_8UC4, m_pDepthRGBX);
		Mat show = DepthImage.clone();

		resize(DepthImage, show, Size(cDepthWidth*1.437, cDepthHeight*1.437));
		showImageDepth = show.clone();

		imshow("DepthImage", show);
	}
}

void Kinect::ProcessColor(RGBQUAD* pBuffer, int nWidth, int nHeight)
{
	// Make sure we've received valid data     
	if (pBuffer && (nWidth == cColorWidth) && (nHeight == cColorHeight))
	{
		// Draw the data with OpenCV  
		Mat ColorImage(nHeight, nWidth, CV_8UC4, pBuffer);
		Mat showImage = ColorImage.clone();
		resize(ColorImage, showImage, Size(nWidth / 2, nHeight / 2));
		Rect rect(145, 0, 702, 538);
		int x = 33, y = -145;
		//CV_8UC4   
		for (int i = 0; i <540; i++)
			for (int j = 145; j < 960 - 114; j++)
				showImage.at<Vec4b>(i, j) = showImageDepth.at<Vec4b>(i + x, j + y)*0.6 + showImage.at<Vec4b>(i, j)*0.4;
		Mat image_roi = showImage(rect);
		//imshow("ColorImage", showImage);//imshow("ColorImage", ColorImage);  
		imshow("Image_Roi", image_roi);
	}
}