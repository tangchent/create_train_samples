#include <iostream>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <stdio.h>

#define MIN_STEP            12
#define SAMPLE_WIDTH        32
#define SAMPLE_HEIGHT       32
#define MAX_SAMPLE_WIDTH    320
#define MAX_SAMPLE_HEIGHT   320
#define FACTOR              1.2f

#define POSTIVE_DIR_NAME    "pos/"
#define NEGTIVE_DIR_NAME    "neg/"

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
        if (selectRect.width > SAMPLE_WIDTH && selectRect.height > SAMPLE_HEIGHT) { //ensure the area is big enough
            selectRect.x = selectRect.x < 0 ? 0 : selectRect.x;
            selectRect.y = selectRect.y < 0 ? 0 : selectRect.y;
            rects.push_back(selectRect);
        }
        selectRect.width = 0;
        selectRect.height = 0;
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
void createSamples(Mat& mat,vector<Rect>& rects,char* filename);
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
    //add '\0'
    filename[result.at(index)->size()] = '\0';
    //read image
    imageRead = imread(filename);
    rects.clear();

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
            //add '\0'
            filename[result.at(index)->size()] = '\0';
            //read image
            imageRead = imread(filename);
            printf("filename:%s \n",filename);
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'd' || c == 'D' || c == 83) { //d or right arrow
            if (++index > (int)(result.size() - 1))    index = 0;
            //remove '\n'
            strncpy(filename,result.at(index)->c_str(),result.at(index)->size() - 1);
            //add '\0'
            filename[result.at(index)->size()] = '\0';
            //read image
            imageRead = imread(filename);
            printf("filename:%s \n",filename);
            rects.clear();
            selectRect = Rect(0,0,0,0);
        } else if (c == 'c' || c == 'C') { //cancel
            if (rects.size() > 0) {
                rects.pop_back();
            }
            selectRect = Rect(0,0,0,0);
            imageShow = imageRead.clone();
            for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
                rectangle(imageShow,*iter,Scalar(255,0,0)); // re-draw
            }
            imshow("image",imageShow);
        } else if (c == 's' || c == 'S') { //save
            createSamples(imageRead,rects,filename);
        }
    }

    destroyWindow("image");

    return 0;
}

/**
 * judge if rect is in rects area
 * @param rects.
 * @param rect.
 * @return bool true is rect in rects area
 */
inline bool isInRect(vector<Rect>& rects,Rect rect) {
    for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
        if (rect.x > (iter->x - SAMPLE_WIDTH/2) &&
                rect.y > (iter->y - SAMPLE_HEIGHT/2)&&
                rect.x < (iter->x + iter->width + SAMPLE_WIDTH/2) &&
                rect.y < (iter->y + iter->height + SAMPLE_HEIGHT/2)) {
            return true;
        }
    }
    return false;
}

/**
 * use a picture to create samples
 * @param mat. The orignal picture that contains postive samples and negtive samples
 * @param rect. postive samples areas that marked by yourself
 * @param filename. filename of mat
 * @return
 */
void createSamples(Mat& mat,vector<Rect>& rects,char* filename)
{
    char cutFilename[256];
    int count;
    int length;
    int filenameLength;
    int widthStep,heightStep;
    Mat cut;
    Mat resizeCut;
    Rect rect = Rect(0,0,SAMPLE_WIDTH,SAMPLE_HEIGHT);

    count  = 0;
    length = strlen(NEGTIVE_DIR_NAME);
    filenameLength = strlen(filename);
    strcpy(cutFilename,NEGTIVE_DIR_NAME);
    strcpy(cutFilename + length,filename);

    while (rect.width < MAX_SAMPLE_WIDTH && rect.height < MAX_SAMPLE_HEIGHT) {
        widthStep  = rect.width*0.8;
        heightStep = rect.height*0.8;
        heightStep = heightStep < MIN_STEP ? MIN_STEP : heightStep;
        widthStep = widthStep < MIN_STEP ? MIN_STEP : widthStep;

        for (int row = 0; row < mat.rows - rect.height; row += heightStep) {
            rect.y = row;
            for (int col = 0;col < mat.cols - rect.width; col += widthStep) {
                rect.x = col;
                if (!isInRect(rects,rect)) {
                    sprintf(cutFilename + length + filenameLength,"-%d.png",count);
                    cut = Mat(mat,
                              Range(rect.y,rect.y+rect.height),//row range
                              Range(rect.x,rect.x+rect.width));//col range
                    if (!cut.empty()) {
                        count++;
                        resize(cut,resizeCut,Size(32,32),INTER_AREA);
                        imwrite(cutFilename, resizeCut);
                    }
                }
            }
        }
        rect.width = (int) (rect.width * FACTOR);
        rect.height = (int) (rect.height * FACTOR);
    }
    count = 0;
    length = strlen(POSTIVE_DIR_NAME);
    strcpy(cutFilename,POSTIVE_DIR_NAME);
    strcpy(cutFilename + length,filename);

    for (vector<Rect>::iterator iter = rects.begin(); iter != rects.end(); ++iter) {
        strcpy(cutFilename + length,filename);
        sprintf(cutFilename + length + filenameLength,"-%d.png",count);
        count++;
        if (iter->height > iter->width) {
            iter->width = iter->height;
        } else {
            iter->height = iter->width;
        }
        if (iter->y+iter->height > mat.rows) {
            iter->y =  mat.rows - iter->height - 1;
        }
        if (iter->x+iter->width > mat.cols) {
            iter->x =  mat.cols - iter->width - 1;
        }
        cut = Mat(mat,
                  Range(iter->y,iter->y+iter->height),//row range
                  Range(iter->x,iter->x+iter->width));//col range
        resize(cut,resizeCut,Size(64,64),INTER_AREA);
        imwrite(cutFilename, resizeCut);
    }

    printf("save finish\n");
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
