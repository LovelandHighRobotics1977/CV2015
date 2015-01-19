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

Mat src; Mat src_gray; Mat dst;
int erosion_size = 2;

int main( int argc, char** argv )
{
  /// Load source image and convert it to gray
  src = imread( argv[1], 1 );

  ///  Resize to make image fit better, specify fx and fy and let the function compute the destination image size.
  resize(src, dst, Size(), 0.5, 0.5, INTER_AREA);

  /// Convert image to gray and blur it
  cvtColor( dst, src_gray, CV_BGR2HSV );
  blur( src_gray, src_gray, Size(3,3) );

  /// Create Window
  char* source_window = "Source";
  namedWindow( source_window, CV_WINDOW_AUTOSIZE );
  imshow( source_window, dst );

  inRange( src_gray, Scalar(20, 100, 100), Scalar(30, 255, 255), src_gray);

  //MORPH_RECT
  //MORPH_CROSS
  //MORPH_ELLIPSE

  erosion_size = 4;
  Mat element = getStructuringElement( MORPH_RECT, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point( erosion_size, erosion_size ) );

  /// Apply the erosion operation
  erode( src_gray, src_gray, element );

  char* gray_window = "GrayScale";
  namedWindow( gray_window, CV_WINDOW_AUTOSIZE );
  imshow( gray_window, src_gray );

  //createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
  //thresh_callback( 0, 0 );

  //Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  //int thresh = 100;
  //int max_thresh = 255;
  RNG rng(12345);


  /// Detect edges using canny
  //Canny( src_gray, canny_output, thresh, thresh*2, 3 );
  //Canny( src_gray, canny_output, 100, 200, 5 );

  /// Find contours
  findContours( src_gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Draw contours
  Mat drawing = Mat::zeros( src_gray.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
  {
      double CA = contourArea( contours[i], false );
      printf( "Countour( %d ) - Area: %g \n", i, CA );
      if( CA >= 50000 )
      {
         Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
         drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );

         Moments cm = moments( contours[i] );
    
         printf( "m00: %g\n", cm.m00 );
         printf( "m10: %g\n", cm.m10 );
         printf( "m01: %g\n", cm.m01 );
         printf( "m20: %g\n", cm.m20 );
         printf( "m11: %g\n", cm.m11 );
         printf( "m02: %g\n", cm.m02 );
         printf( "m30: %g\n", cm.m30 );
         printf( "m21: %g\n", cm.m21 );
         printf( "m12: %g\n", cm.m12 );
         printf( "m03: %g\n", cm.m03 );

         printf( "mu20: %g\n", cm.mu20 );
         printf( "mu11: %g\n", cm.mu11 );
         printf( "mu02: %g\n", cm.mu02 );
         printf( "mu30: %g\n", cm.mu30 );
         printf( "mu21: %g\n", cm.mu21 );
         printf( "mu12: %g\n", cm.mu12 );
         printf( "mu03: %g\n", cm.mu03 );

         printf( "nu20: %g\n", cm.nu20 );
         printf( "nu11: %g\n", cm.nu11 );
         printf( "nu02: %g\n", cm.nu02 );
         printf( "nu30: %g\n", cm.nu30 );
         printf( "nu21: %g\n", cm.nu21 );
         printf( "nu12: %g\n", cm.nu12 );
         printf( "nu03: %g\n", cm.nu03 );
         
         printf( "xbar: %g\n", (cm.m10/cm.m00) );
         printf( "ybar: %g\n", (cm.m01/cm.m00) );
         // central moments
         //double  mu20, mu11, mu02, mu30, mu21, mu12, mu03;
         // central normalized moments
         //double  nu20, nu11, nu02, nu30, nu21, nu12, nu03

         double hu[7];
         HuMoments(cm, hu);

         printf( "hu0: %g\n", hu[0] );
         printf( "hu1: %g\n", hu[1] );
         printf( "hu2: %g\n", hu[2] );
         printf( "hu3: %g\n", hu[3] );
         printf( "hu4: %g\n", hu[4] );
         printf( "hu5: %g\n", hu[5] );
         printf( "hu6: %g\n", hu[6] );

         circle( drawing, Point( (cm.m10/cm.m00), (cm.m01/cm.m00) ), 2, color );
      }
  }

  // Calculate the moments

  /// Show in a window
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );

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
