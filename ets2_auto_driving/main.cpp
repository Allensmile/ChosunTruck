#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/imageproc/imageproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/photo/cuda.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/cudaimgproc.hpp>
//#include <opencv2/cudafilters.hpp>
//#include <opencv2/gpu/gpu.hpp>
#include <Windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include "ets2_self_driving.h"

#define PI 3.1415926

using namespace cv;
using namespace std;

int main() {

	//cudaf();

	
	long long int sum = 0;
	long long int i = 0;
	while (true) {
	auto begin = chrono::high_resolution_clock::now();
	// ETS2
	HWND hWnd = FindWindow("prism3d", NULL);
	// NOTEPAD
	//HWND hWnd = FindWindow("Notepad", NULL);
	Mat image = hwnd2mat(hWnd);
	
	// Mat to GpuMat
	//cuda::GpuMat imageGPU;
	//imageGPU.upload(image);

	medianBlur((Mat)image, (Mat)image, 3);
	//cuda::bilateralFilter(imageGPU, imageGPU, );

	cv::Mat contours;
	cv::Canny((Mat)image, contours, 125, 350);
	LineFinder ld; // �ν��Ͻ� ����

	// Ȯ���� ������ȯ �Ķ���� �����ϱ�
	ld.setLineLengthAndGap(100, 30);
	ld.setMinVote(50);

	std::vector<cv::Vec4i> li = ld.findLines(contours);
	ld.drawDetectedLines((Mat)image);

	imshow("Test", (Mat)image);
	waitKey(1);
	auto end = chrono::high_resolution_clock::now();
	auto dur = end - begin;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	ms++;
	sum += ms;
	cout << 1000 / ms << "fps       avr:" << 1000 / (sum / (++i)) << endl;
	}	

	return 0;
}