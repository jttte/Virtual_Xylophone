//
//  main.cpp
//  Virtual_Xylophone
//
//  Created by LucyLin on 4/17/15.
//  Copyright (c) 2015 LucyLin, Sang Bin Moon. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/video/tracking.hpp>
#include "opencv2/core/core.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <dos.h>

//photobooth video has fps=15


using namespace std;
using namespace cv;

Mat src; int thresh = 100;

/// Global Variables
Mat img; Mat result;
char* image_window = "Source Image";
char* result_window = "Result window";

Point lastDetection;
vector<int> queue;
bool hit = false;

//template matching method parameter for OpenCV function
int match_method = 3;
int max_Trackbar = 5;

//image parameter
int W = 540;
int H = 360;

/// Function Headers
int MatchingMethod( int, void*);
int MatchingMethod( int, void*, int);
void showImage(Mat img, string name);
bool process_queue();
void processMask(Mat& src);
void getProperty(const Mat src, Matx<int, 4, 2>& vertex);
Mat redFilter(const Mat& src);
Mat orangeFilter(const Mat& src);
Mat yellowFilter(const Mat& src);
Mat greenFilter(const Mat& src);
Mat blueFilter(const Mat& src);

string workspace = "/Users/lucylin/Dropbox/class/VI/project/";

class stick{
    public:
        Point bound_left_up_;
        Point bound_right_buttom_;
        Point match_point_;
        Point last_detect_point_;
        int padding;
        Mat templ;
    
        stick(string path){
            last_detect_point_.x = 0;
            last_detect_point_.y = 0;
            padding = 60;
            templ = imread(path, 1);
        }
        Point match (int idx);
    
};

Point stick::match (int idx) {
    /// Source image to display
    Mat img_display;
    img.copyTo( img_display );
    int padding_x;
    int padding_y;

    
    //confine the search area
    if (last_detect_point_.x == 0 && last_detect_point_.y == 0) {//first iteration, search lower 3/5 of the image
        bound_left_up_.x = 0;
        bound_left_up_.y = img.rows/5*2;
        
        bound_right_buttom_.x = img.cols;
        bound_right_buttom_.y = img.rows/5*3;
    } else {
        bound_left_up_.x = last_detect_point_.x - padding > 0 ? last_detect_point_.x - padding: 0;
        bound_left_up_.y = last_detect_point_.y - padding > 0 ? last_detect_point_.y - padding: 0;
        bound_right_buttom_.x = bound_left_up_.x + templ.cols + padding * 2;
        bound_right_buttom_.y = bound_left_up_.y + templ.rows + padding * 2;
        
        if(bound_right_buttom_.x > W) bound_right_buttom_.x = W;
        if(bound_right_buttom_.y > H) bound_right_buttom_.y = H;
        
    }
    padding_x = bound_right_buttom_.x - bound_left_up_.x ;
    padding_y = bound_right_buttom_.y - bound_left_up_.y ;
    
    ///create image of lower half region
    Mat smallImg = Mat(img, Rect(bound_left_up_.x, bound_left_up_.y, padding_x, padding_y));
    int result_cols =  smallImg.cols - templ.cols + 1;
    int result_rows = smallImg.rows - templ.rows + 1;
    
    result.create( result_rows, result_cols, CV_32FC1 );
    
    /// Do the Matching and Normalize
    matchTemplate( smallImg, templ, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
    
    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    
    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    { match_point_ = minLoc; }
    else
    { match_point_ = maxLoc; }
    
    
    // change back to the x, y in original image
    match_point_.y += bound_left_up_.y;
    match_point_.x += bound_left_up_.x;
    
    // Show me what you got
    rectangle( img_display, match_point_, Point( match_point_.x + templ.cols , match_point_.y + templ.rows), Scalar::all(0), 2, 8, 0 );
    rectangle( img_display, bound_left_up_, Point( bound_left_up_.x + padding_x , bound_left_up_.y + padding_y), Scalar::all(0), 2, 8, 0 );
    
    // update
    last_detect_point_ = match_point_;
    
    imwrite(workspace+"test_case2/"+"result_"+to_string(idx)+".jpg", img_display);
    
    cout<< idx<<" " <<match_point_.y<<endl;
    
    return match_point_;

    
};


int main(int argc, const char * argv[])
{
    
//    VideoCapture cap;
//    string filepath = workspace+"test_video2.mov";
//    cap.open(filepath.c_str());
//    
//    if( !cap.isOpened() )
//    {
//        printf("can not open camera or video file\n");
//        return -1;
//    }
//    
//    
//    int count = 0;
//    for(;;count++)
//    {
//        cap >> src;
//        if (count > 150)
//            break;
//
//        resize(src, src, Size(int(src.cols * 0.5), int(src.rows * 0.5)));
//        imwrite(workspace+"test_case2/"+to_string(count)+".jpg", src);
//    }
    
    
    /// Create windows
    namedWindow( image_window, CV_WINDOW_AUTOSIZE );
    namedWindow( result_window, CV_WINDOW_AUTOSIZE );
    /// Create Trackbar
//    char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
//    createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );
    
    
    //process 'color keyboards'
    Mat frame,hsv_frame;
    Mat redMask,orangeMask,yellowMask,greenMask,blueMask;
    //int redSize,orangeSize,yellowSize,greenSize,blueSize;
    Matx<int, 4, 2> redVertex,orangeVertex,yellowVertex,greenVertex,blueVertex;
    
    //use the first frame to locate color regions
    int i = 0;
    frame = imread( workspace+"test_case2/"+to_string(i)+".jpg", 1 );
    cvtColor(frame, hsv_frame, CV_BGR2HSV);
    
    redMask = redFilter(hsv_frame);
    orangeMask = orangeFilter(hsv_frame);
    yellowMask = yellowFilter(hsv_frame);
    greenMask = greenFilter(hsv_frame);
    blueMask = blueFilter(hsv_frame);
    
    processMask(redMask);
    processMask(orangeMask);
    processMask(yellowMask);
    processMask(greenMask);
    processMask(blueMask);
    
    stick stick1(workspace+"test_case2/"+"template.jpg");
    
    
    for (int i = 1; i<150; i++)
    {
        
        img = imread( workspace+"test_case2/"+to_string(i)+".jpg", 1 );
        Point coor = stick1.match(i);
    
        //coor = MatchingMethod( 0, 0, i);
        queue.push_back(coor.y);
        if(process_queue()) {
            cout<<"hit!!";
            //go through masks
            if(redMask.at<uchar>(coor.y, coor.x)>0)
                cout<<"red"<<endl;
            else if(orangeMask.at<uchar>(coor.y, coor.x)>0)
                cout<<"orange"<<endl;
            else if(yellowMask.at<uchar>(coor.y, coor.x)>0)
                cout<<"yellow"<<endl;
            else if(greenMask.at<uchar>(coor.y, coor.x)>0)
                cout<<"green"<<endl;
            else if(blueMask.at<uchar>(coor.y, coor.x)>0)
                cout<<"blue"<<endl;
            
        }
    }
    return 0;

}

bool process_queue() {
    if ( queue.size() < 8 )
        return false;
    if ( queue.size() > 15 )
        queue.erase(queue.begin());
    
    unsigned int partition = queue.size()/3;
    float part1 = 0;
    float part2 = 0;
    float part3 = 0;
    for (unsigned int i = 0; i<partition; i++)
        part1 += queue[i];
    for (unsigned int i = partition; i< 2 * partition; i++)
        part2 += queue[i];
    for (unsigned int i = 2 * partition; i < queue.size(); i++)
        part3 += queue[i];
    if(part3 > part2 && part1 > part2) {
        queue.erase(queue.begin(),queue.begin()+2*partition);
        return true;
    }
    return false;
}



void showImage(Mat img, const string name) {
    //namedWindow( name, CV_WINDOW_AUTOSIZE );
    //imshow( name, img );
    
}

Mat redFilter(const Mat& src)
{
    Mat upperRed, lowerRed, redImage;
    inRange(src, Scalar(170, 100, 120), Scalar(255, 255, 255), upperRed);
    inRange(src, Scalar(0, 100, 120), Scalar(10, 255, 255), lowerRed);
    bitwise_or(upperRed, lowerRed, redImage);
    
    return redImage;
}
Mat orangeFilter(const Mat& src)
{
    Mat orangeImage;
    inRange(src, Scalar(10, 140, 130), Scalar(20, 255, 255), orangeImage);
    
    return orangeImage;
}
Mat yellowFilter(const Mat& src)
{
    Mat yellowImage;
    inRange(src, Scalar(20, 50, 100), Scalar(30, 255, 255), yellowImage);
    
    return yellowImage;
}
Mat greenFilter(const Mat& src)
{
    Mat greenImage;
    inRange(src, Scalar(35, 50, 50), Scalar(75, 255, 255), greenImage);
    
    return greenImage;
}
Mat blueFilter(const Mat& src)
{
    Mat blueImage;
    inRange(src, Scalar(85, 30, 50), Scalar(130, 255, 255), blueImage);
    
    return blueImage;
}

void processMask(Mat& mask)
{
    // cut off the upper 2/5 of the image
    for(int row=0;row<H*2/5;row++){
        for(int col=0;col<W;col++){
            mask.at<uchar>(row,col) = 0;
        }
    }
    
    // remove small regions in mask
    erode(mask, mask, getStructuringElement(MORPH_RECT, Size(3,3)),Point(-1,-1),5);
    dilate(mask, mask, getStructuringElement(MORPH_RECT, Size(3,3)),Point(-1,-1),5);
    
    // remove small holes in mask
    dilate(mask, mask, getStructuringElement(MORPH_RECT, Size(3,3)),Point(-1,-1),5);
    erode(mask, mask, getStructuringElement(MORPH_RECT, Size(3,3)),Point(-1,-1),5);
    
    //    return mask;
}

void getProperty(const Mat src, Matx<int, 4, 2>& vertex){
    Size WH=src.size();
    Mat edges;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    int minThreshold = 2;
    
    // find the polygon inside the mask
    Canny(src, edges, minThreshold, minThreshold*3);
    findContours(edges, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    vector<vector<Point> > contours_poly(contours.size());
    
    // find the four corners and store them in vertex
    int UL=5000,UR=0,DL=5000,DR=-5000;
    int sumXY,diffXY;
    for(int i=0;i<contours.size();i++){
        approxPolyDP(contours[i], contours_poly[i], 3, false);
        vector<Point> temp = contours_poly[i];
        cout<<temp.size()<<endl;
        for(int j=0;j<temp.size();j++){
            sumXY = temp[j].x+temp[j].y;
            if(sumXY<UL){
                UL=sumXY;
                vertex(0,0) = temp[j].x;
                vertex(0,1) = temp[j].y;
            }
            if(sumXY>DR){
                DR=sumXY;
                vertex(3,0) = temp[j].x;
                vertex(3,1) = temp[j].y;
            }
            diffXY = temp[j].x-temp[j].y;
            if(diffXY<DL){
                DL=diffXY;
                vertex(2,0) = temp[j].x;
                vertex(2,1) = temp[j].y;
            }
            if(diffXY>UR){
                UR=diffXY;
                vertex(1,0) = temp[j].x;
                vertex(1,1) = temp[j].y;
            }
        }
    }
    
    // get the area of color mask
    /*
    size=0;
    for(int r=1;r<=WH.height;r++){
        for(int c=1;c<WH.width;c++){
            if(src.at<uchar>(r,c)>0){
                size++;
            }
        }
    }
     */
}
