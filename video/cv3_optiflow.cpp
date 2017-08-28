#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int MAX_POINT_COUNT = 500;

bool needToInit();

void main()
{
	char *fn = "D:\\Program Files\\opencv\\sources\\samples\\data\\768x576.avi";
	VideoCapture cap;
	Mat source, result, gray, lastGray; //gray, lastGray对应上一帧和本帧灰度图
	vector<Point2f> points[2], temp; // 对应上一帧和本帧的特征点，上一帧是给定的，本帧时预测结果
	vector<uchar> status; // 每一特征点检测状态
	vector<float> err; // 每一特征点计算误差


	cap.open(fn);
	if (!cap.isOpened())
	{
		cout << "无法打开视频源。请检查文件或摄像头" << endl;
		return;
	}

	for (;;)
	{
		cap >> source;
		if (source.empty())
			break;

		cvtColor(source, gray, COLOR_BGR2GRAY);
		// 以下是处理
		if (points[0].size()<10)// 点数太少，重新检测特征点
		{
			goodFeaturesToTrack(gray, points[0], MAX_POINT_COUNT, 0.01, 20);
		}

		if (lastGray.empty())
			gray.copyTo(lastGray);

		// 计算光流
		calcOpticalFlowPyrLK(lastGray, gray, points[0], points[1], status, err);

		// 下面删除掉误判点
		int counter = 0;
		//temp.clear();
		//temp.insert(temp.end(), points[0].begin(), point)
		for (int i = 0; i < points[1].size(); i++)
		{
			double dist = norm(points[1][i] - points[0][i]);
			if (status[i] && dist >= 2.0 && dist <= 20.0) // 合理的特征追踪点
			{
				points[0][counter] = points[0][i];
				points[1][counter++] = points[1][i];
			}

		}

		points[0].resize(counter);
		points[1].resize(counter);

		// 显示特征点和运动轨迹
		source.copyTo(result);
		for (int i = 0; i < points[1].size(); i++)
		{
			line(result, points[0][i], points[1][i], Scalar(0, 0, 0xff));
			circle(result, points[1][i], 3, Scalar(0, 0xff, 0));
		}

		swap(points[0], points[1]);
		swap(lastGray, gray);

		imshow("源图像", source);
		imshow("检测结果", result);
		// 以下检测是否终止(按下ESC终止，对应ASCII 27)
		char key = waitKey(100);
		if (key == 27)
			break;
	}

}

bool needToInit()
{
	return true;

}

