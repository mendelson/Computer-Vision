#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

// Gray-level image to be analysed
Mat img_gray;

// Threshold to find contours. Smaller values make us find more contours, but noisier
int thresh = 20;

// Limiar máximo usado para criar uma barra, não estou utilizando mais
int max_thresh = 255;
RNG rng(12345);

// Function that generates contours
void thresh_callback(int, void*);

int main()
{
  cout << "Program has started!"<< endl;

  // Capturing from camera
  CvCapture* capture;
  capture = cvCreateCameraCapture( -1 );

  if(!capture)
  {
    printf("\nCouldn't open the camera\n");
    return -1;
  }

  // Camera captured image
  IplImage *image = cvQueryFrame( capture );

  // Reading .xml files for calibration
  CvMat *intrinsic = (CvMat*)cvLoad("Intrinsics.xml");
  CvMat *distortion = (CvMat*)cvLoad("Distortion.xml");

  // Matrices for map calibration
  IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  cvInitUndistortMap(intrinsic,distortion,mapx,mapy);

  // Naming windows
  cvNamedWindow( "Raw Video");
  cvNamedWindow( "Undistorted Video" );

  // The following loop is interrupted only when user presses Esc. After that, we move to contours find functions
  while(image)
  {
    IplImage *t = cvCloneImage(image);
    cvShowImage( "Raw Video", image );      // Show raw image
    cvRemap( t, image, mapx, mapy );      // Undistort image
    cvReleaseImage(&t);
    cvShowImage("Undistorted Video", image);      // Show corrected image
        
    int c = cvWaitKey(15);

    if(c == 'p')
    {
      c = 0;
      while(c != 'p' && c != 27)
      {
        c = cvWaitKey(250);
      }
    }

    if(c == 27)
      break;

    image = cvQueryFrame( capture );
  }

  // Intermediary Mat matrix
  Mat img(image);

  // Show captured image
  namedWindow( "Image", WINDOW_AUTOSIZE );
  imshow( "Image", img );

  // Transform img into img_gray
  cvtColor(img, img_gray, CV_BGR2GRAY);

  // Bluring img_gray
  blur( img_gray, img_gray, Size(3,3) );

  //cria uma barra para poder alterar o limiar de encontrar contornos durante a execucao, nao esta funcionando
  createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );

  // Finding contours
  thresh_callback( 0, 0 );

  // Wait to finalize program
  waitKey(0);

  return 0;
}



void thresh_callback(int, void* )
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  // Detect edges using canny
  Canny( img_gray, canny_output, thresh, thresh*2, 3 );

  // Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  // Draw contours
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for (int i = 0; i < contours.size(); i++)
  {
    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
  }

  // Show in a window
  namedWindow( "Contours", WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
}
