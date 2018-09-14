#include "stdafx.h"
#include "kinect.h"
#include <iostream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <Eigen/Dense>
#include "Camera_Intrinsic_Parameters.h"
#include "AxPairwiseRegistration.h"

//using Eigen::MatrixXd;
using namespace Eigen;
using namespace Eigen::internal;
using namespace Eigen::Architecture;

using namespace cv;
using namespace std;

const double camera_factor = 1000;
const double camera_cx = 263.73696;
const double camera_cy = 201.72450;
const double camera_fx = 379.40726;
const double camera_fy = 378.54472;

int main()
{
	//����ڲ�
	Eigen::Matrix3d intricRGB;
	intricRGB << 1094.75283, 0, 942.00992,
				 0, 1087.37528, 530.35240,
				 0, 0, 1;
	Eigen::Matrix3d intricDepth;
	intricDepth << camera_fx, 0, camera_cx,
				   0, camera_fy, camera_cy,
		           0, 0, 1;
	Camera_Intrinsic_Parameters intricParams(263.73696, 201.72450, 379.40726, 378.54472, 1000);

	//�������֮��Ĺ�ϵ��Ŀǰȱ��һ������任��
	Eigen::Matrix3d intricDepth2RGB;
	intricDepth2RGB = intricRGB*intricDepth.inverse();

	Eigen::Matrix3d intricRGB2Depth;
	intricRGB2Depth = intricDepth*intricRGB.inverse();
	// ��ȡKinect�豸
	IKinectSensor* m_pKinectSensor;
	HRESULT hr;
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	IMultiSourceFrameReader* m_pMultiFrameReader=NULL;
	if (m_pKinectSensor)
	{
		hr = m_pKinectSensor->Open();
		if (SUCCEEDED(hr))
		{
			// ��ȡ������Դ����ȡ��  
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Color |
				//FrameSourceTypes::FrameSourceTypes_Infrared |
				FrameSourceTypes::FrameSourceTypes_Depth,
				&m_pMultiFrameReader);
		}
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		return E_FAIL;
	}
	// ��������֡������
	IDepthFrameReference* m_pDepthFrameReference = NULL;
	IColorFrameReference* m_pColorFrameReference = NULL;
	//IInfraredFrameReference* m_pInfraredFrameReference = NULL;
	//IInfraredFrame* m_pInfraredFrame = NULL;
	IDepthFrame* m_pDepthFrame = NULL;
	IColorFrame* m_pColorFrame = NULL;
	IMultiSourceFrame* m_pMultiFrame = nullptr;
	// ����ͼƬ��ʽ
	Mat i_rgb(1080, 1920, CV_8UC4);      //ע�⣺�������Ϊ4ͨ����ͼ��Kinect������ֻ����Bgra��ʽ����
	UINT16 *depthData = new UINT16[424 * 512];

	cv::Mat last_depth;
	cv::Mat last_calibrate_rgb;
	Mat current_depth(424, 512, CV_16UC1);
	Mat current_calibrate_rgb(424, 512, CV_8UC3);
	int idxFrame=0;
	Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
	while (true)
	{
		// ��ȡ�µ�һ����Դ����֡
		hr = m_pMultiFrameReader->AcquireLatestFrame(&m_pMultiFrame);
		if (FAILED(hr) || !m_pMultiFrame)
		{
			continue;
		}
		// �Ӷ�Դ����֡�з������ɫ���ݣ�������ݺͺ�������
		if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_ColorFrameReference(&m_pColorFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pColorFrameReference->AcquireFrame(&m_pColorFrame);
		if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_DepthFrameReference(&m_pDepthFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pDepthFrameReference->AcquireFrame(&m_pDepthFrame);
		/*if (SUCCEEDED(hr))
			hr = m_pMultiFrame->get_InfraredFrameReference(&m_pInfraredFrameReference);
		if (SUCCEEDED(hr))
			hr = m_pInfraredFrameReference->AcquireFrame(&m_pInfraredFrame);*/

		// color������ͼƬ��
		UINT nColorBufferSize = 1920 * 1080 * 4;
		if (SUCCEEDED(hr))
			hr = m_pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(i_rgb.data), ColorImageFormat::ColorImageFormat_Bgra);

		// depth������ͼƬ��
		if (SUCCEEDED(hr))
		{
			hr = m_pDepthFrame->CopyFrameDataToArray(424 * 512, depthData);
			for (int i = 0; i < 512 * 424; i++)
			{
				// 0-255���ͼ��Ϊ����ʾ���ԣ�ֻȡ������ݵĵ�8λ
				BYTE intensity = static_cast<BYTE>(depthData[i]);
				reinterpret_cast<BYTE*>(current_depth.data)[i] = intensity;
			}
		}

		int icor = 0;
		for (int row = 0; row < 1080; row++)
		{
			for (int col = 0; col < 1920; col++)
			{
				/*UINT16* p = (UINT16*)i_rgb.data;
				UINT16 r = static_cast<UINT16>(p[4*(row * 1920 + col)]);
				UINT16 g = static_cast<UINT16>(p[4 * (row * 1920 + col)+1]);
				UINT16 b = static_cast<UINT16>(p[4 * (row * 1920 + col)+2]);
				UINT16 a = static_cast<UINT16>(p[4 * (row * 1920 + col) + 3]);*/
				//cout << "depthValue       " << depthValue << endl;
				//if (depthValue != -std::numeric_limits<UINT16>::infinity() && depthValue != -std::numeric_limits<UINT16>::infinity() && depthValue != 0 && depthValue != 65535)
				{
					// ��ɫͼͶӰ�����ͼ�ϵ�����
					Eigen::Vector3d uv_color(col, row, 1.0f);
					Eigen::Vector3d uv_depth = intricRGB2Depth* uv_color;//  / 1000.f +T / 1000;

					int X = static_cast<int>(uv_depth[0] / uv_depth[2]);
					int Y = static_cast<int>(uv_depth[1] / uv_depth[2]);
					if ((X >= 0 && X < 512) && (Y >= 0 && Y < 424))
					{
						current_calibrate_rgb.data[3 * (Y * 512 + X)] = i_rgb.data[4 * icor];
						current_calibrate_rgb.data[3 * (Y * 512 + X) + 1] = i_rgb.data[4 * icor + 1];
						current_calibrate_rgb.data[3 * (Y * 512 + X) + 2] = i_rgb.data[4 * icor + 2];
					}
				}
				icor++;
			}
		}
		
		// ��ʾ
		imshow("depth", current_depth);
		imshow("mosic", current_calibrate_rgb);
		
		char key = waitKey(1);

		if (key == 'q' || key == 'Q')
			break;

		if (key == 'r' || key == 'R')
		{
			current_depth.copyTo(last_depth);
			current_calibrate_rgb.copyTo(last_calibrate_rgb);
		}
		if (idxFrame==0)
		{
			/*last_depth = current_depth;
			last_calibrate_rgb = current_calibrate_rgb;*/
			current_depth.copyTo(last_depth);
			current_calibrate_rgb.copyTo(last_calibrate_rgb);
			SYSTEMTIME st;
			GetLocalTime(&st);
			char output_file[32];
			char output_RGB[40];
			char output_depth[40];
			if (key == 's' || key == 'S') {
				sprintf_s(output_file, "%4d-%2d-%2d-%2d-%2d-%2d.txt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				sprintf_s(output_RGB, "data\\%4d-%2d-%2d-%2d-%2d-%2d-rgb.png", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				sprintf_s(output_depth, "data\\%4d-%2d-%2d-%2d-%2d-%2d-depth.png", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			
				imwrite(output_depth, current_depth);
				imwrite(output_RGB, current_calibrate_rgb);

				FILE *file = fopen(output_file, "w");
				int idx = 0;
				for (int row = 0; row < 424; row++)
				{
					for (int col = 0; col < 512; col++)
					{
						UINT16* p = (UINT16*)current_depth.data;
						UINT16 depthValue = static_cast<UINT16>(p[row * 512 + col]);

						if (depthValue != -std::numeric_limits<UINT16>::infinity() && depthValue != 0)
						{
							double z = double(depthValue) / camera_factor;
							double x = (col - camera_cx) * z / camera_fx;
							double y = (row - camera_cy) * z / camera_fy;

							float cameraX = -static_cast<float>(x);
							float cameraY = static_cast<float>(z);
							float cameraZ = -static_cast<float>(y);

							if (file)
							{
								int b = current_calibrate_rgb.data[idx * 3 + 0];
								int g = current_calibrate_rgb.data[idx * 3 + 1];
								int r = current_calibrate_rgb.data[idx * 3 + 2];
								fprintf(file, "%.4f %.4f %.4f %d %d %d\n", cameraX, cameraY, cameraZ, r, g, b);
							}
						}
						idx++;
					}
				}
				fclose(file);
				cout << "xyzrgb�ļ�����ɹ�" << endl;
			}
		}
		else
		{
			AxPairwiseRegistration reg;
			reg.setTargetRGB(last_calibrate_rgb);
			reg.setSourceRGB(current_calibrate_rgb);
			reg.setTargetDepth(last_depth);
			reg.setSourceDepth(current_depth);
			reg.setDepthIntrinsicParams(intricParams);
			int flag=reg.PnPMatch();
			Eigen::Matrix4d finalTransformation = reg.getTransformation();
			cout << "RelateTransformation:" << endl;
			cout << finalTransformation << endl;
			
			if (flag == NOT_MATCH)
			{
				idxFrame++;

				// �ͷ���Դ
				SafeRelease(m_pColorFrame);
				SafeRelease(m_pDepthFrame);
				//SafeRelease(m_pInfraredFrame);
				SafeRelease(m_pColorFrameReference);
				SafeRelease(m_pDepthFrameReference);
				//SafeRelease(m_pInfraredFrameReference);
				SafeRelease(m_pMultiFrame);
				continue;
			} 
			else
			{
				double dist = reg.getAbsMotionDistance();
				if (dist<0.1)
				{
					idxFrame++;

					// �ͷ���Դ
					SafeRelease(m_pColorFrame);
					SafeRelease(m_pDepthFrame);
					//SafeRelease(m_pInfraredFrame);
					SafeRelease(m_pColorFrameReference);
					SafeRelease(m_pDepthFrameReference);
					//SafeRelease(m_pInfraredFrameReference);
					SafeRelease(m_pMultiFrame);
					continue;
				}
			}
			//���λ��
			pose = pose * finalTransformation.inverse();
			cout << "Pose:" << endl;
			cout << pose << endl;
			SYSTEMTIME st;
			GetLocalTime(&st);
			char output_file[32];
			if (key == 's' || key == 'S') {
				sprintf_s(output_file, "data\\%4d-%2d-%2d-%2d-%2d-%2d.txt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				FILE *file = fopen(output_file, "w");
				int idx = 0;
				for (int row = 0; row < 424; row++)
				{
					for (int col = 0; col < 512; col++)
					{
						UINT16* p = (UINT16*)current_depth.data;
						UINT16 depthValue = static_cast<UINT16>(p[row * 512 + col]);

						if (depthValue != -std::numeric_limits<UINT16>::infinity() && depthValue != 0)
						{
							double z = double(depthValue) / camera_factor;
							double x = (col - camera_cx) * z / camera_fx;
							double y = (row - camera_cy) * z / camera_fy;
							cv::Point3f p3(x, y, z);
							cv::Point3f pout;
							reg.transformPointcloud(p3, pout, pose);
							float cameraX = -static_cast<float>(pout.x);
							float cameraY = static_cast<float>(pout.z);
							float cameraZ = -static_cast<float>(pout.y);

							if (file)
							{
								int b = current_calibrate_rgb.data[idx * 3 + 0];
								int g = current_calibrate_rgb.data[idx * 3 + 1];
								int r = current_calibrate_rgb.data[idx * 3 + 2];
								fprintf(file, "%.4f %.4f %.4f %d %d %d\n", cameraX, cameraY, cameraZ, r, g, b);
							}
						}
						idx++;
					}
				}
				fclose(file);
				cout << "pose�ļ�����ɹ�" << endl;
			}
			/*last_depth = current_depth;
			last_calibrate_rgb = current_calibrate_rgb;*/
			current_depth.copyTo(last_depth);
			current_calibrate_rgb.copyTo(last_calibrate_rgb);
		}
		idxFrame++;
		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		//SafeRelease(m_pInfraredFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		//SafeRelease(m_pInfraredFrameReference);
		SafeRelease(m_pMultiFrame);
	}

	// �رմ��ڣ��豸
	cv::destroyAllWindows();
	m_pKinectSensor->Close();
	return 0;
}

