#include "custom.h"

using namespace std;
using namespace cv;


void detectKeyboard(Mat& sorce, Mat& destnation, Rect& rect) {

	int LowH = 0;
	int HighH = 120;

	int LowS = 0;
	int HighS = 85;

	int LowV = 170;
	int HighV = 255;
	Mat img_input, img_hsv, img_binary;



	img_input = sorce;


	//HSV로 변환
	cvtColor(img_input, img_hsv, COLOR_BGR2HSV);

	//지정한 HSV 범위를 이용하여 영상을 이진화
	inRange(img_hsv, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), img_binary);

	//morphological opening 작은 점들을 제거 
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)), Point(-1, -1), 2);
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)), Point(-1, -1), 2);

	//morphological closing 영역의 구멍 메우기 
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)), Point(-1, -1), 2);
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)), Point(-1, -1), 2);


	//라벨링 
	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(img_binary, img_labels,
		stats, centroids, 8, CV_32S);


	//영역박스 그리기
	int max = -1, idx = 0;
	for (int j = 1; j < numOfLables; j++) {
		int area = stats.at<int>(j, CC_STAT_AREA);
		if (max < area)
		{
			max = area;
			idx = j;

		}
	}


	int left = stats.at<int>(idx, CC_STAT_LEFT);
	int top = stats.at<int>(idx, CC_STAT_TOP);
	int width = stats.at<int>(idx, CC_STAT_WIDTH);
	int height = stats.at<int>(idx, CC_STAT_HEIGHT);


	//rectangle(img_input, Point(left, top), Point(left + width, top + height), Scalar(0, 0, 255), 3);
	//rectangle(img_binary, Point(left, top), Point(left + width, top + height), Scalar(255, 255, 255), 1);

	Rect roiRect(Point2i(left, top), Point2i(left + width, top + height));
	Mat realRoi = img_input(roiRect);
	rect = roiRect;
	destnation = realRoi.clone();


}

void distortionRotation(Mat& sorce, Mat& destnation, Rect& rect) {

	Mat img_input;	
	
	img_input = sorce;
	
	cvtColor(img_input, img_input, COLOR_BGR2GRAY);
	Mat roi = img_input(rect);	
	
	
	Point2i roiLT(roi.cols / 2, roi.rows / 2), roiLB(roi.cols / 2, roi.rows / 2), roiRT(roi.cols / 2, roi.rows / 2), roiRB(roi.cols / 2, roi.rows / 2);

	Point2i temp;
	float distance1 = INT_MAX, distance2 = INT_MAX, distance3 = INT_MAX, distance4 = INT_MAX;

	for (int i = 0; i < roi.rows; i++) {
		for (int j = 0; j < roi.cols; j++) {
			if (roi.at<uchar>(i, j) != 0) {
				temp = Point2i(j, i);
				if (sqrt(pow(temp.x - 0, 2) + pow(temp.y - 0, 2)) <= distance1) {
					distance1 = sqrt(pow(temp.x - 0, 2) + pow(temp.y - 0, 2));
					roiLT = temp;
				}
				if (sqrt(pow(temp.x - roi.cols, 2) + pow(0 - temp.y, 2)) <= distance3) {
					distance3 = sqrt(pow(temp.x - roi.cols, 2) + pow(temp.y - 0, 2));
					roiRT = temp;
				}
				if (sqrt(pow(temp.x - 0, 2) + pow(temp.y - roi.rows, 2)) <= distance2) {
					distance2 = sqrt(pow(temp.x - 0, 2) + pow(temp.y - roi.rows, 2));
					roiLB = temp;
				}
				
				if (sqrt(pow(temp.x - roi.cols, 2) + pow(temp.y - roi.rows, 2)) <= distance4) {
					distance4 = sqrt(pow(temp.x - roi.cols, 2) + pow(temp.y - roi.rows, 2));
					roiRB = temp;
				}
			}
		}
	}
	
	cvtColor(roi, roi, CV_GRAY2BGR);

	//원본영상좌표 좌상,우상,좌하,우하
	Point2f pts1[4] = {
		roiLT,roiRT,roiLB,roiRB
	};


	//목적영상좌표 좌상,우상,좌하,우하
	Point2f pts2[4] = {
		Point2f(0,0),Point2f(roi.cols,0),Point2f(0,roi.rows),Point2f(roi.cols,roi.rows)
	};
	for (int i = 0; i < 4; i++) {
		cout << "원본영상좌표 : " << pts1[i] << endl;
	}

	Mat dst(roi.size(), CV_8UC1);

	
	Mat perspect_map = getPerspectiveTransform(pts1, pts2);
	warpPerspective(roi, dst, perspect_map, roi.size(), INTER_CUBIC);
	
	destnation = dst.clone();

}