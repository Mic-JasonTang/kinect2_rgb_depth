#include "highgui.hpp"
#include "core/mat.hpp"
#include "stdafx.h"
#include "iostream"
#include <strsafe.h>
#include "string"
#include <thread>

using namespace cv;
using namespace std;

// ת��depthͼ��cv::Mat
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
// ת��colorͼ��cv::Mat
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
HRESULT GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize, int filetype)
{
	WCHAR* pszKnownPath = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &pszKnownPath);

	if (SUCCEEDED(hr))
	{
		// Get the time
		WCHAR szTimeString[MAX_PATH];
		GetTimeFormatEx(NULL, 0, NULL, L"HH'-'mm'-'ss", szTimeString, _countof(szTimeString));

		// File name will be KinectScreenshotDepth-HH-MM-SS.bmp
		if (filetype == 1)
			StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\kinect\\KinectScreenshot-Color-%s.bmp", pszKnownPath, szTimeString);
		else if (filetype == 2)
			StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\kinect\\KinectScreenshot-Depth-%s.bmp", pszKnownPath, szTimeString);
		else if (filetype == 3)
			StringCchPrintfW(lpszFilePath, nFilePathSize, L"%s\\kinect\\KinectScreenshot-DepthColor-%s.bmp", pszKnownPath, szTimeString);

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
	//��������
	int depth_width = 512; //depthͼ�������ôС
	int depth_height = 424;
	int color_width = 1920; //colorͼ�������ô��
	int color_height = 1080;

	// ��ʾ�õľ���
	cv::Mat depthImg_show = cv::Mat::zeros(depth_height, depth_width, CV_8UC3);//ԭʼUINT16 ���ͼ���ʺ�������ʾ��������Ҫ����8λ�ľͿ����ˣ�������ʾ����Ҳ���Ƿǳ��ã��������ԭʼ16λͼ����ɫ���룬�պ��ſ���
	cv::Mat depthImg = cv::Mat::zeros(depth_height, depth_width, CV_16UC1);//the depth image
	cv::Mat colorImg = cv::Mat::zeros(color_height, color_width, CV_8UC3);//the color image
	
	// Kinect ������
	IKinectSensor* m_pKinectSensor = NULL;
	// Depth reader
	IDepthFrameReader*  m_pDepthFrameReader = NULL;
	// Color reader
	IColorFrameReader*  m_pColorFrameReader = NULL;
	// Initialize the Kinect and get the depth reader
	IDepthFrameSource* pDepthFrameSource = NULL;

	RGBQUAD* m_pColorRGBX = new RGBQUAD[color_width * color_height];
	HRESULT hr;

	// Buffer
	UINT nBufferSize_depth = 0;
	UINT16 *pBuffer_depth = NULL;
	UINT nBufferSize_coloar = 0;
	RGBQUAD *pBuffer_color = NULL;

	// ���ڶ����2������
	Mat i_rgb(color_height, color_width, CV_8UC4);      //ע�⣺�������Ϊ4ͨ����ͼ��Kinect������ֻ����Bgra��ʽ����
	Mat depthToRgb(depth_height, depth_width, CV_8UC4);

	//color frame
	IColorFrame* pColorFrame = NULL;
	// color size
	UINT nColorBufferSize = color_width * color_height * 4;

	ColorImageFormat imageFormat = ColorImageFormat_None;
	// depth frame
	IDepthFrame* pDepthFrame = NULL;
	
	// Mapper
	ICoordinateMapper*      m_pCoordinateMapper = NULL;
	
	//depth data
	UINT16 *depthData = new UINT16[depth_width * depth_height];

	// ���ڶ���ĵ�
	ColorSpacePoint* m_pColorCoordinates = new ColorSpacePoint[depth_width * depth_height];
	
	// 1. ��ȡKinect������
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	
	if (FAILED(hr))
	{
		cout << "Can not find the Kinect!" << endl;
		cv::waitKey(0);
		exit(0);
	}

	if (m_pKinectSensor)
	{
		// 2. ��Kinect
		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			//3. ��ȡ���Դ
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			//4. ��ȡ�����Ⱦ
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}
		SafeRelease(pDepthFrameSource);

		// for color
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;
		if (SUCCEEDED(hr))
		{
			//5. ��ȡ��ɫԴ
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			//6. ��ȡ��ɫ��Ⱦ
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

	cout << "Press the key 's' to save depth and color image, press the key 'q' to quit." << endl;
	// ʹ���߳���ִ��
	thread main_thread;
	thread print_thread;

	main_thread = std::thread([&]{
		while (true) // ò��Ҫһֱ���ԣ���һ��ÿ�ζ��ܶ�ȡ��ͼ��
		{
			// 7. ��ȡ��ɫ֡
			hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

			if (SUCCEEDED(hr))
				// 8. ��ɫͼƬ��������
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(i_rgb.data), ColorImageFormat::ColorImageFormat_Bgra);

			if (SUCCEEDED(hr))
			{
				// 9. ��ȡ��ɫ��ʽ
				hr = pColorFrame->get_RawColorImageFormat(&imageFormat);

				if (SUCCEEDED(hr))
				{
					if (imageFormat == ColorImageFormat_Bgra)//����������format����֪�����庬�壬���һ��Ԥ�ȷ����ڴ棬һ����Ҫ�Լ����ռ��
					{
						hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize_coloar, reinterpret_cast<BYTE**>(&pBuffer_color));
					}
					else if (m_pColorRGBX)
					{
						pBuffer_color = m_pColorRGBX;
						nBufferSize_coloar = color_width * color_height * sizeof(RGBQUAD);
						// 10. ��ɫͼƬ��������
						hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize_coloar, reinterpret_cast<BYTE*>(pBuffer_color), ColorImageFormat_Bgra);
					}
					else
					{
						hr = E_FAIL;
					}
					// 11. ʹ��opencvת��ΪͼƬ��ʽ
					colorImg = ConvertMat(pBuffer_color, color_width, color_height);
				}
				SafeRelease(pColorFrame);
			}

			// acquire depth frame
			hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

			if (SUCCEEDED(hr))
			{
				//12. �����������������
				hr = pDepthFrame->CopyFrameDataToArray(depth_width * depth_height, depthData);

				//13. �������뺯��
				hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
				hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(depth_width * depth_height, depthData,
					depth_width * depth_height, m_pColorCoordinates);

				if (SUCCEEDED(hr))
				{
					//14. �������ͼ��ɫͼ�Ķ���
					for (int i = 0; i < depth_width * depth_height; i++)
					{
						ColorSpacePoint p = m_pColorCoordinates[i];
						if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
						{
							int colorX = static_cast<int>(p.X + 0.5f);
							int colorY = static_cast<int>(p.Y + 0.5f);

							if ((colorX >= 0 && colorX < color_width) && (colorY >= 0 && colorY < color_height))
							{
								depthToRgb.data[i * 4 + 0] = i_rgb.data[(colorY * color_width + colorX) * 4 + 0];
								depthToRgb.data[i * 4 + 1] = i_rgb.data[(colorY * color_width + colorX) * 4 + 1];
								depthToRgb.data[i * 4 + 2] = i_rgb.data[(colorY * color_width + colorX) * 4 + 2];
								depthToRgb.data[i * 4 + 3] = i_rgb.data[(colorY * color_width + colorX) * 4 + 3];
							}
						}
					}
				}
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
			cv::imshow("rgb2depth", depthToRgb);
			char key = cv::waitKey(1);

			if (key == 'q')
			{
				cout << "exited" << endl;
				break;
			}
			if (key == 's')
			{
				WCHAR szScreenshotPath[MAX_PATH];
				string filename;
				
				// Retrieve the path to My Photos
				GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath), 1);
				Wchar_tToString(filename, szScreenshotPath);
				// Write out the color bitmap to disk
				cv::imwrite(filename, colorImg);
				wcout << "Color screenshot saved to " << szScreenshotPath << endl;

				GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath), 2);
				filename;
				Wchar_tToString(filename, szScreenshotPath);
				// Write out the depth bitmap to disk
				cv::imwrite(filename, depthImg_show);
				wcout << "Depth screenshot saved to " << szScreenshotPath << endl;

				GetScreenshotFileName(szScreenshotPath, _countof(szScreenshotPath), 3);
				filename;
				Wchar_tToString(filename, szScreenshotPath);
				// Write out the depth bitmap to disk
				cv::imwrite(filename, depthToRgb);
				wcout << "DepthColor screenshot saved to " << szScreenshotPath << endl;
			}
			if (key == 'p')
			{
				print_thread = std::thread([&]{
					if (m_pCoordinateMapper != NULL)
					{
						CameraSpacePoint* m_pCameraCoordinates = new CameraSpacePoint[512 * 424];
						hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(512 * 424, depthData, 512 * 424, m_pCameraCoordinates);
						if (SUCCEEDED(hr))
						{
							for (int i = 0; i < 512 * 424; i++)
							{
								CameraSpacePoint p = m_pCameraCoordinates[i];
								if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity() && p.Z != -std::numeric_limits<float>::infinity())
								{
									float cameraX = static_cast<float>(p.X);
									float cameraY = static_cast<float>(p.Y);
									float cameraZ = static_cast<float>(p.Z);

									cout << "x: " << cameraX << " y: " << cameraY << " z: " << cameraZ;
									// ��ʾ��
									float b = depthToRgb.data[i * 4 + 0];
									float g = depthToRgb.data[i * 4 + 1];
									float r = depthToRgb.data[i * 4 + 2];
									//fprintf(file, "%.4f %.4f %.4f %.4f %.4f %.4f\n", cameraX, cameraY, cameraZ, r, g, b);
									cout << "r: " << r << " g: " << g << " b: " << b << " i = " << i << endl;
								}
							}
						}
					}
				});
			}
		}
	});
	
	main_thread.join();
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
