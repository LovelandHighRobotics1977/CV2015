#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <microhttpd.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

#include <raspicam/raspicam_cv.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

#define PORT 8888
#define FILENAME "../../TestImages/yellow_crate2.jpg"
//#define FILENAME "../../../Notes.txt"
#define MIMETYPE "image/jpg"

raspicam::RaspiCam_Cv Camera;

unsigned char *imageBuf = NULL;

bool startCamera()
{
    int width  = 1280;
    int height = 960;

    Camera.set( CV_CAP_PROP_FRAME_WIDTH, width );
    Camera.set( CV_CAP_PROP_FRAME_HEIGHT, height );
    Camera.set ( CV_CAP_PROP_BRIGHTNESS, 50 );
    Camera.set ( CV_CAP_PROP_CONTRAST, 50 ) );
    Camera.set ( CV_CAP_PROP_SATURATION, 50 ) );
    Camera.set ( CV_CAP_PROP_GAIN, 50 ) );
    //Camera.set ( CV_CAP_PROP_FORMAT, CV_8UC1 ); // Do Gray Scale
    //Camera.set ( CV_CAP_PROP_EXPOSURE, ?  );

    cout<<"Connecting to camera"<<endl;
    if ( !Camera.open() ) {
        cerr<<"Error opening camera"<<endl;
        return -1;
    }

    cout<<"Connected to camera ="<<Camera.getId() <<endl
}

void stopCamera()
{
    Camera.release();
}

int getImage()
{
//    int width  = 1280;
//    int height = 960;

//    cout << "Initializing ..." << width << "x" << height << endl;

//    Camera.set( CV_CAP_PROP_FRAME_WIDTH, width );
//    Camera.set( CV_CAP_PROP_FRAME_HEIGHT, height );
//    Camera.open();

    cv::Mat image;

    cout << "capturing" << endl;

    if( !Camera.grab() )
    {
        cerr << "Error in grab" << endl;
        return 0;
    }

    Camera.retrieve( image );
    cout << "saving picture.jpg" << endl;

    //cv::imwrite( "StillCamTest.jpg", image );

    cv::vector<uchar> buf;
    cv::imencode(".jpg", image, buf, std::vector<int>() );
    
    imageBuf = (unsigned char *) realloc( imageBuf, buf.size() );
    memcpy( imageBuf, &buf[0], buf.size() );

    return buf.size();
}

int getToteImage()
{
    int width  = 1280;
    int height = 960;

    cout << "Initializing ..." << width << "x" << height << endl;

    Camera.set( CV_CAP_PROP_FRAME_WIDTH, width );
    Camera.set( CV_CAP_PROP_FRAME_HEIGHT, height );

    //printf( "Format: %d\n", Camera.getFormat() );


    Camera.open();

    cv::Mat image;

    cout << "capturing" << endl;

    if( !Camera.grab() )
    {
        cerr << "Error in grab" << endl;
        return 0;
    }

    Camera.retrieve( image );
    cout << "saving picture.jpg" << endl;

    //cv::imwrite( "StillCamTest.jpg", image );
    cv::Mat src; cv::Mat src_gray; cv::Mat dst;

    /// Convert image to gray and blur it
    cv::cvtColor( image, src_gray, CV_BGR2HSV );
    cv::blur( src_gray, src_gray, cv::Size(3,3) );

    //cv::inRange( src_gray, cv::Scalar(20, 100, 100), cv::Scalar(30, 255, 255), src_gray);
    cv::inRange( src_gray, cv::Scalar(20, 100, 100), cv::Scalar(200, 255, 255), src_gray);

    //MORPH_RECT
    //MORPH_CROSS
    //MORPH_ELLIPSE

    //int erosion_size = 4;
    //cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ), cv::Point( erosion_size, erosion_size ) );

    /// Apply the erosion operation
    //cv::erode( src_gray, src_gray, element );

    cv::vector<uchar> buf;
    cv::imencode(".jpg", src_gray, buf, std::vector<int>() );
    
    imageBuf = (unsigned char *) realloc( imageBuf, buf.size() );
    memcpy( imageBuf, &buf[0], buf.size() );

    return buf.size();
}

static int
answer_to_connection( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls )
{
    struct MHD_Response *response;
    int ret;

    printf( "Request URL: %s\n", url );

#if 0
    const char *page = "<html><body>Hello, browser!</body></html>";
    //struct MHD_Response *response;
    //int ret;

    response = MHD_create_response_from_buffer( strlen( page ), (void *) page, MHD_RESPMEM_PERSISTENT );
    ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
    MHD_destroy_response( response );
    return ret;
#endif

#if 1
    if( ( strcmp( url, "/camera/raw" ) == 0 ) || ( strcmp( url, "/camera/raw/" ) == 0 ) )
    {
        //struct MHD_Response *response;
        //int ret;
        int bufLength;

        bufLength = getImage();

        printf( "Sending image -- size: %d\n", bufLength );

        if( bufLength == 0 )
            return MHD_NO;

        response = MHD_create_response_from_buffer( bufLength, (void *) imageBuf, MHD_RESPMEM_PERSISTENT );
        MHD_add_response_header( response, "Content-Type", MIMETYPE );
        ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
        MHD_destroy_response( response );
        return ret;
    }
#endif

#if 1
    if( ( strcmp( url, "/tote/image" ) == 0 ) || ( strcmp( url, "/tote/image/" ) == 0 ) )
    {
        //struct MHD_Response *response;
        //int ret;
        int bufLength;

        bufLength = getToteImage();

        printf( "Sending image -- size: %d\n", bufLength );

        if( bufLength == 0 )
            return MHD_NO;

        response = MHD_create_response_from_buffer( bufLength, (void *) imageBuf, MHD_RESPMEM_PERSISTENT );
        MHD_add_response_header( response, "Content-Type", MIMETYPE );
        ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
        MHD_destroy_response( response );
        return ret;
    }
#endif

#if 0
    //struct MHD_Response *response;
    int fd;
    //int ret;
    struct stat sbuf;

    if( 0 != strcmp (method, "GET") )
    {
        return MHD_NO;
    }

    if( (-1 == (fd = open( FILENAME, O_RDONLY ))) || (0 != fstat( fd, &sbuf )) )
    {
        /* error accessing file */
        if( fd != -1 ) 
        {
            close (fd);
        }

        const char *errorstr = "<html><body>An internal server error has occured!</body></html>";
  
        response = MHD_create_response_from_buffer( strlen( errorstr ), (void *) errorstr, MHD_RESPMEM_PERSISTENT );

        if( response )
        {
            ret = MHD_queue_response( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response );
            MHD_destroy_response( response );
            return MHD_YES;
        }
        else
            return MHD_NO;
    }

    printf( "Sending image -- size: %d\n", sbuf.st_size );
 
    response = MHD_create_response_from_fd( sbuf.st_size, fd );
    MHD_add_response_header( response, "Content-Type", MIMETYPE );
    ret = MHD_queue_response( connection, MHD_HTTP_OK, response );
    MHD_destroy_response( response );
    
    return ret;

#endif

    const char *errorstr = "<html><body>Request not understood.</body></html>";
  
    response = MHD_create_response_from_buffer( strlen( errorstr ), (void *) errorstr, MHD_RESPMEM_PERSISTENT );

    if( response )
    {
        ret = MHD_queue_response( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response );
        MHD_destroy_response( response );
        return MHD_YES;
    }
    else
        return MHD_NO;

}

int
main ()
{
    struct MHD_Daemon *daemon;

    // Try to open the camera
    if( startCamera() )
        return;

    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END );

    if (NULL == daemon)
        return 1;

    getchar ();

    MHD_stop_daemon (daemon);

    // Close the camera
    stopCamera();

    return 0;
}

