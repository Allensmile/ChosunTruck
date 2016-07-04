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



class LineFinder {
private:
	cv::Mat image; // �� ����
	std::vector<cv::Vec4i> lines; // ���� �����ϱ� ���� ������ ���� ������ ����
	double deltaRho;
	double deltaTheta; // ����� �ػ� �Ķ����
	int minVote; // ���� ����ϱ� ���� �޾ƾ� �ϴ� �ּ� ��ǥ ����
	double minLength; // ���� ���� �ּ� ����
	double maxGap; // ���� ���� �ִ� ��� ����

public:
	LineFinder() : deltaRho(1), deltaTheta(PI / 180), minVote(10), minLength(0.), maxGap(0.) {}
	// �⺻ ���� �ػ󵵴� 1���� 1ȭ�� 
	// ������ ���� �ּ� ���̵� ����

	// �ش� ���� �޼ҵ��

	// �����⿡ �ػ� ����
	void setAccResolution(double dRho, double dTheta) {
		deltaRho = dRho;
		deltaTheta = dTheta;
	}

	// ��ǥ �ּ� ���� ����
	void setMinVote(int minv) {
		minVote = minv;
	}

	// �� ���̿� ���� ����
	void setLineLengthAndGap(double length, double gap) {
		minLength = length;
		maxGap = gap;
	}

	// ���� �� ���׸�Ʈ ������ �����ϴ� �޼ҵ�
	// Ȯ���� ���� ��ȯ ����
	std::vector<cv::Vec4i> findLines(cv::Mat& binary) {
		lines.clear();
		cv::HoughLinesP(binary, lines, deltaRho, deltaTheta, minVote, 200, maxGap);
		return lines;
	} // cv::Vec4i ���͸� ��ȯ�ϰ�, ������ �� ���׸�Ʈ�� ���۰� ������ �� ��ǥ�� ����.

	// �� �޼ҵ忡�� ������ ���� ���� �޼ҵ带 ����ؼ� �׸�
	// ���󿡼� ������ ���� �׸���
	void drawDetectedLines(cv::Mat &image, cv::Scalar color = cv::Scalar(0, 0, 255)) {

		// �� �׸���
		std::vector<cv::Vec4i>::const_iterator it2 = lines.begin();

		while (it2 != lines.end()) {
			cv::Point pt1((*it2)[0], (*it2)[1]);
			cv::Point pt2((*it2)[2], (*it2)[3]);
			cv::line(image, pt1, pt2, color, 3);
			++it2;
		}
	}
};


Mat hwnd2mat(HWND hwnd) {

	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom / 2 + 200;// change this to whatever size you want to resize to
	srcwidth = windowsize.right;
	height = windowsize.bottom / 2 + 200; // change this to whatever size you want to resize to
	width = windowsize.right;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 340, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);

	return src;
}

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