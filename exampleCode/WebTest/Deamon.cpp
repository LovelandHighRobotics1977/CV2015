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
#include <strstream>

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

    Camera.set( cv::CAP_PROP_FRAME_WIDTH, width );
    Camera.set( cv::CAP_PROP_FRAME_HEIGHT, height );
    Camera.set ( cv::CAP_PROP_BRIGHTNESS, 50 );
    Camera.set ( cv::CAP_PROP_CONTRAST, 50 );
    Camera.set ( cv::CAP_PROP_SATURATION, 50 );
    Camera.set ( cv::CAP_PROP_GAIN, 50 );
    //Camera.set ( cv::CAP_PROP_FORMAT, CV_8UC1 ); // Do Gray Scale
    //Camera.set ( cv::CAP_PROP_EXPOSURE, ?  );

    cout<<"Connecting to camera"<<endl;
    if ( !Camera.open() ) {
        cerr<<"Error opening camera"<<endl;
        return true;
    }

    cout<<"Connected to camera ="<<Camera.getId() <<endl;

    return false;
}

void stopCamera()
{
    Camera.release();
}

class SceneObj
{
    private:
        unsigned int    contourIndex;
        cv::RotatedRect minRect;
        cv::Moments     cm;

    public:

        SceneObj();
       ~SceneObj();

        void setContourIndex( unsigned int index );
        unsigned int getContourIndex();

        void setMoments( cv::Moments newMoments );
        unsigned int getContourArea();
        cv::Point getContourCentroid();

        void setMinRect( cv::RotatedRect newRect );
        cv::RotatedRect getMinRect();

};

SceneObj::SceneObj()
{

}

SceneObj::~SceneObj()
{

}

void
SceneObj::setContourIndex( unsigned int index )
{
    contourIndex = index;
}

unsigned int 
SceneObj::getContourIndex()
{
    return contourIndex;
}

void
SceneObj::setMoments( cv::Moments newMoments )
{
    cm = newMoments;
}

unsigned int 
SceneObj::getContourArea()
{
    return cm.m00;
}

cv::Point 
SceneObj::getContourCentroid()
{
    cv::Point centroid( (cm.m10/cm.m00), (cm.m01/cm.m00) );
    return centroid;
}

void
SceneObj::setMinRect( cv::RotatedRect newRect )
{
    minRect = newRect;
}

cv::RotatedRect
SceneObj::getMinRect()
{
    return minRect;
}

class Scene
{
    private:

        cv::Mat image;

        vector< vector< cv::Point > > contours;
        vector< cv::Vec4i > hierarchy;

        vector< SceneObj > objList;
        
        unsigned int minObjectArea;

    public:
        Scene();
       ~Scene();

        bool retrieveImage();

        bool thresholdImage();
        bool erodeImage();

        bool findObjects();   
        bool createObjectImage();     

        std::string getObjectListAsContent();

        int buildResponseImage();
};

Scene::Scene()
{
    minObjectArea = 10000;
}

Scene::~Scene()
{

}

bool 
Scene::retrieveImage()
{
    cout << "capturing" << endl;

    if( !Camera.grab() )
    {
        cerr << "Error in grab" << endl;
        return true;
    }

    Camera.retrieve( image );

    return false;
}

bool
Scene::thresholdImage()
{
    /// Convert image to gray and blur it
    cv::cvtColor( image, image, cv::COLOR_BGR2HSV );
    cv::blur( image, image, cv::Size(3,3) );

    //cv::inRange( src_gray, cv::Scalar(20, 100, 100), cv::Scalar(30, 255, 255), src_gray);
    //cv::inRange( src_gray, cv::Scalar(23, 100, 100), cv::Scalar(28, 255, 255), src_gray);
    //cv::inRange( src_gray, cv::Scalar(20, 100, 100), cv::Scalar(200, 255, 255), src_gray);

    // Red
    cv::inRange( image, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), image );

    return false;
}

bool
Scene::erodeImage()
{
    int erosion_size = 4;
    cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ), cv::Point( erosion_size, erosion_size ) );

    /// Apply the erosion operation
    cv::erode( image, image, element );

    return false;
}

bool
Scene::findObjects()
{
    /// Find contours
    findContours( image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    /// Look through contours
    for( int i = 0; i< contours.size(); i++ )
    {
        double CA = contourArea( contours[i], false );
        printf( "Countour( %d ) - Area: %g \n", i, CA );
        if( CA >= minObjectArea )
        {
            SceneObj obj; 

            obj.setContourIndex( i );
            obj.setMoments( moments( contours[i] ) );
            obj.setMinRect( minAreaRect( cv::Mat( contours[i] ) ) );

            objList.push_back( obj );
        }
    }

    return false;
}

bool
Scene::createObjectImage()
{
    cv::RNG rng(12345);

    cv::Mat drawing = cv::Mat::zeros( image.size(), CV_8UC3 );

    for( std::vector< SceneObj >::iterator it = objList.begin(); it != objList.end(); it++ )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, it->getContourIndex(), color, 2, 8, hierarchy, 0, cv::Point() );

        circle( drawing, it->getContourCentroid(), 2, color );

        // rotated rectangle
        cv::Point2f rect_points[4]; 
        it->getMinRect().points( rect_points );
        for( int j = 0; j < 4; j++ )
            line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );

    }
 
    // Replace the original image
    image = drawing;

    return false;
}

std::string 
Scene::getObjectListAsContent()
{
    stringstream rspData;;

    rspData << "<tote-list>";

    /// Draw contours
    for( std::vector< SceneObj >::iterator it = objList.begin(); it != objList.end(); it++ )
    {
        rspData << "<tote>";
        rspData << "<contour-area>" << it->getContourArea() << "</contour-area>";

        cv::Point centroid = it->getContourCentroid();
        rspData << "<centroid-x>" << (centroid.x) << "</centroid-x>";
        rspData << "<centroid-y>" << (centroid.y) << "</centroid-y>";

        rspData << "<rbox-cx>" << it->getMinRect().center.x << "</rbox-cx>";
        rspData << "<rbox-cy>" << it->getMinRect().center.y << "</rbox-cy>";
        rspData << "<rbox-w>" << it->getMinRect().size.width << "</rbox-w>";
        rspData << "<rbox-h>" << it->getMinRect().size.height << "</rbox-h>";
        rspData << "<rbox-angle>" << it->getMinRect().angle << "</rbox-angle>";
        rspData << "<rbox-aspect>" << (it->getMinRect().size.width/it->getMinRect().size.height) << "</rbox-aspect>";

        cv::Point2f rectPts[4];
        it->getMinRect().points( rectPts );

        double cornerDist[4];
        for( int j = 0; j < 4; j++ )
        {
            cornerDist[j] = pointPolygonTest( contours[ it->getContourIndex() ], rectPts[j], true );
            rspData << "<rbox-dist>" << cornerDist[j] << "</rbox-dist>";
        }

        rspData << "</tote>";
    }

    rspData << "</tote-list>";

    return rspData.str();
}

int
Scene::buildResponseImage()
{
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf, std::vector<int>() );
    
    imageBuf = (unsigned char *) realloc( imageBuf, buf.size() );
    memcpy( imageBuf, &buf[0], buf.size() );

    return buf.size();
}

int getImage()
{
    Scene scn;

    scn.retrieveImage();

    return scn.buildResponseImage();
}

int getToteImage()
{
    Scene scn;

    scn.retrieveImage();
    scn.thresholdImage();

    return scn.buildResponseImage();
}

int getContourImage()
{
    Scene scn;

    scn.retrieveImage();
    scn.thresholdImage();
    scn.erodeImage();

    scn.findObjects();
    scn.createObjectImage();

    return scn.buildResponseImage();
}

std::string getContourData()
{
    Scene scn;

    scn.retrieveImage();
    scn.thresholdImage();
    scn.erodeImage();

    scn.findObjects();
    scn.createObjectImage();

    return scn.getObjectListAsContent();
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

#if 1
    if( ( strcmp( url, "/tote/contours" ) == 0 ) || ( strcmp( url, "/tote/contours/" ) == 0 ) )
    {
        //struct MHD_Response *response;
        //int ret;
        int bufLength;

        bufLength = getContourImage();

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
    if( ( strcmp( url, "/tote/data" ) == 0 ) || ( strcmp( url, "/tote/data/" ) == 0 ) )
    {
  
        std::string rspData = getContourData();

        response = MHD_create_response_from_buffer( rspData.size(), (void *) rspData.c_str(), MHD_RESPMEM_MUST_COPY );
        MHD_add_response_header( response, "Content-Type", "text/xml" );
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
        return 1;

    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END );

    if (NULL == daemon)
        return 1;

    getchar ();

    MHD_stop_daemon (daemon);

    // Close the camera
    stopCamera();

    return 0;
}

