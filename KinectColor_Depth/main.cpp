//
//#include "ImageRenderer.h"
//#include "stdafx.h"
//#include <strsafe.h>
//
//static const int        cDepthWidth = 512;
//static const int        cDepthHeight = 424;
//
//// Current Kinect
//IKinectSensor*          m_pKinectSensor;
//
//// Depth reader
//IDepthFrameReader*      m_pDepthFrameReader;
//
//// Direct2D
//ImageRenderer*          m_pDrawDepth;
//ID2D1Factory*           m_pD2DFactory;
//RGBQUAD*                m_pDepthRGBX;
//
///// <summary>
///// Main processing function
///// </summary>
//void                    Update(boolean bsave);
//
///// <summary>
///// Initializes the default Kinect sensor
///// </summary>
///// <returns>S_OK on success, otherwise failure code</returns>
//HRESULT                 InitializeDefaultSensor();
//
///// <summary>
///// Handle new depth data
///// <param name="nTime">timestamp of frame</param>
///// <param name="pBuffer">pointer to frame data</param>
///// <param name="nWidth">width (in pixels) of input image data</param>
///// <param name="nHeight">height (in pixels) of input image data</param>
///// <param name="nMinDepth">minimum reliable depth</param>
///// <param name="nMaxDepth">maximum reliable depth</param>
///// </summary>
//void                    ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth, boolean bsave);
//
///// <summary>
///// Get the name of the file where screenshot will be stored.
///// </summary>
///// <param name="lpszFilePath">string buffer that will receive screenshot file name.</param>
///// <param name="nFilePathSize">number of characters in lpszFilePath string buffer.</param>
///// <returns>
///// S_OK on success, otherwise failure code.
///// </returns>
//HRESULT                 GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize);
//
///// <summary>
///// Save passed in image data to disk as a bitmap
///// </summary>
///// <param name="pBitmapBits">image data to save</param>
///// <param name="lWidth">width (in pixels) of input image data</param>
///// <param name="lHeight">height (in pixels) of input image data</param>
///// <param name="wBitsPerPixel">bits per pixel of image data</param>
///// <param name="lpszFilePath">full file path to output bitmap to</param>
///// <returns>indicates success or failure</returns>
//HRESULT                 SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath);
//
//int main()
//{
//	InitializeDefaultSensor();
//	while (1){
//		Update(false);
//	}
//	return 0;
//}
//
//
///// <summary>
///// Initializes the default Kinect sensor
///// </summary>
///// <returns>indicates success or failure</returns>
//HRESULT InitializeDefaultSensor()
//{
//
//	HRESULT hr;
//
//	hr = GetDefaultKinectSensor(&m_pKinectSensor);
//	if (FAILED(hr))
//	{
//		return hr;
//	}
//
//	if (m_pKinectSensor)
//	{
//		// Initialize the Kinect and get the depth reader
//		IDepthFrameSource* pDepthFrameSource = NULL;
//
//		hr = m_pKinectSensor->Open();
//
//		if (SUCCEEDED(hr))
//		{
//			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
//		}
//
//		SafeRelease(pDepthFrameSource);
//	}
//
//	if (!m_pKinectSensor || FAILED(hr))
//	{
//		printf("No ready Kinect found!");
//		return E_FAIL;
//	}
//	printf("Kinect ready!");
//	return hr;
//}
//
///// <summary>
///// Main processing function
///// </summary>
//void Update(boolean bsave)
//{
//	if (!m_pDepthFrameReader)
//	{
//		return;
//	}
//
//	IDepthFrame* pDepthFrame = NULL;
//
//	HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
//
//	if (SUCCEEDED(hr))
//	{
//		INT64 nTime = 0;
//		IFrameDescription* pFrameDescription = NULL;
//		int nWidth = 0;
//		int nHeight = 0;
//		USHORT nDepthMinReliableDistance = 0;
//		USHORT nDepthMaxDistance = 0;
//		UINT nBufferSize = 0;
//		UINT16 *pBuffer = NULL;
//
//		hr = pDepthFrame->get_RelativeTime(&nTime);
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pFrameDescription->get_Width(&nWidth);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pFrameDescription->get_Height(&nHeight);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			// In order to see the full range of depth (including the less reliable far field depth)
//			// we are setting nDepthMaxDistance to the extreme potential depth threshold
//			nDepthMaxDistance = USHRT_MAX;
//
//			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
//			//// hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
//		}
//
//		if (SUCCEEDED(hr))
//		{
//			ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance, bsave);
//		}
//
//		SafeRelease(pFrameDescription);
//	}
//
//	SafeRelease(pDepthFrame);
//}
//
///// <summary>
///// Handle new depth data
///// <param name="nTime">timestamp of frame</param>
///// <param name="pBuffer">pointer to frame data</param>
///// <param name="nWidth">width (in pixels) of input image data</param>
///// <param name="nHeight">height (in pixels) of input image data</param>
///// <param name="nMinDepth">minimum reliable depth</param>
///// <param name="nMaxDepth">maximum reliable depth</param>
///// </summary>
//void ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth, boolean bsave)
//{
//
//	// Make sure we've received valid data
//	if (m_pDepthRGBX && pBuffer && (nWidth == cDepthWidth) && (nHeight == cDepthHeight))
//	{
//		RGBQUAD* pRGBX = m_pDepthRGBX;
//
//		// end pixel is start + width*height - 1
//		const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);
//
//		while (pBuffer < pBufferEnd)
//		{
//			USHORT depth = *pBuffer;
//
//			// To convert to a byte, we're discarding the most-significant
//			// rather than least-significant bits.
//			// We're preserving detail, although the intensity will "wrap."
//			// Values outside the reliable depth range are mapped to 0 (black).
//
//			// Note: Using conditionals in this loop could degrade performance.
//			// Consider using a lookup table instead when writing production code.
//			BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);
//
//			pRGBX->rgbRed = intensity;
//			pRGBX->rgbGreen = intensity;
//			pRGBX->rgbBlue = intensity;
//
//			++pRGBX;
//			++pBuffer;
//		}
//
//		// Draw the data with Direct2D
//		m_pDrawDepth->Draw(reinterpret_cast<BYTE*>(m_pDepthRGBX), cDepthWidth * cDepthHeight * sizeof(RGBQUAD));
//
//		if (bsave)
//		{
//			WCHAR szScreenshotPath[MAX_PATH];
//
//			// Retrieve the path to My Photos
//			GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath));
//
//			// Write out the bitmap to disk
//			HRESULT hr = SaveBitmapToFile(reinterpret_cast<BYTE*>(m_pDepthRGBX), nWidth, nHeight, sizeof(RGBQUAD) * 8, szScreenshotPath);
//
//			WCHAR szStatusMessage[64 + MAX_PATH];
//			if (SUCCEEDED(hr))
//			{
//				// Set the status bar to show where the screenshot was saved
//				StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L"Screenshot saved to %s", szScreenshotPath);
//			}
//			else
//			{
//				StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L"Failed to write screenshot to %s", szScreenshotPath);
//			}
//			bsave = false; 
//		}
//	}
//}
//

#include "highgui.hpp"
#include "core/mat.hpp"
#include "stdafx.h"
#include "iostream"
#include <strsafe.h>
#include "string"
//#include "DepthBasics.h"

using namespace cv;
using namespace std;

// 转换depth图像到cv::Mat
cv::Mat ConvertMat(const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{
	cv::Mat img(nHeight, nWidth, CV_8UC3);
	uchar* p_mat = img.data;

	const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

	while (pBuffer < pBufferEnd)
	{
		USHORT depth = *pBuffer;

		BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

		*p_mat = intensity;
		p_mat++;
		*p_mat = intensity;
		p_mat++;
		*p_mat = intensity;
		p_mat++;

		++pBuffer;
	}
	return img;
}
// 转换color图像到cv::Mat
cv::Mat ConvertMat(const RGBQUAD* pBuffer, int nWidth, int nHeight)
{
	cv::Mat img(nHeight, nWidth, CV_8UC3);
	uchar* p_mat = img.data;

	const RGBQUAD* pBufferEnd = pBuffer + (nWidth * nHeight);

	while (pBuffer < pBufferEnd)
	{
		*p_mat = pBuffer->rgbBlue;
		p_mat++;
		*p_mat = pBuffer->rgbGreen;
		p_mat++;
		*p_mat = pBuffer->rgbRed;
		p_mat++;

		++pBuffer;
	}
	return img;
}

/// <summary>
/// Get the name of the file where screenshot will be stored.
/// </summary>
/// <param name="lpszFilePath">string buffer that will receive screenshot file name.</param>
/// <param name="nFilePathSize">number of characters in lpszFilePath string buffer.</param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize, boolean bdepth)
{
	WCHAR* pszKnownPath = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &pszKnownPath);

	if (SUCCEEDED(hr))
	{
		// Get the time
		WCHAR szTimeString[MAX_PATH];
		GetTimeFormatEx(NULL, 0, NULL, L"HH'-'mm'-'ss", szTimeString, _countof(szTimeString));

		// File name will be KinectScreenshotDepth-HH-MM-SS.bmp
		if (bdepth)
			StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\kinect\\KinectScreenshot-Depth-%s.bmp", pszKnownPath, szTimeString);
		else 
			StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\kinect\\KinectScreenshot-Color-%s.bmp", pszKnownPath, szTimeString);
	}

	if (pszKnownPath)
	{
		CoTaskMemFree(pszKnownPath);
	}

	return hr;
}

// wchar_t to string
void Wchar_tToString(std::string& szDst, wchar_t *wchar)
{
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	char *psText; 
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	szDst = psText;// std::string
	delete[]psText;// psText
}


void main()
{
	////////////////////////////////////////////////////////////////
	int depth_width = 512; //depth图像就是这么小
	int depth_height = 424;
	int color_widht = 1920; //color图像就是辣么大
	int color_height = 1080;

	cv::Mat depthImg_show = cv::Mat::zeros(depth_height, depth_width, CV_8UC3);//原始UINT16 深度图像不适合用来显示，所以需要砍成8位的就可以了，但是显示出来也不是非常好，最好能用原始16位图像颜色编码，凑合着看了
	cv::Mat depthImg = cv::Mat::zeros(depth_height, depth_width, CV_16UC1);//the depth image
	cv::Mat colorImg = cv::Mat::zeros(color_height, color_widht, CV_8UC3);//the color image
	// Current Kinect
	IKinectSensor* m_pKinectSensor = NULL;
	// Depth reader
	IDepthFrameReader*  m_pDepthFrameReader = NULL;
	// Color reader
	IColorFrameReader*  m_pColorFrameReader = NULL;
	RGBQUAD* m_pColorRGBX = new RGBQUAD[color_widht * color_height];
	//open it!
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		cout << "Can not find the Kinect!" << endl;
		cv::waitKey(0);
		exit(0);
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get the depth reader
		IDepthFrameSource* pDepthFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		SafeRelease(pDepthFrameSource);

		// for color
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		SafeRelease(pColorFrameSource);
	}

	//valify the depth reader
	if (!m_pDepthFrameReader)
	{
		cout << "Can not find the m_pDepthFrameReader!" << endl;
		cv::waitKey(0);
		exit(0);
	}
	//valify the color reader
	if (!m_pColorFrameReader)
	{
		cout << "Can not find the m_pColorFrameReader!" << endl;
		cv::waitKey(0);
		exit(0);
	}

	// get the data!
	UINT nBufferSize_depth = 0;
	UINT16 *pBuffer_depth = NULL;
	UINT nBufferSize_coloar = 0;
	RGBQUAD *pBuffer_color = NULL;

	cout << "Press the key 's' to save depth and color image, press the key 'q' to quit." << endl;
	while (true) // 貌似要一直尝试，不一定每次都能读取到图像
	{

		//color frame
		IColorFrame* pColorFrame = NULL;
		hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
		ColorImageFormat imageFormat = ColorImageFormat_None;
		if (SUCCEEDED(hr))
		{
			ColorImageFormat imageFormat = ColorImageFormat_None;
			if (SUCCEEDED(hr))
			{
				hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
			}
			if (SUCCEEDED(hr))
			{
				if (imageFormat == ColorImageFormat_Bgra)//这里有两个format，不知道具体含义，大概一个预先分配内存，一个需要自己开空间吧
				{
					hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize_coloar, reinterpret_cast<BYTE**>(&pBuffer_color));
				}
				else if (m_pColorRGBX)
				{
					pBuffer_color = m_pColorRGBX;
					nBufferSize_coloar = color_widht * color_height * sizeof(RGBQUAD);
					hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize_coloar, reinterpret_cast<BYTE*>(pBuffer_color), ColorImageFormat_Bgra);
				}
				else
				{
					hr = E_FAIL;
				}
				colorImg = ConvertMat(pBuffer_color, color_widht, color_height);
			}

			SafeRelease(pColorFrame);
		}

		// depth frame
		IDepthFrame* pDepthFrame = NULL;
		// acquire depth frame
		HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
		//// save depth data to array
		//UINT16 *depthData = new UINT16[depth_width * depth_height];
		//hr = pDepthFrame->CopyFrameDataToArray(depth_width * depth_height, depthData);

		//ICoordinateMapper*      m_pCoordinateMapper;
		//hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);

		//ColorSpacePoint* m_pColorCoordinates = new ColorSpacePoint[depth_width * depth_height];
		//HRESULT hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(depth_width * depth_height, depthData,
		//															depth_width * depth_height, m_pColorCoordinates);

		if (SUCCEEDED(hr))
		{
			USHORT nDepthMinReliableDistance = 0;
			USHORT nDepthMaxReliableDistance = 0;
			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
			}

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
			}
			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize_depth, &pBuffer_depth);
				depthImg_show = ConvertMat(pBuffer_depth, depth_width, depth_height, nDepthMinReliableDistance, nDepthMaxReliableDistance);
			}
		}
		SafeRelease(pDepthFrame);

		cv::imshow("depth", depthImg_show);
		cv::imshow("color", colorImg);

		char key = cv::waitKey(1);

		if (key == 'q')
		{
			cout << "exited" << endl;
			break;
		} 
		if (key == 's')
		{
			WCHAR szScreenshotPath[MAX_PATH];
			// Retrieve the path to My Photos
			GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath), false);
			string filename;
			Wchar_tToString(filename, szScreenshotPath);
			// Write out the color bitmap to disk
			cv::imwrite(filename, colorImg);
			wcout << "Color screenshot saved to" << szScreenshotPath << endl;
			
			// Retrieve the path to My Photos
			GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath), true);
			filename;
			Wchar_tToString(filename, szScreenshotPath);
			// Write out the depth bitmap to disk
			cv::imwrite(filename, depthImg_show);
			wcout << "Depth screenshot saved to" << szScreenshotPath << endl;
		}
	}
	
	// release resource
	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	cv::destroyAllWindows();
	SafeRelease(m_pKinectSensor);
}
