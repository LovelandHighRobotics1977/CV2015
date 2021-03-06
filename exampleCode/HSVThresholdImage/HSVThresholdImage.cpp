//#include "cv.h"
//#include "highgui.h"
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

IplImage* GetThresholdedImage(IplImage* img)
{
    // Convert the image into an HSV image
    IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
    cvCvtColor(img, imgHSV, CV_BGR2HSV);

    IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);

    cvInRangeS(imgHSV, cvScalar(20, 100, 100), cvScalar(30, 255, 255), imgThreshed);

    cvReleaseImage(&imgHSV);
    return imgThreshed;
}

Mat src; Mat src_gray;

int main( int argc, char** argv )
{
  /// Load source image and convert it to gray
  src = imread( argv[1], 1 );

  /// Convert image to gray and blur it
  cvtColor( src, src_gray, CV_BGR2HSV );
  blur( src_gray, src_gray, Size(3,3) );

  /// Create Window
  char* source_window = "Source";
  namedWindow( source_window, CV_WINDOW_AUTOSIZE );
  imshow( source_window, src );

  inRange( src_gray, Scalar(20, 100, 100), Scalar(30, 255, 255), src_gray);

  char* gray_window = "GrayScale";
  namedWindow( gray_window, CV_WINDOW_AUTOSIZE );
  imshow( gray_window, src_gray );

  //createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
  //thresh_callback( 0, 0 );

  waitKey(0);
  return(0);
}

#if 0
int main()
{
    // Initialize capturing live feed from the camera
    CvCapture* capture = 0;

    capture = cvCaptureFromCAM(0);

    // Couldn't get a device? Throw an error and quit
    if(!capture)
    {
        printf("Could not initialize capturing...");
        return -1;
    }

    // The two windows we'll be using
    cvNamedWindow("video");
    cvNamedWindow("thresh");

    // This image holds the "scribble" data...
    // the tracked positions of the ball
    IplImage* imgScribble = NULL;

    // An infinite loop
    while(true)
    {
        // Will hold a frame captured from the camera
        IplImage* frame = 0;
        frame = cvQueryFrame(capture);

        // If we couldn't grab a frame... quit
        if(!frame)
            break;

        // If this is the first frame, we need to initialize it
        if(imgScribble == NULL)
        {
            imgScribble = cvCreateImage(cvGetSize(frame), 8, 3);
        }

        // Holds the yellow thresholded image (yellow = white, rest = black)
        IplImage* imgYellowThresh = GetThresholdedImage(frame);

        // Calculate the moments to estimate the position of the ball
        CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));

        cvMoments(imgYellowThresh, moments, 1);

        // The actual moment values
        double moment10 = cvGetSpatialMoment(moments, 1, 0);
        double moment01 = cvGetSpatialMoment(moments, 0, 1);

        double area = cvGetCentralMoment(moments, 0, 0);

        // Holding the last and current ball positions
        static int posX = 0;

        static int posY = 0;

        int lastX = posX;

        int lastY = posY;

        posX = moment10/area;
        posY = moment01/area;

        // Print it out for debugging purposes
        printf("position (%d,%d)", posX, posY);

        // We want to draw a line only if its a valid position
        if(lastX>0 && lastY>0 && posX>0 && posY>0)
        {
            // Draw a yellow line from the previous point to the current point
            cvLine(imgScribble, cvPoint(posX, posY), cvPoint(lastX, lastY), cvScalar(0,255,255), 5);

        }

        // Add the scribbling image and the frame...
        cvAdd(frame, imgScribble, frame);
        cvShowImage("thresh", imgYellowThresh);
        cvShowImage("video", frame);

        // Wait for a keypress
        int c = cvWaitKey(10);
        if(c!=-1)
        {
            // If pressed, break out of the loop
            break;
        }

        // Release the thresholded image+moments... we need no memory leaks.. please
        cvReleaseImage(&imgYellowThresh);
        delete moments;
    }

    // We're done using the camera. Other applications can now use it
    cvReleaseCapture(&capture);
    return 0;
}
#endif
