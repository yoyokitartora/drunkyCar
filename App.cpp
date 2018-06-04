#include<windows.h>				// for windows
#include<opencv2/opencv.hpp>
#include<opencv2/ximgproc.hpp>
using namespace std;
using namespace cv;

Mat SQI(Mat &src, Size ksize, bool mul = true) {
	Mat s, b;
	src.depth() == CV_8U ? src.convertTo(s, CV_32F, 1.0 / 255.0) : src.copyTo(s);
	blur(s, b, ksize);
	s = mul ? s.mul(s / b) : s / b;
	if (src.depth() == CV_8U) convertScaleAbs(s, s, 255);
	return s;
}

void demoview(Mat &frame, Mat &edgemap, Mat &bin, Mat &seg, Mat &bin_seg, vector<Point> &waypoints,string &message) {
	Mat canvas = Mat::zeros(Size(frame.cols * 6, frame.rows), CV_8UC3), swap;
	Rect roi(0, 0, frame.cols, frame.rows);
	frame.copyTo(canvas(roi));
	applyColorMap(255-edgemap, canvas(roi + Point(frame.cols, 0)), COLORMAP_BONE);
	canvas(roi + Point(frame.cols * 2, 0)).setTo(Scalar(255, 255, 255), bin);
	applyColorMap(seg, canvas(roi + Point(frame.cols * 3, 0)), COLORMAP_PARULA);
	addWeighted(canvas(roi + Point(frame.cols * 3, 0)), 0.5, frame, 0.5, 0, canvas(roi + Point(frame.cols * 4, 0)));
	swap = canvas(roi + Point(frame.cols * 4, 0));
	for (size_t i = 1; i < waypoints.size(); i++) 
		line(swap, waypoints[i - 1], waypoints[i], Scalar(255, 0, 255), 1, 16);
	applyColorMap(bin_seg, canvas(roi + Point(frame.cols * 5, 0)), COLORMAP_SUMMER);
	putText(canvas, message, Point(5, 115), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0),2);
	putText(canvas, message, Point(5, 115), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
	imshow("costmap:roadseg", canvas);
}

int main()
{
	SetProcessAffinityMask(GetCurrentProcess, 0x01);	// for windows
	VideoCapture cam(1);
	auto sed = ximgproc::createStructuredEdgeDetection("model.yml");
	Mat frame; cam >> frame;
	vector<Point> waypoints;
	TickMeter perf_meter;
	for (; waitKey(1) != 27; cam >> frame)
	{
		perf_meter.reset();
		perf_meter.start();
		resize(frame, frame, Size(160, 120),0,0, INTER_NEAREST);
		Mat fframe,edgemap;
		frame.convertTo(fframe, CV_32F, 1.0 / 255);
		sed->detectEdges(fframe, edgemap);
		convertScaleAbs(SQI(edgemap, Size(5, 1)), edgemap, 255.0);
		rectangle(edgemap, Rect(0,0, edgemap.cols, edgemap.rows), Scalar(20), 11);
		Mat bin;
		threshold(edgemap, bin, 5, 255, THRESH_BINARY_INV);
		int w = bin.cols / 100.0, x = bin.cols / 2 - w, y = bin.rows*0.75, h = bin.rows - y;
		Rect search_roi = Rect(waypoints.empty() ? x : waypoints[waypoints.size() / 2].x, y, w, h);
		rectangle(bin, search_roi, Scalar(255), -1);
		vector<vector<Point>> contours;
		findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		bin.setTo(0);
		for (size_t i = 0; i < contours.size(); i++)
			if (!(boundingRect(contours[i])&search_roi).empty()) 
				drawContours(bin, contours, i, Scalar(255), -1);
		findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		if (contours.empty()) continue;
		vector<vector<Point>> road_hull(1);
		for (size_t i = 0; i < contours.size(); i++)
			convexHull(contours[i], road_hull[0]);
		drawContours(bin, road_hull, 0, Scalar(255), -1);
		Mat seg,bin_seg;
		ximgproc::guidedFilter(frame, bin, seg, 5, 1);
		threshold(seg, bin_seg, 127, 255, THRESH_BINARY);
		waypoints.clear();
		for (size_t i = bin_seg.rows *0.1; i < bin_seg.rows*0.9; i++){
			auto rect = boundingRect(bin_seg.row(i));
			if (rect.empty())continue;
			waypoints.push_back(Point(rect.x + rect.width / 2, i));
		}
		perf_meter.stop();
		demoview(frame, edgemap, bin, seg, bin_seg, waypoints, string("pipeline: ") + to_string((int)perf_meter.getTimeMilli()) + string(" ms"));
	}
} 