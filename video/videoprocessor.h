#if !defined VPROCESSOR
#define VPROCESSOR

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class FrameProcessor {

public:
	virtual void process(cv::Mat &input, cv::Mat &output) = 0;
};

class VideoProcessor {

private:

	VideoCapture capture;  //视频捕获对象

	void(*process)(cv::Mat&, cv::Mat&);  //处理每一帧时都会调用的回调函数

	FrameProcessor *frameProcessor;  //一个FrameProcessor实例

	bool callIt;  //布尔型变量，表示回调函数是否会被调用
	
	string windowNameInput;  //输入窗口的显示名称
	
	string windowNameOutput;  //输出窗口的显示名称
	
	int delay;  //帧之间的延迟时间
	
	long fnumber;  //已经处理的帧数
	
	long frameToStop;  //到达这个帧时结束
	
	bool stop;  //结束处理的标志

	vector<string> images;  //作为输入对象的图像文件名向量
	
	vector<string>::const_iterator itImg;  //图像向量的迭代器

	VideoWriter writer;  //opencv的写视频对象
	
	string outputFile;  //输出文件名
	
	int currentIndex;  //输出图像的当前序号
	
	int digits;  //输出图像名中数字的位数
	
	string extension;  //输出图像的扩展名

	// to get the next frame 
	// could be: video file; camera; vector of images
	bool readNextFrame(Mat& frame) 
	{
		if (images.size() == 0)
			return capture.read(frame);
		else 
		{
			if (itImg != images.end()) 
			{
				frame = imread(*itImg);
				itImg++;
				return frame.data != 0;
			}
			return false;
		}
	}

	// to write the output frame 
	// could be: video file or images
	void writeNextFrame(Mat& frame) 
	{
		if (extension.length()) 
		{ 
			stringstream ss;
			ss << outputFile << setfill('0') << setw(digits) << currentIndex++ << extension;
			imwrite(ss.str(), frame);  //写入到图像组
		}
		else 
		{ 
			writer.write(frame);  //写入视频文件
		}
	}

public:

	//构造函数，设置初始属性值
	VideoProcessor() : callIt(false), delay(-1),
		fnumber(0), stop(false), digits(0), frameToStop(-1),
		process(0), frameProcessor(0) {}

	//设置视频文件的名称
	bool setInput(string filename) 
	{
		fnumber = 0;
		
		capture.release();  //防止已有资源与VideoCapture实例关联
		images.clear();

		return capture.open(filename);  //打开视频文件
	}

	//设置相机ID
	bool setInput(int id) 
	{
		fnumber = 0;
	
		capture.release();  //防止已有资源与VideoCapture实例关联
		images.clear();

		return capture.open(id);  //打开视频文件
	}

	//设置输入图像的向量
	bool setInput(const vector<string>& imgs) {

		fnumber = 0;
		
		capture.release();  //防止已有资源与VideoCapture实例关联

		images = imgs;  //将这个图像向量作为输入对象
		itImg = images.begin();

		return true;
	}

	//设置输出视频文件
	//默认情况下会使用与输入视频相同的参数
	bool setOutput(const string &filename, int codec = 0, double framerate = 0.0, bool isColor = true) 
	{
		outputFile = filename;
		extension.clear();

		if (framerate == 0.0)
			framerate = getFrameRate(); //与输入相同

		char c[4];	
		if (codec == 0) 
			codec = getCodec(c);  //使用与输入相同的编解码器
		

		//打开输出视频
		return writer.open(outputFile, //文件名
			codec,  //编解码器
			framerate,  //视频的帧速率
			getFrameSize(), //帧的尺寸
			isColor);  //是否是彩色视频
	}

	//设置输出为一系列图像文件
	//扩展名必须是 ".jpg", ".bmp" ...
	bool setOutput(const string &filename,  //前缀
		const string &ext,  //图像文件的扩展名
		int numberOfDigits = 3,  //数字的位数
		int startIndex = 0)  //开始序号
	{		
		if (numberOfDigits<0)  //数字的位数必须是正数
			return false;

		outputFile = filename;  //文件名
		extension = ext;  //扩展名
		
		digits = numberOfDigits;  //文件编号方案中数字的位数
		
		currentIndex = startIndex;  //从这个序号开始编号

		return true;
	}

	// set the callback function that will be called for each frame
	void setFrameProcessor(void(*frameProcessingCallback)(Mat&, Mat&)) {

		// invalidate frame processor class instance
		frameProcessor = 0;
		// this is the frame processor function that will be called
		process = frameProcessingCallback;
		callProcess();
	}

	// set the instance of the class that implements the FrameProcessor interface
	void setFrameProcessor(FrameProcessor* frameProcessorPtr) {

		// invalidate callback function
		process = 0;
		// this is the frame processor instance that will be called
		frameProcessor = frameProcessorPtr;
		callProcess();
	}

	// stop streaming at this frame number
	void stopAtFrameNo(long frame) {

		frameToStop = frame;
	}

	// process callback to be called
	void callProcess() {

		callIt = true;
	}

	// do not call process callback
	void dontCallProcess() {

		callIt = false;
	}

	//用于显示输入的帧
	void displayInput(string wn) {

		windowNameInput = wn;
		namedWindow(windowNameInput);
	}

	//用于显示处理过的帧
	void displayOutput(string wn) {

		windowNameOutput = wn;
		namedWindow(windowNameOutput);
	}

	// do not display the processed frames
	void dontDisplay() {

		cv::destroyWindow(windowNameInput);
		cv::destroyWindow(windowNameOutput);
		windowNameInput.clear();
		windowNameOutput.clear();
	}

	// set a delay between each frame
	// 0 means wait at each frame
	// negative means no delay
	void setDelay(int d) {

		delay = d;
	}

	// a count is kept of the processed frames
	long getNumberOfProcessedFrames() {

		return fnumber;
	}

	// return the size of the video frame
	cv::Size getFrameSize() {

		if (images.size() == 0) {

			// get size of from the capture device
			int w = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
			int h = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));

			return cv::Size(w, h);

		}
		else { // if input is vector of images

			cv::Mat tmp = cv::imread(images[0]);
			if (!tmp.data) return cv::Size(0, 0);
			else return tmp.size();
		}
	}

	// return the frame number of the next frame
	long getFrameNumber() {

		if (images.size() == 0) {

			// get info of from the capture device
			long f = static_cast<long>(capture.get(CV_CAP_PROP_POS_FRAMES));
			return f;

		}
		else { // if input is vector of images

			return static_cast<long>(itImg - images.begin());
		}
	}

	// return the position in ms
	double getPositionMS() {

		// undefined for vector of images
		if (images.size() != 0) return 0.0;

		double t = capture.get(CV_CAP_PROP_POS_MSEC);
		return t;
	}

	// return the frame rate
	double getFrameRate() {

		// undefined for vector of images
		if (images.size() != 0) return 0;

		double r = capture.get(CV_CAP_PROP_FPS);
		return r;
	}

	// return the number of frames in video
	long getTotalFrameCount() {

		// for vector of images
		if (images.size() != 0) return images.size();

		long t = capture.get(CV_CAP_PROP_FRAME_COUNT);
		return t;
	}

	// get the codec of input video
	int getCodec(char codec[4]) {

		// undefined for vector of images
		if (images.size() != 0) return -1;

		union {
			int value;
			char code[4];
		} returned;

		returned.value = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));

		codec[0] = returned.code[0];
		codec[1] = returned.code[1];
		codec[2] = returned.code[2];
		codec[3] = returned.code[3];

		return returned.value;
	}

	// go to this frame number
	bool setFrameNumber(long pos) {

		// for vector of images
		if (images.size() != 0) {

			// move to position in vector
			itImg = images.begin() + pos;
			// is it a valid position?
			if (pos < images.size())
				return true;
			else
				return false;

		}
		else { // if input is a capture device

			return capture.set(CV_CAP_PROP_POS_FRAMES, pos);
		}
	}

	// go to this position
	bool setPositionMS(double pos) {

		// not defined in vector of images
		if (images.size() != 0)
			return false;
		else
			return capture.set(CV_CAP_PROP_POS_MSEC, pos);
	}

	// go to this position expressed in fraction of total film length
	bool setRelativePosition(double pos) {

		// for vector of images
		if (images.size() != 0) {

			// move to position in vector
			long posI = static_cast<long>(pos*images.size() + 0.5);
			itImg = images.begin() + posI;
			// is it a valid position?
			if (posI < images.size())
				return true;
			else
				return false;

		}
		else { // if input is a capture device

			return capture.set(CV_CAP_PROP_POS_AVI_RATIO, pos);
		}
	}

	// Stop the processing
	void stopIt() {

		stop = true;
	}

	// Is the process stopped?
	bool isStopped() {

		return stop;
	}

	// Is a capture device opened?
	bool isOpened() {

		return capture.isOpened() || !images.empty();
	}

	// to grab (and process) the frames of the sequence
	void run() 
	{
		Mat frame;  //当前帧	
		Mat output;  //输出帧

		if (!isOpened())
			return;

		stop = false;

		while (!isStopped()) 
		{
			if (!readNextFrame(frame))
				break;

			//显示输入的帧
			if (windowNameInput.length() != 0)
				imshow(windowNameInput, frame);

			//调用处理函数
			if (callIt) 
			{
				//处理帧
				if (process)
					process(frame, output);
				else if (frameProcessor)
					frameProcessor->process(frame, output);
	
				fnumber++;  //递增帧数
			}
			else 
			{
				output = frame;
			}

			//写入到输出的序列
			if (outputFile.length() != 0)
				writeNextFrame(output);

			//显示输出的序列
			if (windowNameOutput.length() != 0)
				imshow(windowNameOutput, output);

			//产生延时
			if (delay >= 0 && cv::waitKey(delay) >= 0)
				stopIt();

			//检查是否需要结束
			if (frameToStop >= 0 && getFrameNumber() == frameToStop)
				stopIt();
		}
	}
};

#endif


