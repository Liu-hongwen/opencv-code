#include <string>
#include <iostream>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "videoprocessor.h"

using namespace cv;
using namespace std;

void draw(const cv::Mat& img, cv::Mat& out) {

	img.copyTo(out);
	cv::circle(out, cv::Point(100, 100), 5, cv::Scalar(255, 0, 0), 2);
}

// processing function
void canny(cv::Mat& img, cv::Mat& out) {

	// Convert to gray
	if (img.channels() == 3)
		cv::cvtColor(img, out, CV_BGR2GRAY);
	// Compute Canny edges
	cv::Canny(out, out, 100, 200);
	// Invert the image
	cv::threshold(out, out, 128, 255, cv::THRESH_BINARY_INV);
}

int main()
{
	VideoCapture capture("D:\\Program Files\\opencv\\sources\\samples\\data\\768x576.avi");

	if (!capture.isOpened())
		cout << "无法打开视频文件" << endl;

	double rate = capture.get(CV_CAP_PROP_FPS);  //获取帧速率
	cout << "Frame rate: " << rate << "fps" << endl;

	int delay = 1000 / rate;  //根据帧速率计算帧之间的等待时间，单位ms
	cout << "Delay time: " << delay << "ms" << endl;

	bool stop = false;

	Mat frame;  //当前视频帧
	namedWindow("Extracted Frame");

	long long i = 0;
	string b = "bike";
	string ext = ".bmp";
	// for all frames in video
	while (!stop) {

		if (!capture.read(frame))
			break;

		imshow("Extracted Frame", frame);
		/*
		string name(b);
		// note: some MinGW compilers generate an error for this line
		// this is a compiler bug
		// try: std::ostringstream ss; ss << i; name+= ss.rdbuf(); i++;
		//		name+=std::to_string(i++);
		ostringstream ss; ss << i; name += ss.str(); i++;
		name += ext;

		cout << name << endl;

		imwrite(name, frame);
		*/
		if (waitKey(delay) >= 0)  //等待一段时间或者通过按键停止
			stop = true;
	}

	capture.release();  //关闭视频文件

	waitKey();

	cout << "123";

	VideoProcessor processor;

	//打开视频文件
	processor.setInput("D:\\Program Files\\opencv\\sources\\samples\\data\\768x576.avi");

	//声明显示视频的窗口
	processor.displayInput("Input Video");
	processor.displayOutput("Output Video");

	//用原始帧速率播放视频
	processor.setDelay(1000. / processor.getFrameRate());

	//设置处理帧的回调函数
	processor.setFrameProcessor(canny);

	// output a video
	processor.setOutput("D:\\Program Files\\opencv\\sources\\samples\\data\\768x576_2.avi");

	// stop the process at this frame
	processor.stopAtFrameNo(151);

	//开始处理
	processor.run();

	waitKey();

	return 0;
}


