#include "stdafx.h"
#include "AxPairwiseRegistration.h"


AxPairwiseRegistration::AxPairwiseRegistration()
{
	transformation = Eigen::Matrix4d::Identity();
}


AxPairwiseRegistration::~AxPairwiseRegistration()
{
}

void AxPairwiseRegistration::setSourceRGB(cv::Mat source_)
{
	source_rgb = source_;
}

void AxPairwiseRegistration::setTargetRGB(cv::Mat target_)
{
	target_rgb = target_;
}

void AxPairwiseRegistration::setSourceDepth(cv::Mat sourced_)
{
	source_depth = sourced_;
}
void AxPairwiseRegistration::setTargetDepth(cv::Mat targetd_)
{
	target_depth = targetd_;
}

void AxPairwiseRegistration::transformPointcloud(const cv::Point3f cloud_in, cv::Point3f& cloud_out,
	const Eigen::Matrix<double, 4, 4>  &transform)
{
	Eigen::Matrix<double, 3, 1> pt(cloud_in.x, cloud_in.y, cloud_in.z);
	double x = static_cast<float> (transform(0, 0) * pt.coeffRef(0) + transform(0, 1) * pt.coeffRef(1) + transform(0, 2) * pt.coeffRef(2) + transform(0, 3));
	double y = static_cast<float> (transform(1, 0) * pt.coeffRef(0) + transform(1, 1) * pt.coeffRef(1) + transform(1, 2) * pt.coeffRef(2) + transform(1, 3));
	double z = static_cast<float> (transform(2, 0) * pt.coeffRef(0) + transform(2, 1) * pt.coeffRef(1) + transform(2, 2) * pt.coeffRef(2) + transform(2, 3));
	cv::Point3f pp(x, y, z);
	cloud_out = pp;

}

int AxPairwiseRegistration::PnPMatch()
{
	// 声明特征提取器与描述子提取器
	cv::Ptr<cv::FeatureDetector> _detector;
	cv::Ptr<cv::DescriptorExtractor> _descriptor;

	// 构建提取器，默认两者都为sift
	// 构建sift, surf之前要初始化nonfree模块
	cv::initModule_nonfree();
	_detector = cv::FeatureDetector::create("GridSIFT");
	_descriptor = cv::DescriptorExtractor::create("SIFT");

	vector< cv::KeyPoint > kp1, kp2; //关键点
	_detector->detect(target_rgb, kp1);  //提取关键点
	_detector->detect(source_rgb, kp2);  //提取关键点
	cv::Mat image;
	cv::drawKeypoints(target_rgb, kp1, image, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::imshow("keypoints", image);
	//cv::imwrite("data\\keypoints.png", image);
	cv::Mat desp1, desp2;
	_descriptor->compute(target_rgb, kp1, desp1);
	_descriptor->compute(source_rgb, kp2, desp2);
	std::vector<cv::DMatch> matches;
	cv::FlannBasedMatcher matcher;
	matcher.match(desp1, desp2, matches);
	// 可视化：显示匹配的特征
	cv::Mat imgMatches;
	//cv::drawMatches(target_rgb, kp1, source_rgb, kp2, matches, imgMatches);
	//cv::imshow("matches", imgMatches);
	//cv::imwrite("data\\matches.png", imgMatches);
	// 筛选匹配，把距离太大的去掉
	// 这里使用的准则是去掉大于四倍最小距离的匹配
	vector< cv::DMatch > goodMatches;
	// 找到最小距离
	double minDis = 9999;
	for (size_t i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance < minDis)
			minDis = matches[i].distance;
	}
	int ratio = 8; //最小距离的多少倍,这里选择8
	for (size_t i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance < ratio * minDis)
			goodMatches.push_back(matches[i]);
	}

	// 显示 good matches
	cv::drawMatches(target_rgb, kp1, source_rgb, kp2, goodMatches, imgMatches);
	cv::imshow("good matches", imgMatches);
	char key = waitKey(1);
	if (key == 's' || key == 'S') {
		cv::imwrite("data\\good_matches.png", imgMatches);
		printf("good_matches 保存成功");
	}
	std::vector<cv::Point3f> pts_obj;
	vector<cv::Point2f> pts_img;
	if (goodMatches.size() <= 5)
	{
		transformation=Eigen::Matrix4d::Identity();
		return NOT_MATCH;
	}
	for (size_t i = 0; i < goodMatches.size(); i++)
	{
		cv::Point2f p = kp1[goodMatches[i].queryIdx].pt;
		// 获取d是要小心！x是向右的，y是向下的，所以y才是行，x是列！
		ushort d = target_depth.ptr<ushort>(int(p.y))[int(p.x)];
		if (d == 0)
		{
			continue;
		}
		pts_img.push_back(cv::Point2f(kp2[goodMatches[i].trainIdx].pt));
		// 将(u,v,d)转成(x,y,z)
		cv::Point3f pt(p.x, p.y, d);
		cv::Point3f pd = point2dTo3d(pt, depth_intrinsic_par);
		pts_obj.push_back(pd);
	}
	double camera_matrix_data[3][3] = {
		{ depth_intrinsic_par.camera_fx, 0, depth_intrinsic_par.camera_cx },
		{ 0, depth_intrinsic_par.camera_fy, depth_intrinsic_par.camera_cy },
		{ 0, 0, 1 } };
	cv::Mat cameraMatrix(3, 3, CV_64F, camera_matrix_data);
	cv::Mat rvec, tvec, inliers;

	if (pts_obj.size() == 0 || pts_img.size() == 0)
	{
		transformation = Eigen::Matrix4d::Identity();
		return NOT_MATCH;
	}
	cv::solvePnPRansac(pts_obj, pts_img, cameraMatrix, cv::Mat(), rvec, tvec, false, 100, 1.0, 100, inliers);

	std::vector<cv::DMatch> matchesShow;
	for (size_t i = 0; i < inliers.rows; i++)
	{
		matchesShow.push_back(goodMatches[inliers.ptr<int>(i)[0]]);
	}
	// 显示 inliers matches
	cv::drawMatches(target_rgb, kp1, source_rgb, kp2, matchesShow, imgMatches);
	cv::imshow("inlier matches", imgMatches);
	
	if (key == 's' || key == 'S') {
		cv::imwrite("data\\inliers.png", imgMatches);
		printf("inliers 保存成功");
	}
	
	//waitKey(0);
	if (inliers.rows < 5)
	{
		transformation = Eigen::Matrix4d::Identity();
		return NOT_MATCH;
	}
	distance = normOfTransform(rvec, tvec);
	Eigen::Isometry3d RT = cvMat2Eigen(rvec, tvec);
	transformation = RT.matrix();
	return MATCH;
}

void AxPairwiseRegistration::setDepthIntrinsicParams(Camera_Intrinsic_Parameters params)
{
	depth_intrinsic_par = params;
}


