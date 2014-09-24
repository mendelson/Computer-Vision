#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
using namespace cv;

//Imagem final para análise em nivel de cinza
Mat img_gray;
//Limiar para achar contorno, quanto menor mais contorno encontra, porém encontra ruídos
int thresh = 20;
//Limiar máximo usado para criar uma barra, não estou utilizando mais
int max_thresh = 255;
RNG rng(12345);

//função que gera os contornos
void thresh_callback(int, void*);

int main()
{
  cout << "Pograma inciado!"<< endl;
  //Captura da camera
  CvCapture* capture;
  capture = cvCreateCameraCapture( -1 );
	if(!capture)
	{
		printf("\nCouldn't open the camera\n");
		return -1;
	}
	//Imagem capturada pela Camera
  IplImage *image = cvQueryFrame( capture );
    //Ler os arquivos xml da calibração
	CvMat *intrinsic = (CvMat*)cvLoad("Intrinsics.xml");
	CvMat *distortion = (CvMat*)cvLoad("Distortion.xml");

	// Cria matrizer de mapa para a calibração

	IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
	IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
	cvInitUndistortMap(intrinsic,distortion,mapx,mapy);

	// Cria janelas para a camera
	cvNamedWindow( "Raw Video");
	cvNamedWindow( "Undistort" );
	//Este loop vai até o usuário apertar Esc, quando o usuário apertar Esc, o loop para e vai para as funções de achar contorno
	while(image)
	{
		IplImage *t = cvCloneImage(image);
		cvShowImage( "Raw Video", image );			// Show raw image
		cvRemap( t, image, mapx, mapy );			// Undistort image
		cvReleaseImage(&t);
		cvShowImage("Undistort", image);			// Show corrected image
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
	//Cria uma matriz Mat para trabalhar nela
    Mat img(image);
    //mostra  a imagem pega
    namedWindow( "Imagem", WINDOW_AUTOSIZE );
    imshow( "Imagem", img );
    //Transforma a mat img em uma img_gray em escala de cinza
    cvtColor(img, img_gray, CV_BGR2GRAY);
    //bora a img_gray
    blur( img_gray, img_gray, Size(3,3) );

    //cria uma barra para poder alterar o limiar de encontrar contornos durante a execucao, nao esta funcionando
    createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
    //chama a funcao de encontrar contornos
  	thresh_callback( 0, 0 );
  	//espera para finalizar o programa
  	waitKey(0);


  	return 0;
   }



void thresh_callback(int, void* )
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using canny
  Canny( img_gray, canny_output, thresh, thresh*2, 3 );
  /// Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Draw contours
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
     }

  /// Show in a window
   namedWindow( "Contours", WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
}
