#include <iostream>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>


using namespace cv;
using namespace std;
Rect selectRect;
vector<Rect> rects(20);
bool clickFlag = false;
Point start;
Mat frame;
int postiveCount = 0;

void onMouse(int event,int x,int y,int,void*)
{
    if(event == CV_EVENT_LBUTTONDOWN) {
        clickFlag = true;
        start.x = x;
        start.y = y;
    }
    else if(event == CV_EVENT_LBUTTONUP) {
        clickFlag = false;
        if (selectRect.width > 10 && selectRect.height > 10) {
            rects.push_back(selectRect);
            selectRect.width = 0;
            selectRect.height = 0;
        }
    }
    if (clickFlag) {
        if (x < start.x) {
            selectRect.x = x;
            selectRect.width = start.x - x;
        } else {
            selectRect.x = start.x;
            selectRect.width = x - start.x;
        }
        if (y < start.y) {
            selectRect.y = y;
            selectRect.height = start.y - y;
        } else {
            selectRect.y = start.y;
            selectRect.height = y - start.y;
        }
    }
}

void createSamples(Mat& mat,vector<Rect>& rect);

int main()
{
    char c;
    Mat imageRead;
    Mat imageShow;

    namedWindow("image",1);
    setMouseCallback("image",onMouse,0);
    imageRead = imread("data/0000000000.png");

    while (1) {

        imageShow = imageRead.clone();
        for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
            rectangle(imageShow,*iter,Scalar(255,0,0));
        }
        rectangle(imageShow,selectRect,Scalar(255,0,0));
        imshow("image",imageShow);

        if(imageShow.empty()){
            break;
        }
        c = waitKey(33);
        if (c == 27) { //ESC
            break;
        } else if (c == 'a' || c == 'A' || c == 81) { //a or left arrow
            imageRead = imread("data/0000000010.png");
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'd' || c == 'D' || c == 83) { //d or right arrow
            imageRead = imread("data/0000000020.png");
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'c' || c == 'C') {
            rects.pop_back();
            selectRect = Rect(0,0,0,0);
            imageShow = imageRead.clone();
            for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
                rectangle(imageShow,*iter,Scalar(255,0,0));
            }
            imshow("image",imageShow);
        } else if (c == 's' || c == 'S') { //save
            createSamples(imageRead,rects);
        }
    }

    destroyWindow("image");

    return 0;
}

void createSamples(Mat& mat,vector<Rect>& rect)
{
    Mat grayImage;
    Mat cut;
    cvtColor(mat, grayImage, CV_BGR2GRAY);
    for (vector<Rect>::iterator iter = rect.begin(); iter != rect.end(); ++iter) {
        cut = Mat(grayImage,
                  Range((*iter).y,(*iter).y+(*iter).height),//row range
                  Range((*iter).x,(*iter).x+(*iter).width));//col range
        if (cut.rows > 10 && cut.cols > 10) {
            imwrite("pos/pos.png", cut);
        }
    }
}
