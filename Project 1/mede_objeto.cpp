#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

//const String calibFilesPath = "./calibration_files_mendelson/";
const String calibFilesPath = "./calibration_files_quaresma/";

RNG rng(12345);

// Threshold to find contours. Smaller values make us find more contours, but noisier
int thresh = 80;

// Limiar máximo usado para criar uma barra, não estou utilizando mais
const int max_thresh = 255;

//constantes de distorçao
float k1,k2,p1,p2;

//constantes intrinsecas
float fx,fy,cx,cy;

//distance of the object
float dist;

// Function that gets position of a click
void click_callback(int event, int x, int y, int flags, void* userdata);

// Function that generates contours
void thresh_callback(int, void*);


/**************
*  Classes    *
**************/
class Pair
{
private:
  int x;
  int y;
  //xp and yp of formula, the coordiantes of the image corrected
  float xp;
  float yp;
  // r² = x² + y²
  float r;
  //constante referente a formula que usa os ks pra multiplicar a primeira matriz
  float const_ks;
  //elementos da segunda matriz que irão somar na primeira
  float m1;
  float m2;
  //values of xp and yp in meters
  float xp_m;
  float yp_m;

public:
  Pair(): x(-1), y(-1) {}

  int getX(){ return x; };

  int getY(){ return y; };

  float getXp(){return xp;}

  float getYp(){return yp;}

  float getXpM(){return xp_m;}

  float getYpM(){return yp_m;}

  void setX(int newX){ x = newX; };

  void setY(int newY){ y = newY; };

  void setXp(int newXp){xp = newXp;}

  void setYp(int newYp){yp = newYp;}

  void setXpm(int newXpm){xp_m = newXpm;}

  void setYpm(int newYpm){yp_m = newYpm;}

  void calcR(){ r = sqrt(pow(x,2) + pow(y,2));}

  void calcConstKs(){const_ks =  1 + (k1*pow(r,2))   + (k2*pow(r,4));}

  void calcMat2 (){
    m1 = (2*p1*x*y) + (p2*(pow(r,2) + (2* pow(x,2))));
    m2 = (p1*(pow(r,2) + (2*pow(y,2)))) + (2*p2*x*y);
  }

  void calcXpYp(){
    calcR();
    calcConstKs();
    calcMat2();
    xp = (const_ks*x)+m1;
    yp = (const_ks*y)+m2;
  }

  void calcXpYpM(){
    calcXpYp();
    xp_m = xp*fx*dist/1000000000;
    yp_m = yp*fy*dist/1000000000;
  }

};

class objectMeasure
{
private:
  IplImage *image;

public:
  // Gray-level image to be analysed
  Mat img_gray;

  // Width
  Pair *initialW;
  Pair *finalW;
  float dW;

  // Height
  Pair *initialH;
  Pair *finalH;
  float dH;

  /** Methods **/
  objectMeasure()
  {
    cout << "Program has started!"<< endl;
    printf("Digite a distancia do objeto a ser medido:\n");
    scanf("%f",&dist);
    // Initializing width and height
    initialW = new Pair();
    finalW = new Pair();
    initialH = new Pair();
    finalH = new Pair();
    dW = -1;
    dH = -1;


    // Capturing from camera
    CvCapture* capture;
    capture = cvCreateCameraCapture( -1 );

    if(!capture)
    {
      printf("\nCouldn't open the camera\n");
      exit(-1);
    }

    // Camera captured image
    image = cvQueryFrame( capture );



    String intrinsicFile  = calibFilesPath + "Intrinsics.xml";
    String distortionFile = calibFilesPath + "Distortion.xml";

    // Reading .xml files for calibration
    CvMat *intrinsic  = (CvMat*)cvLoad(intrinsicFile.c_str());
    CvMat *distortion = (CvMat*)cvLoad(distortionFile.c_str());

    //Getting the values of constants fx,fy,cx and cy
    CvScalar aux_intensity;
    float aux_valor;
    printf("Intrinsic: \n");
          aux_intensity = cvGet2D(intrinsic,0,0);
          fx = aux_intensity.val[0];
          printf(" fx : %f \n",fx);
          aux_intensity = cvGet2D(intrinsic,1,1);
          fy = aux_intensity.val[0];
          printf(" fy : %f \n",fy);
          aux_intensity = cvGet2D(intrinsic,0,2);
          cx = aux_intensity.val[0];
          printf(" cx : %f \n",cx);
          aux_intensity = cvGet2D(intrinsic,1,2);
          cy = aux_intensity.val[0];
          printf(" cy : %f \n",cy);
      printf("\n");
      //Getting the values of constans k1,k2,p1,p2
    printf("Distortion: \n");
          aux_intensity = cvGet2D(distortion,0,0);
          k1 = aux_intensity.val[0];
          printf(" k1 : %f \n",k1);
          aux_intensity = cvGet2D(distortion,1,0);
          k2 = aux_intensity.val[0];
          printf(" k2 : %f \n",k2);
          aux_intensity = cvGet2D(distortion,2,0);
          p1 = aux_intensity.val[0];
          printf(" p1 : %f \n",p1);
          aux_intensity = cvGet2D(distortion,3,0);
          p2 = aux_intensity.val[0];
          printf(" p2 : %f \n",p2);
      printf("\n");

    // Matrices for map calibration
    IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    cvInitUndistortMap(intrinsic,distortion,mapx,mapy);

    // Naming windows
    cvNamedWindow( "Raw Video");
//  cvNamedWindow( "Undistorted Video" );

    // The following loop is interrupted only when user presses Esc. After that, we move to contours find functions
    while(image)
    {
//    IplImage *t = cvCloneImage(image);
      cvShowImage( "Raw Video", image );      // Show raw image
//    cvRemap( t, image, mapx, mapy );      // Undistort image
//    cvReleaseImage(&t);
//    cvShowImage("Undistorted Video", image);      // Show corrected image
          
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

    // Close video window
    destroyWindow( "Raw Video");

    // Intermediary Mat matrix
    Mat img(image);

    // Output instructions for user
    cout << "\nNow, you must click, on the 'Contours Gray' window, in the following order:\n"
         << "1) two points to calculate width, and\n"
         << "2) two point to calculate height" << endl;

    // Set window
    namedWindow( "Image", WINDOW_AUTOSIZE );

    // Show image
    imshow( "Image", img );

    // Transform img into img_gray
    cvtColor(img, img_gray, CV_BGR2GRAY);

    // Bluring img_gray
    blur( img_gray, img_gray, Size(3,3) );

    namedWindow( "Contours Gray", WINDOW_AUTOSIZE );

    // Create a bar to change contours thresh
    createTrackbar( "Canny thresh:", "Contours Gray", &thresh, max_thresh, thresh_callback, this );

    // Finding contours
    thresh_callback( thresh, this );
  }
};

int main()
{
  const objectMeasure *object = new objectMeasure();

  // Wait to finalize program
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
  objectMeasure *object = (objectMeasure*)(input);
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  // Detect edges using canny
  Canny( object->img_gray, canny_output, thresh, thresh*2, 3 );

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


  Mat drawing_gray;
  cvtColor(drawing, drawing_gray, CV_BGR2GRAY);
  //namedWindow( "Contours Gray", WINDOW_AUTOSIZE );

  // Set mouse callback
  setMouseCallback( "Contours Gray", click_callback, object);

  imshow( "Contours Gray", drawing_gray );
  /* tentativa de fazer abertura e fechamento para tirar pequenos ruidos vindos de outros objetos, funcionou bem em alguns casos mas em outros nao, considere usar se quiser =D
  //fechamento
  int morph_size = 1;
  Mat element = getStructuringElement( 2, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  morphologyEx( drawing_gray, drawing_gray, 3, element );
  //abertura
  morph_size = 3;
  element = getStructuringElement( 2, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  morphologyEx( drawing_gray, drawing_gray, 2, element );


  namedWindow( "Contours Gray Morphed", WINDOW_AUTOSIZE );
  imshow( "Contours Gray Morphed", drawing_gray );

  */
  Scalar intensity;
  int x,y;
  int aux;
  int xmaior = 0,xmenor = drawing_gray.cols;
  int ymaior = 0,ymenor = drawing_gray.rows;



  //Get the min and max values of x and y
  for(y=0;y<drawing_gray.rows;y++){
    for(x=0;x<drawing_gray.cols;x++){
        intensity = drawing_gray.at<int>(Point(x, y));
        int aux = intensity.val[0];
        if(aux!=0){
          if(x>xmaior){
            xmaior = x;
          }
          if(x<xmenor){
            xmenor = x;
          }
          if(y>ymaior){
            ymaior = y;
          }
          if(y<xmenor){
            ymenor = y;
          }


        }
    }
  }


  //printf("Largura do objeto em pixels: %d \n",(xmaior - xmenor));
  //printf("Altura do objeto em pixels: %d \n",(ymaior - ymenor));
}

void click_callback(int event, int x, int y, int flags, void* userdata)
{
  if (event == EVENT_LBUTTONUP)
  {
    //cout << "Left button released at " << x << ", " << y << endl;

    objectMeasure *object = (objectMeasure*)(userdata);

    if (object->initialW->getX() == -1 && object->initialW->getY() == -1)
    {
      object->initialW->setX(x);
      object->initialW->setY(y);
    }
    else if (object->finalW->getX() == -1 && object->finalW->getY() == -1)
    {
      object->finalW->setX(x);
      object->finalW->setY(y);
    }
    else if (object->initialH->getX() == -1 && object->initialH->getY() == -1)
    {
      object->initialH->setX(x);
      object->initialH->setY(y);
    }
    else if (object->finalH->getX() == -1 && object->finalH->getY() == -1)
    {
      object->finalH->setX(x);
      object->finalH->setY(y);

      float temp_dW;
      float temp_dH;

      //calculate XpCm and YpCm
      object->initialW->calcXpYpM();
      object->finalW->calcXpYpM();
      object->initialH->calcXpYpM();
      object->finalH->calcXpYpM();

      // Calculating distances in pixels ...
      temp_dW = object->finalW->getX() - object->initialW->getX();
      temp_dW *= temp_dW;
      temp_dW += pow(object->finalW->getY() - object->initialW->getY(), 2);
      temp_dW = sqrt(temp_dW);
      object->dW = temp_dW;
      cout << "\nWidth selected (pixels): " << object->dW << endl;

      temp_dH = object->finalH->getX() - object->initialH->getX();
      temp_dH *= temp_dH;
      temp_dH += pow(object->finalH->getY() - object->initialH->getY(), 2);
      temp_dH = sqrt(temp_dH);
      object->dH = temp_dH;
      cout << "\nHeight selected (pixels): " << object->dH << endl;

            // Calculating distances in meters ...
      printf("Calculando distancia em metros.. \n");
      temp_dW = object->finalW->getXpM() - object->initialW->getXpM();
      temp_dW *= temp_dW;
      object->dW = sqrt(temp_dW);
      cout << "\nWidth selected (meters): " << object->dW << endl;

      temp_dH = object->finalH->getYpM() - object->initialH->getYpM();
      temp_dH *= temp_dH;
      object->dH = sqrt(temp_dH);
      cout << "\nHeight selected (meters): " << object->dH << endl;

      //destroyWindow( "Image" );
    }
  }
}
