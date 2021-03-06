//
//  main.cpp
//  Virtual_Xylophone
//
//  Created by LucyLin on 4/17/15.
//  Copyright (c) 2015 LucyLin, Sang Bin Moon. All rights reserved.
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

#include <AL/alut.h>
#include <OpenAL/al.h>

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
void processMask(Mat& src);
void getProperty(const Mat src, Matx<int, 4, 2>& vertex);
Mat redFilter(const Mat& src);
Mat orangeFilter(const Mat& src);
Mat yellowFilter(const Mat& src);
Mat greenFilter(const Mat& src);
Mat blueFilter(const Mat& src);

//string workspace = "/Users/lucylin/Dropbox/class/VI/project/";
string workspace = "/Users/sangbinmin/Virtual_Xylophone";

#define NUM_BUFFERS 1
#define NUM_SOURCES 1
#define NUM_ENVIRONMENTS 1

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0, 0.0,1.0,0.0};

ALfloat source0Pos[]={ -2.0, 0.0, 0.0};
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

ALuint  buffer[NUM_BUFFERS];
ALuint  source[NUM_SOURCES];
ALuint  environment[NUM_ENVIRONMENTS];

ALsizei size,freq;
ALenum  audio_format;
ALvoid  *data;

void init(void)
{
    //alutInit(0, NULL);
    alutInitWithoutContext(NULL, NULL);
    
    alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);
    
    alGetError(); // clear any error messages
    
    /*
    // Generate buffers, or else no sound will happen!
    alGenBuffers(NUM_BUFFERS, buffer);
    
    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating buffers !!\n");
        exit(1);
    }
    else
    {
        printf("init() - No errors yet.");
    }
    
    string test = workspace+"sound/Piano/emo_dn_16_06.wav";
    alutLoadWAVFile(test.c_str(),&audio_format,&data,&size,&freq);
    alBufferData(buffer[0],audio_format,data,size,freq);
    alutUnloadWAV(audio_format,data,size,freq);
    */
    
    
    string test = workspace+"sound/Piano/emo_dn_16_06.wav";
    buffer[0] = alutCreateBufferFromFile(test.c_str());
    if (alutGetError() != ALUT_ERROR_NO_ERROR) {
        return;
    }
    
    
    alGetError(); /* clear error */
    alGenSources(NUM_SOURCES, source);
    
    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating sources !!\n");
        exit(2);
    }
    else
    {
        printf("init - no errors after alGenSources\n");
    }
    
    alSourcef(source[0], AL_PITCH, 1.0f);
    alSourcef(source[0], AL_GAIN, 1.0f);
    alSourcefv(source[0], AL_POSITION, source0Pos);
    alSourcefv(source[0], AL_VELOCITY, source0Vel);
    alSourcei(source[0], AL_BUFFER,buffer[0]);
    alSourcei(source[0], AL_LOOPING, AL_TRUE);
}


int main(int argc, const char * argv[])
{
    //alutInit(&argc, argv);
    init();
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
    
//    getProperty(redMask,redVertex);
//    getProperty(orangeMask,orangeVertex);
//    getProperty(yellowMask,yellowVertex);
//    getProperty(greenMask,greenVertex);
//    getProperty(blueMask,blueVertex);
    
//    imwrite(workspace+"Red"+to_string(i)+".jpg",redMask);
//    imwrite(workspace+"Orange"+to_string(i)+".jpg",orangeMask);
//    imwrite(workspace+"Yellow"+to_string(i)+".jpg",yellowMask);
//    imwrite(workspace+"Green"+to_string(i)+".jpg",greenMask);
//    imwrite(workspace+"Blue"+to_string(i)+".jpg",blueMask);
    
    for (int i = 1; i<150; i++)
    {
        alSourcePlay(source[0]);
        img = imread( workspace+"test_case2/"+to_string(i)+".jpg", 1 );
    
        coor = MatchingMethod( 0, 0, i);
        queue.push_back(coor);
        if(process_queue()) {
            cout<<"hit!!";
            //go through masks
            if(redMask.at<uchar>(lastDetection.y, lastDetection.x)>0)
                cout<<"red"<<endl;
            else if(orangeMask.at<uchar>(lastDetection.y, lastDetection.x)>0)
                cout<<"orange"<<endl;
            else if(yellowMask.at<uchar>(lastDetection.y, lastDetection.x)>0)
                cout<<"yellow"<<endl;
            else if(greenMask.at<uchar>(lastDetection.y, lastDetection.x)>0)
                cout<<"green"<<endl;
            else if(blueMask.at<uchar>(lastDetection.y, lastDetection.x)>0)
                cout<<"blue"<<endl;

        }
    }

    waitKey(0);
    alutExit();
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
    
    //confine the search area
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
    
    
    // change back to the x, y in original image
    matchLoc.y += anchor.y;
    matchLoc.x += anchor.x;
    
    // Show me what you got
    rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0 );
    rectangle( img_display, anchor, Point( anchor.x + padding_x , anchor.y + padding_y), Scalar::all(0), 2, 8, 0 );
    
    // update
    lastDetection = matchLoc;
    
//    imshow( image_window, img_display );
//    imshow( result_window, result );
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
