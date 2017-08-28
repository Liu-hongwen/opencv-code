#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int labelTargets(Mat &src, Mat &mask, int thresh = 100);

int main()
{
	char *fn = "D:\\Program Files\\opencv\\sources\\samples\\data\\768x576.avi";
	VideoCapture cap;
	Mat source, image, foreGround, backGround, fgMask;
	Ptr<BackgroundSubtractor> pBgModel =
		createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();

	cap.open(fn);
	if (!cap.isOpened())
		cout << "无法打开视频文件： " << fn << endl;

	for (;;)
	{
		cap >> source;
		if (source.empty())
			break;

		//resize(source, image, Size(source.cols / 2, source.rows / 2), INTER_LINEAR);

		resize(source, image, Size(source.cols, source.rows), INTER_LINEAR);

		if (foreGround.empty())
			foreGround.create(image.size(), image.type());

		pBgModel->apply(image, fgMask);

		//GaussianBlur(fgMask, fgMask, Size(5, 5), 0);
		threshold(fgMask, fgMask, 30, 255, THRESH_BINARY);

		foreGround = Scalar::all(0);
		image.copyTo(foreGround, fgMask);
		// 标记找到的运动目标
		int nTargets = labelTargets(image, fgMask);
		cout << "共检测到 " << nTargets << " 个目标" << endl;

		pBgModel->getBackgroundImage(backGround);

		// 显示原始图像及背景，前景
		imshow("Source", image);
		imshow("Background", backGround);
		imshow("Foreground", foreGround);
		imshow("Foreground Mask", fgMask);


		// 以下检测是否终止(按下ESC终止，对应ASCII 27)
		char key = waitKey(100); // 每一帧等待100ms
		if (key == 27)
			break;
	}

	waitKey(0);
}

int labelTargets(Mat &src, Mat &mask, int thresh)
{
	// 以下是图像分割
	Mat seg = mask.clone();
	vector<vector<Point> > cnts;
	findContours(seg, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);


	// 以下进行筛选
	float area;
	Rect rect;
	int count = 0;
	string strCount;
	for (int i = cnts.size() - 1; i >= 0; i--)
	{
		vector<Point> c = cnts[i];
		area = contourArea(c);
		if (area < thresh) // 滤除面积小于10的分割结果：可能是噪声
			continue;

		count++; 
		//cout << "blob " << i << " : " << area << endl;
		rect = boundingRect(c);
		// 在原始图像上画出包围矩形，并给每个矩形标号
		rectangle(src, rect, Scalar(0, 0, 0xff), 1);

		stringstream ss;
		ss << count;
		ss >> strCount;
		putText(src, strCount, Point(rect.x, rect.y), CV_FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 0xff, 0));
	}

	return count;
}


