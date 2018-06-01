#include<opencv2/opencv.hpp>
#include<opencv2/ximgproc.hpp>
using namespace std;
using namespace cv;

Mat SQI(Mat &src, Scalar param, int code = 'box', bool mul = true) {
	Mat b, s;
	src.copyTo(s);
	switch (code)
	{
	case'box':
		blur(src, b, Size(param[0], param[1]));
		break;
	case'guid':
		ximgproc::guidedFilter(src, src, b, param[0], param[1]);
		break;
	default:
		src.copyTo(b);
		break;
	}
	return mul ? s.mul(s / b) : s / b;
}
nt main()
{
	string path = "C:/Users/biche/OneDrive/ͼƬ/1/";
	vector<Mat> frames;
	auto sed = ximgproc::createStructuredEdgeDetection("model.yml");
	for (size_t i = 1; waitKey(1) != 27; i++) 
	{
		Mat frame = imread(path + to_string(i) + "_cam-image_array_.jpg");
		if (frame.empty()) { i = 1; continue; }
		devView(frame);
		Mat fframe,edgemap;
		frame.convertTo(fframe, CV_32F, 1.0 / 255);
		sed->detectEdges(fframe, edgemap);
		convertScaleAbs(SQI(edgemap, Scalar(5, 1), 'box'), edgemap, 255.0);
		rectangle(edgemap, Rect(0,0, edgemap.cols, edgemap.rows), Scalar(127), 11);
		devView(edgemap);
		Mat bin;
		threshold(edgemap, bin, 15, 255, THRESH_BINARY_INV);
		int w = bin.cols / 100.0, x = bin.cols / 2 - w, y = bin.rows*0.55, h = bin.rows - y;
		Rect search_roi = Rect(x, y, w, h);
		rectangle(bin, search_roi, Scalar(255), -1);
		vector<vector<Point>> contours;
		findContours(bin, contours, RETR_EXTERNAL, RETR_LIST);
		bin.setTo(0);
		for (size_t i = 0; i < contours.size(); i++){
			auto rect = boundingRect(contours[i]);
			if (rect.contains(Point(edgemap.cols / 2, edgemap.rows*0.8))|| rect.contains(Point(edgemap.cols / 2, edgemap.rows*0.6)))
				drawContours(bin, contours, i, Scalar(255), -1);
		}
		findContours(bin, contours, RETR_EXTERNAL, RETR_LIST);
		if (contours.empty()) {
			cout << "can't find road at frame " << i << endl;
			continue;
		}
		bin.setTo(0);
		vector<vector<Point>> road_hull(1);
		for (size_t i = 0; i < contours.size(); i++)
			convexHull(contours[i], road_hull[0]);
		drawContours(bin, road_hull, 0, Scalar(255), -1);
		devView(bin);
		Mat mask,mask_seg;
		ximgproc::guidedFilter(frame, bin, mask, 5, 1);
		threshold(mask, mask_seg, 127, 255, THRESH_BINARY);
		devView(mask);
		vector<int> steer_q;
		for (size_t i = mask_seg.rows *0.1; i < mask_seg.rows*0.9; i++){
			auto rect = boundingRect(mask_seg.row(i));
			if (rect.empty())continue;
			circle(frame, Point(rect.x + rect.width / 2, i), 1, Scalar(255, 0, 255));
			steer_q.push_back(rect.x + rect.width / 2);
		}
		cvtColor(mask, mask, COLOR_GRAY2BGR);
		addWeighted(mask-(Scalar(127, 0, 255)), 0.5, frame, 1, 0, frame);
		imshow("", frame);
	}
} 
