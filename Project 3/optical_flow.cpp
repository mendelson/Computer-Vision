#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "opencv2/legacy/legacy.hpp"
#define CVX_GRAY50 cvScalar(100)
#define CVX_WHITE  cvScalar(255)

using namespace std;
using namespace cv;

// Function that creates the trackbar
void thresh_callback(int, void*);

// Threshold to find contours. Smaller values make us find more contours, but noisier
int thresh = 10;

// Limiar máximo usado para criar uma barra, não estou utilizando mais
const int max_thresh = 100;


//class that gets the image via the option choosen
class ImageGetter{
private:
    int option;
    CvCapture* capture;
public:
    IplImage* image_new;
    IplImage* image_old;
    IplImage* image_new_gray;
    IplImage* image_old_gray;

    ImageGetter(){
        setOptionFromMenu();
        getFirstImages();
    }
    void setOptionFromMenu(){
        printf("*******************************\n");
        printf("Wich video to you want to apply the algorithm?\n");
        printf("1 - Video 1\n");
        printf("2 - Video 2\n");
        printf("3 - Video 3\n");
        printf("*******************************\n");
        scanf("%d",&option);
        if(option != 1 && option!=2 && option!=3){
            printf("Invalid option!!! Now, you'll be punished: rerun this program...\n");
            exit(1);
        }
    }

    void getFirstImages(){
        openVideo();
        getRgbImages();
        initGrays();
        getImages();
    }

    void initGrays(){
        //printf("Criando imagens\n");
        image_new_gray = cvCreateImage(cvGetSize(image_new),IPL_DEPTH_8U,1);
        image_old_gray = cvCreateImage(cvGetSize(image_new),IPL_DEPTH_8U,1);
        //printf("Incializou grays!\n");
    }

    void passRgbToGrays(){
        cvCvtColor(image_new,image_new_gray,CV_RGB2GRAY);
        cvCvtColor(image_old,image_old_gray,CV_RGB2GRAY);
        //printf("Passou pro gray!\n");
    }


    void openVideo(){
        if(option == 1){
            capture = cvCreateFileCapture("video1.mov");
        }else if(option == 2){
            capture = cvCreateFileCapture("video2.mov");
        }else{
            capture = cvCreateFileCapture("video3.mov");
        }

        printf("Abriu Video! \n");

         if (!capture)
              {
                    cout << "\nCouldn't open video file!" << endl;
                    exit(-2);
               }
    }

    void getImages(){
        getRgbImages();
        passRgbToGrays();
        //printf("Pegou imagens\n");
    }

    void getRgbImages(){
        //reinicia video
        if(!capture){
            cvReleaseCapture(&capture);
            openVideo();
        }
        image_old = image_new;
        image_new = cvQueryFrame(capture);
        //printf("Pegou imagens RGB\n");
    }

    void finish(){
        cvReleaseCapture(&capture);
        cvReleaseImage( &image_old );
        cvReleaseImage( &image_new );
        cvReleaseImage( &image_old_gray);
        cvReleaseImage( &image_new_gray );
    }


//  void getImageFromVideo(){
//      printf("Enter file's name (with extension): \n");
//      scanf("%s",file_name);
//      printf("\nPress Esc to capture a frame\n\n");
//
//      capture = cvCaptureFromFile(file_name);
//
//      if (!capture)
//      {
//          cout << "\nCouldn't open video file!" << endl;
//          exit(-2);
//      }
//
//      namedWindow("Video", CV_WINDOW_AUTOSIZE);
//      image = cvQueryFrame(capture);
//      cvShowImage("Video", image);
//
//      while(1)
//      {
//          if(!capture)
//          {
//              cout << "\nVideo has reached its end!" << endl;
//              break;
//          }
//
//          char c = waitKey(20);
//          if (c == 27)
//              break;
//
//          image = cvQueryFrame(capture);
//
//          cvShowImage("Video", image);
//      }
//
//      cvReleaseCapture(&capture);
//      destroyWindow("Video");
//
//      //cout << "\nimage channels: " << image->nChannels << endl;
//  }


};

//Class that implements the optical flow
class OpticalFlow{
public:
    ImageGetter* imagegetter;
     IplImage* velx ;
     IplImage* vely;
     IplImage* imgC;
     double p1,p2;


     OpticalFlow(){
         printf("Iniciando OpticalFlow! \n");
         flow();
     }
     void flow(){
         init();
         while(1){
             callHornSchunk();
             mountImageToShow();
             showImages();
             imagegetter->getImages();
             char c = cvWaitKey(33);
            if( c == 27 ) break;
        }
         finish();
     }

     void flowTest(){
        p1=0;
        p2=0;
        init();
        for(p1=0.01;p1<10;p1+=0.001){
            printf("p1: %lf \n",p1);
            for(p2=0;p2<0.0001;p2+=0.00000001){
                callHornSchunk();
                mountImageToShow();
                //showImages();
            }
        }
     }
     void init(){
         imagegetter = new ImageGetter();
         velx = cvCreateImage(cvGetSize(imagegetter->image_old_gray),IPL_DEPTH_32F,1);
         vely = cvCreateImage(cvGetSize(imagegetter->image_old_gray),IPL_DEPTH_32F,1);
         imgC = cvCreateImage(cvGetSize(imagegetter->image_old_gray),IPL_DEPTH_8U,3);
         createWindows();
         createTrackbar( "Canny thresh:", "Flow Results", &thresh, max_thresh, thresh_callback, this );
         thresh_callback(thresh,this);
         //printf("Iniciou com sucesso! \n");
     }

     void createWindows(){
         cvNamedWindow("Original Video");
         //cvNamedWindow("OpticalFlow0");
         //cvNamedWindow("OpticalFlow1");
         cvNamedWindow("Flow Results");
     }

     void callHornSchunk(){
         cvCalcOpticalFlowHS( imagegetter->image_old_gray, imagegetter->image_new_gray, 0,velx,vely,p1,cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,imagegetter->image_old->width, p2 ));
        //printf("Terminou HornShunk\n");
     }

     void mountImageToShow(){
         cvZero( imgC );
         int step = 4;
         for( int y=0; y<imgC->height; y += step ) {
             float* px = (float*) ( velx->imageData + y * velx->widthStep );
             float* py = (float*) ( vely->imageData + y * vely->widthStep );
                 for( int x=0; x<imgC->width; x += step ) {
                     if( px[x]>1 && py[x]>1 ) {
                        printf("P1 = %lf \n",p1);
                        printf("P2 = %lf \n",p2);
                        printf("Detectou!\n");
                        //waitKey(0);
                         cvCircle(imgC,cvPoint( x, y ),2,CVX_GRAY50, -1);
                         cvLine(imgC,cvPoint( x, y ), cvPoint( x+px[x]/2, y+py[x]/2 ), CV_RGB(255,0,0),1,  0);
                        }
                    }
                }
     }

     void showImages(){
         cvShowImage( "Original Video",imagegetter->image_old );
         //vShowImage( "OpticalFlow1",imagegetter->image_new );
         cvShowImage( "Flow Results",imgC );
     }

     void finish(){
         //cvDestroyWindow( "OpticalFlow0" );
         //cvDestroyWindow( "OpticalFlow1" );
         cvDestroyWindow( "Flow Results" );
         cvReleaseImage( &imgC );
         imagegetter->finish();
     }
};


int main(int argc, char** argv)
{
    OpticalFlow *opticalflow = new OpticalFlow();
    waitKey(0);
    return 0;
}


void thresh_callback(int i, void* input)
{
  //cout << "\ni = " << i << endl;
  if (input == NULL)
  {
    cout << "\nNull\n";
  }
 OpticalFlow *object = (OpticalFlow*)(input);
 object->p1 = 1/(thresh*10);
 object->p2 = 1/(thresh*10000);
}
