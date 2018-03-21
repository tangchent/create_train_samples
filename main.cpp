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

/**
 * mouse push CALLBACK function
 *
 * @param event enum kind,type of mouse push.
 * @param x The x coordinate of the mouse in the window.
 * @param y The y coordinate of the mouse in the window.
 * @return
 */
void onMouse(int event,int x,int y,int,void*)
{
    if(event == CV_EVENT_LBUTTONDOWN) { //start of select
        clickFlag = true;
        start.x = x;
        start.y = y;
    }
    else if(event == CV_EVENT_LBUTTONUP) { //end of select
        clickFlag = false;
        if (selectRect.width > 10 && selectRect.height > 10) { //ensure the area is big enough
            rects.push_back(selectRect);
            selectRect.width = 0;
            selectRect.height = 0;
        }
    }
    if (clickFlag) { // show realtime select area
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

//function declarations
void createSamples(Mat& mat,vector<Rect>& rect);
void executeCMD(const char *cmd, vector<String *>& result);

int main()
{
    int index = 0;
    char c;
    Mat imageRead;
    Mat imageShow;
    vector<String *> result(1024);
    char filename[256];

    result.clear();
    executeCMD("ls data/*.png",result); //get file name

    namedWindow("image",1);
    setMouseCallback("image",onMouse,0);
    //remove '\n'
    strncpy(filename,result.at(index)->c_str(),result.at(index)->size() - 1);
    //read image
    imageRead = imread(filename);

    while (1) {
        if (imageRead.empty()) {
            printf(" --(!) No image -- Break!");
            break;
        }

        //copy for draw
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
            if (--index <  0)    index = 0;
            //remove '\n'
            strncpy(filename,result.at(index)->c_str(),result.at(index)->size() - 1);
            //read image
            imageRead = imread(filename);
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'd' || c == 'D' || c == 83) { //d or right arrow
            if (++index > (int)(result.size() - 1))    index = 0;
            //remove '\n'
            strncpy(filename,result.at(index)->c_str(),result.at(index)->size() - 1);
            //read image
            imageRead = imread(filename);
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'c' || c == 'C') { //cancel
            rects.pop_back();
            selectRect = Rect(0,0,0,0);
            imageShow = imageRead.clone();
            for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
                rectangle(imageShow,*iter,Scalar(255,0,0)); // re-draw
            }
            imshow("image",imageShow);
        } else if (c == 's' || c == 'S') { //save
            createSamples(imageRead,rects);
        }
    }

    destroyWindow("image");

    return 0;
}

/**
 * use a picture to create samples
 * @param mat The orignal picture that contains postive samples and negtive samples
 * @param rect postive samples areas that marked by yourself
 * @return
 */
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

/**
 * execute command use command line
 * only for linux platform , windows is not supported!
 * @param cmd. the command name needed to be executed
 * only support linux commands
 * @param result . the reference of command result.
 * if execute 'ls' ,the result contains all file name in current dir
 * attention that result is split by '\n',and each element contains '\n'
 * @return
 */
void executeCMD(const char *cmd, vector<String *>& result)
{
    char buf_ps[1024];
    char ps[1024];
    FILE * ptr;
    String * str = NULL;
    strcpy(ps, cmd);
    if((ptr = popen(ps, "r")) != NULL) {
        while(fgets(buf_ps, 1024, ptr) != NULL) {
            str = new String(buf_ps);
            if (str != NULL) {
                result.push_back(str);
            } else {
                break;
            }
            str = NULL;
        }
        pclose(ptr);
        ptr = NULL;
    }
    else {
        printf("popen '%s' error\n", ps);
    }
}
