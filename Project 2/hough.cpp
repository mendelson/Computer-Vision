#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

//class that gets the image via the option choosen
class ImageGetter{
private:
	int option;
	char nome_arquivo[100];
	CvCapture* capture;
public:
	IplImage* image;

	ImageGetter(){
		setOptionFromMenu();
		getImage();
	}
	void setOptionFromMenu(){
		printf("*******************************\n");
		printf("De onde deseja obter a imagem?\n");
		printf("1 - arquivo de imagem\n");
		printf("2 - arquivo de video \n");
		printf("3 - câmera \n");
		printf("*******************************\n");
		scanf("%d",&option);
		if(option != 1 && option!=2 && option!=3){
			printf("Invalido!!!\n");
			exit(1);
		}
	}
	void getImage(){
		if(option == 1){
			getImageFromImageFile();
		}else if(option == 2){
			getImageFromVideo();
		}else{
			getImageFromCamera();
		}
	}

	void getImageFromImageFile(){
		printf("Digite o nome do arquivo(com extensão): \n");
		scanf("%s",nome_arquivo);
		image = cvLoadImage(nome_arquivo, CV_LOAD_IMAGE_COLOR);
		if(!image){
			printf("Nao foi possivel abrir a imagem! \n");
			exit(1);
		}
	}

	void getImageFromVideo(){
		printf("Digite o nome do arquivo(com extensão): \n");
		scanf("%s",nome_arquivo);
		printf("Aperte Esc quando chegar no frame desejado \n");
		cvNamedWindow( "Video", CV_WINDOW_AUTOSIZE );
		capture = cvCreateFileCapture( nome_arquivo);
		while(1) {
			image = cvQueryFrame( capture );
			if( !image ) break;
			cvShowImage( "Video", image );
			char c = cvWaitKey(33);
			if( c == 27 ) break;
		}
	cvReleaseCapture( &capture );
	cvDestroyWindow( "Video" );
	}

	void getImageFromCamera(){
		capture = cvCreateCameraCapture( -1 );
		if(!capture)
	    {
	      printf("\nCouldn't open the camera\n");
	      exit(-1);
	    }
	    image = cvQueryFrame( capture );
	    cvNamedWindow( "Camera");
	    while(image)
    	{
    		cvShowImage( "Camera", image );
    		char c = cvWaitKey(33);
    		if(c == 27)
        		break;
      		image = cvQueryFrame( capture );
    	}
    	destroyWindow( "Camera");
	}
};


//class that uses the Hough transform to detect lines and circles
class HoughDetector{
private:
	Mat *src,dst,cdst,src_gray;
	ImageGetter *imagegetter;
	vector<Vec4i> lines;
	vector<Vec3f> circles;
public:

	HoughDetector(){
		prepare();
		hough();
	}

	void prepare(){
		imagegetter = new ImageGetter();
		src = new Mat(imagegetter->image);
	}

	void prepareHoughLines(){
		Canny(*src, dst, 50, 200, 3);
 		cvtColor(dst, cdst, CV_GRAY2BGR);

	}

	void prepareHoughCircles(){
		cvtColor( *src, src_gray, CV_BGR2GRAY );
		GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
	}

	void makeHoughLines(){
		HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
	}

	void makeHoughCircles(){
		HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, 200, 100, 0, 0 );
	}

	void drawLines(){
		for( size_t i = 0; i < lines.size(); i++ )
		{
		  Vec4i l = lines[i];
		  line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
		}
	}

	void drawCicles(){
		for( size_t i = 0; i < circles.size(); i++ )
		{
		   Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		   int radius = cvRound(circles[i][2]);
		   circle( *src, center, 3, Scalar(0,255,0), -1, 8, 0 );
		   circle( *src, center, radius, Scalar(0,0,255), 3, 8, 0 );
		 }
	}

	void showHoughLines(){
		namedWindow( "Source", CV_WINDOW_AUTOSIZE );
		namedWindow( "Detected lines", CV_WINDOW_AUTOSIZE );
		imshow("Source", *src);
		imshow("Detected lines", cdst);
	}

	void showHoughCircles(){
		namedWindow( "Detected circles", CV_WINDOW_AUTOSIZE );
		imshow( "Detected circles", *src );
	}

	void houghLines(){
		prepareHoughLines();
		makeHoughLines();
		drawLines();
		showHoughLines();
	}

	void houghCircles(){
		prepareHoughCircles();
		makeHoughCircles();
		drawCicles();
		showHoughCircles();
	}

	void hough(){
		houghLines();
		houghCircles();
	}

};

int main(int argc, char** argv)
{
	HoughDetector *houghdetector = new HoughDetector();
	waitKey(0);
    return 0;
}
