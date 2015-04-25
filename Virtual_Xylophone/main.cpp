//
//  main.cpp
//  Virtual_Xylophone
//
//  Created by LucyLin on 4/17/15.
//  Copyright (c) 2015 LucyLin. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/video/tracking.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

//photobooth video has fps=15


using namespace std;
using namespace cv;

Mat src; int thresh = 100;

/// Global Variables
Mat img; Mat templ; Mat result;
char* image_window = "Source Image";
char* result_window = "Result window";
int W;
int H;
Point lastDetection;
vector<int> queue;
bool hit = false;

//template matching method parameter for OpenCV function
int match_method = 3;
int max_Trackbar = 5;


/// Function Headers
int MatchingMethod( int, void*);
int MatchingMethod( int, void*, int);
void showImage(Mat img, string name);
bool process_queue();


string workspace = "/Users/lucylin/Dropbox/class/VI/project/";
//test case: 15.jpg (267, 421) (293, 447)

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
    
    //initilize detecteion point
    lastDetection.x = 0;
    lastDetection.y = 0;
    
    W = 540;
    H = 360;
    
    /// Create windows
    namedWindow( image_window, CV_WINDOW_AUTOSIZE );
    namedWindow( result_window, CV_WINDOW_AUTOSIZE );
    /// Create Trackbar
//    char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
//    createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );

    /// Load image and template
    int coor;
    templ = imread( workspace+"test_case2/"+"template.jpg", 1 );
    for (int i = 0; i<150; i++)
    {
        img = imread( workspace+"test_case2/"+to_string(i)+".jpg", 1 );
    
        coor = MatchingMethod( 0, 0, i);
        queue.push_back(coor);
        if(process_queue())
         cout<<"hit!!";
    }
    
//    img = imread( workspace+"test_case2/"+"136.jpg", 1 );
//    MatchingMethod( 0, 0);

    waitKey(0);
    return 0;

}

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
int MatchingMethod( int, void*, int idx)
{
    /// Source image to display
    Mat img_display;
    img.copyTo( img_display );
    Point anchor;
    int padding_x;
    int padding_y;
    int bound_x;
    int bound_y;
    if (lastDetection.x == 0 && lastDetection.y == 0) {//first iteration, search lower 3/5 of the image
        anchor.x = 0;
        anchor.y = img.rows/5*2;
        
        padding_x = img.cols;
        padding_y = img.rows/5*3;
    } else {
        anchor.x = lastDetection.x - 60 > 0 ? lastDetection.x - 60: 0;
        anchor.y = lastDetection.y - 60 > 0 ? lastDetection.y - 60: 0;
        bound_x = anchor.x + templ.cols + 60 * 2;
        bound_y = anchor.y + templ.rows + 60 * 2;

        if(bound_x > W) bound_x = W;
        if(bound_y > H) bound_y = H;
        
        padding_x = bound_x - anchor.x ;
        padding_y = bound_y - anchor.y ;
        
    }
    
    ///create image of lower half region
    Mat smallImg = Mat(img, Rect(anchor.x, anchor.y, padding_x, padding_y));
    //cout<<"last: x "<<to_string(lastDetection.x)<<" y "<<to_string(lastDetection.y)<<endl;
    //cout<<"anchor: x "<<to_string(anchor.x)<<" y "<<to_string(anchor.y)<<endl;
    //cout<<"anchor + padding x "<<to_string(padding_x + anchor.x)<<" y "<<to_string(anchor.y+padding_y)<<endl;
    
    /// Create the result matrix
//    int result_cols =  img.cols - templ.cols + 1;
//    int result_rows = img.rows - templ.rows + 1;
    int result_cols =  smallImg.cols - templ.cols + 1;
    int result_rows = smallImg.rows - templ.rows + 1;
    
    result.create( result_rows, result_cols, CV_32FC1 );
    
    /// Do the Matching and Normalize
    matchTemplate( smallImg, templ, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
    
    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    
    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
    else
    { matchLoc = maxLoc; }
    
    
    ///change back to the x, y in original image
    matchLoc.y += anchor.y;
    matchLoc.x += anchor.x;
    
    /// Show me what you got
    rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0 );
    rectangle( img_display, anchor, Point( anchor.x + padding_x , anchor.y + padding_y), Scalar::all(0), 2, 8, 0 );
//    rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    
    lastDetection = matchLoc;
    
    imshow( image_window, img_display );
    imshow( result_window, result );
    imwrite(workspace+"test_case2/"+"result_"+to_string(idx)+".jpg", img_display);
    
    cout<< idx<<" " <<matchLoc.y<<endl;
    
    return matchLoc.y;
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

