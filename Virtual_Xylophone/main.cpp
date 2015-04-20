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


using namespace std;
using namespace cv;

Mat src; int thresh = 100;


/// Function header
void showImage(Mat img, string name);
/// Global Variables
Mat img; Mat templ; Mat result;
char* image_window = "Source Image";
char* result_window = "Result window";

int match_method = 3;
int max_Trackbar = 5;

/// Function Headers
void MatchingMethod( int, void*);
void MatchingMethod( int, void*, int);


//int num = 0;
string workspace = "/Users/lucylin/Dropbox/class/VI/project/";
//test case: 15.jpg (267, 421) (293, 447)

int main(int argc, const char * argv[])
{
//    src = imread(workspace+"test.png");
//    int cn = src.channels();
//    Scalar_<uint8_t> bgrPixel;
//    uint8_t* pixelPtr = (uint8_t*)src.data;
//    
//    for(int i = 0; i < src.rows; i++)
//    {
//        for(int j = 0; j < src.cols; j++)
//        {
//            bgrPixel.val[0] = pixelPtr[i*src.cols*cn + j*cn + 0]; // B
//            cout<<int(bgrPixel.val[0])<<endl;
//            bgrPixel.val[1] = pixelPtr[i*src.cols*cn + j*cn + 1]; // G
//            bgrPixel.val[2] = pixelPtr[i*src.cols*cn + j*cn + 2]; // R
//            
//            // do something with BGR values...
//        }
//    }
    
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

    /// Load image and template
    templ = imread( workspace+"test_case2/"+"template.jpg", 1 );
    for (int i = 0; i<150; i++)
    {
        img = imread( workspace+"test_case2/"+to_string(i)+".jpg", 1 );
    
        MatchingMethod( 0, 0, i);
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
void MatchingMethod( int, void*, int idx)
{
    /// Source image to display
    Mat img_display;
    img.copyTo( img_display );
    int padding = img.rows/5*2;
    
    ///create image of lower half region
    Mat smallImg = Mat(img, Rect(0,padding,img.cols,img.rows-padding));
    
    /// Create the result matrix
//    int result_cols =  img.cols - templ.cols + 1;
//    int result_rows = img.rows - templ.rows + 1;
    int result_cols =  smallImg.cols - smallImg.cols + 1;
    int result_rows = smallImg.rows - smallImg.rows + 1;
    
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
    
    ///change back to the original y
    matchLoc.y += padding;
    /// Show me what you got
    rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0 );
//    rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
    
    imshow( image_window, img_display );
    imshow( result_window, result );
    imwrite(workspace+"test_case2/"+"result_"+to_string(idx)+".jpg", img_display);
    
    cout<< idx<<" " <<matchLoc.y<<endl;
    
    return;
}



void showImage(Mat img, const string name) {
    //namedWindow( name, CV_WINDOW_AUTOSIZE );
    //imshow( name, img );
    
}

