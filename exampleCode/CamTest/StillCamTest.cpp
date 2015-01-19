#include <iostream>
#include <fstream>
#include <cstdlib>

#include <raspicam/raspicam_still_cv.h>

using namespace std;

raspicam::RaspiCam_Still_Cv Camera;

int main( int argc, char *argv[] )
{
    int width  = 1280;
    int height = 960;

    cout << "Initializing ..." << width << "x" << height << endl;

    Camera.set( CV_CAP_PROP_FRAME_WIDTH, width );
    Camera.set( CV_CAP_PROP_FRAME_HEIGHT, height );
    Camera.open();

    cv::Mat image;

    cout << "capturing" << endl;

    if( !Camera.grab() )
    {
        cerr << "Error in grab" << endl;
        return -1;
    }

    Camera.retrieve( image );
    cout << "saving picture.jpg" << endl;

    cv::imwrite( "StillCamTest.jpg", image );
    return 0;
}

