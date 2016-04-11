#if 0
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

using namespace cv;
using namespace std;

static void help()
{
	cout << "\nThis program demonstrated the floodFill() function\n"
		"Call:\n"
		"./ffilldemo [image_name -- Default: fruits.jpg]\n" << endl;

	cout << "Hot keys: \n"
		"\tESC - quit the program\n"
		"\tc - switch color/grayscale mode\n"
		"\tm - switch mask mode\n"
		"\tr - restore the original image\n"
		"\ts - use null-range floodfill\n"
		"\tf - use gradient floodfill with fixed(absolute) range\n"
		"\tg - use gradient floodfill with floating(relative) range\n"
		"\t4 - use 4-connectivity mode\n"
		"\t8 - use 8-connectivity mode\n" << endl;
}

Mat image0, image, gray, mask;
int ffillMode = 1;
int loDiff = 20, upDiff = 20;
int connectivity = 4;
int isColor = true;
bool useMask = false;
int newMaskVal = 255;

static void onMouse( int event, int x, int y, int, void* )
{
	if( event != CV_EVENT_LBUTTONDOWN )
		return;

	Point seed = Point(x,y);
	int lo = ffillMode == 0 ? 0 : loDiff;
	int up = ffillMode == 0 ? 0 : upDiff;
	int flags = connectivity + (newMaskVal << 8) +
		(ffillMode == 1 ? CV_FLOODFILL_FIXED_RANGE : 0);
	int b = (unsigned)theRNG() & 255;
	int g = (unsigned)theRNG() & 255;
	int r = (unsigned)theRNG() & 255;
	Rect ccomp;

	Scalar newVal = isColor ? Scalar(b, g, r) : Scalar(r*0.299 + g*0.587 + b*0.114);
	Mat dst = isColor ? image : gray;
	int area;

	if( useMask )
	{
		threshold(mask, mask, 1, 128, CV_THRESH_BINARY);
		area = floodFill(dst, mask, seed, newVal, &ccomp, Scalar(lo, lo, lo),
			Scalar(up, up, up), flags);
		imshow( "mask", mask );
	}
	else
	{
		area = floodFill(dst, seed, newVal, &ccomp, Scalar(lo, lo, lo),
			Scalar(up, up, up), flags);
	}

	imshow("image", dst);
	cout << area << " pixels were repainted\n";
}


int main( int argc, char** argv )
{
	char* filename = argc >= 2 ? argv[1] : (char*)"fruits.jpg";
	image0 = imread(filename, 1);

	if( image0.empty() )
	{
		cout << "Image empty. Usage: ffilldemo <image_name>\n";
		return 0;
	}
	help();
	image0.copyTo(image);
	cvtColor(image0, gray, COLOR_BGR2GRAY);
	mask.create(image0.rows+2, image0.cols+2, CV_8UC1);

	namedWindow( "image", 0 );
	createTrackbar( "lo_diff", "image", &loDiff, 255, 0 );
	createTrackbar( "up_diff", "image", &upDiff, 255, 0 );

	setMouseCallback( "image", onMouse, 0 );

	for(;;)
	{
		imshow("image", isColor ? image : gray);

		int c = waitKey(0);
		if( (c & 255) == 27 )
		{
			cout << "Exiting ...\n";
			break;
		}
		switch( (char)c )
		{
		case 'c':
			if( isColor )
			{
				cout << "Grayscale mode is set\n";
				cvtColor(image0, gray, COLOR_BGR2GRAY);
				mask = Scalar::all(0);
				isColor = false;
			}
			else
			{
				cout << "Color mode is set\n";
				image0.copyTo(image);
				mask = Scalar::all(0);
				isColor = true;
			}
			break;
		case 'm':
			if( useMask )
			{
				destroyWindow( "mask" );
				useMask = false;
			}
			else
			{
				namedWindow( "mask", 0 );
				mask = Scalar::all(0);
				imshow("mask", mask);
				useMask = true;
			}
			break;
		case 'r':
			cout << "Original image is restored\n";
			image0.copyTo(image);
			cvtColor(image, gray, COLOR_BGR2GRAY);
			mask = Scalar::all(0);
			break;
		case 's':
			cout << "Simple floodfill mode is set\n";
			ffillMode = 0;
			break;
		case 'f':
			cout << "Fixed Range floodfill mode is set\n";
			ffillMode = 1;
			break;
		case 'g':
			cout << "Gradient (floating range) floodfill mode is set\n";
			ffillMode = 2;
			break;
		case '4':
			cout << "4-connectivity mode is set\n";
			connectivity = 4;
			break;
		case '8':
			cout << "8-connectivity mode is set\n";
			connectivity = 8;
			break;
		}
	}

	return 0;
}
#endif

#include <iostream> // for standard I/O
#include <string>   // for strings
#include <iomanip>  // for controlling float print precision 
#include <sstream>  // string to number conversion 

#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O



using namespace std;
using namespace cv;


// images
Mat inputImg, showImg, segMask, segShowImg;

// mask
Mat fgScribbleMask, bgScribbleMask;


// user clicked mouse buttons flags
bool rButtonDown = false;
bool lButtonDown = false;
int scribbleRadius = 2;
Point prePt;
Point firstPt;


// mouse listener
static void onMouse( int event, int x, int y, int, void* )
{
	//cout << "On Mouse: (" << x << "," << y << ")" <<endl;

	switch(event)
	{
	case CV_EVENT_LBUTTONDOWN:
		if (!lButtonDown)
		{
			circle(bgScribbleMask, Point(x,y), scribbleRadius, 255, -1);
			circle(showImg, Point(x,y), scribbleRadius, CV_RGB(255,0,0), -1);
			firstPt = Point(x,y);
			lButtonDown = true;
		}
		else
		{
			circle(bgScribbleMask, Point(x,y), scribbleRadius, 255, -1);
			circle(showImg, Point(x,y), scribbleRadius, CV_RGB(255,0,0), -1);
			//line(bgScribbleMask, prePt, Point(x,y), 255, 2);
			line(showImg, prePt, Point(x,y), CV_RGB(255,0,0), 2);
			//lButtonDown = false;
		}

		break;
	case CV_EVENT_LBUTTONUP:
		prePt = Point(x,y);
		break;
	case CV_EVENT_RBUTTONDOWN:
		if (lButtonDown)
		{
			line(showImg, prePt, firstPt, CV_RGB(255,0,0), 2);
		}
		break;
	case CV_EVENT_RBUTTONUP:
		break;
	default:
		break;
	}
	/*if (event == CV_EVENT_LBUTTONDOWN)
	{
		lButtonDown = true;

	}
	else if (event == CV_EVENT_RBUTTONDOWN)
	{
		rButtonDown = true;

	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		lButtonDown = false;
	}
	else if (event == CV_EVENT_RBUTTONUP)
	{
		rButtonDown = false;
	}
	else if (event == CV_EVENT_MOUSEMOVE)
	{
		if (rButtonDown)
		{
			// scribble the background

			circle(bgScribbleMask,Point(x,y),scribbleRadius, 255,-1);
			circle(showImg,Point(x,y),scribbleRadius, CV_RGB(0,0,255),-1);

		}
		else if (lButtonDown)
		{
			// scribble the foreground

			circle(fgScribbleMask,Point(x,y),scribbleRadius, 255,-1);
			circle(showImg,Point(x,y),scribbleRadius, CV_RGB(255,0,0),-1);

			//fgScribbleMask.at<char>(y,x)=(char)255;
			// set variables using mask
			//showImg.setTo(redColorElement,fgScribbleMask);

			//showImg.at<Vec3b>(y,x)[0] = 0;
			//showImg.at<Vec3b>(y,x)[1] = 0;
			//showImg.at<Vec3b>(y,x)[2] = 255;
		}

	}*/


	imshow("Scribble Image", showImg);
	imshow("fg mask", fgScribbleMask);
	imshow("bg mask", bgScribbleMask);
}


// clear everything before closing
void destroyAll()
{
	// destroy all windows
	//destroyWindow("Input Image");
	destroyWindow("Scribble Image");


	destroyWindow("bg mask");
	destroyWindow("fg mask");
	//destroyWindow("Segmentation Mask");
	//destroyWindow("Segmentation Image");

	// clear all data
	fgScribbleMask.release();
	bgScribbleMask.release();
	inputImg.release();
	showImg.release();

	segMask.release();
	segShowImg.release();
}

// init all images/vars
int init(char * imgFileName)
{
	// Read the file
	inputImg = imread(imgFileName, CV_LOAD_IMAGE_COLOR);   
	showImg = inputImg.clone();
	segShowImg = inputImg.clone();
	lButtonDown = false;

	// Check for invalid input
	if(!inputImg.data )                              
	{
		cout <<  "Could not open or find the image: " << imgFileName << endl ;
		return -1;
	}

	// this is the mask to keep the user scribbles
	fgScribbleMask.create(2,inputImg.size,CV_8UC1);
	fgScribbleMask = 0;
	bgScribbleMask.create(2,inputImg.size,CV_8UC1);
	bgScribbleMask = 0;
	segMask.create(2,inputImg.size,CV_8UC1);
	segMask = 0;


	// Create a window for display.
	//namedWindow( "Input Image", CV_WINDOW_AUTOSIZE );
	namedWindow( "Scribble Image", CV_WINDOW_AUTOSIZE);
	namedWindow( "fg mask", CV_WINDOW_AUTOSIZE );
	namedWindow( "bg mask", CV_WINDOW_AUTOSIZE );

	// Show our image inside it.
	imshow("fg mask", fgScribbleMask);
	imshow("bg mask", bgScribbleMask);                       
	imshow( "Scribble Image", showImg );

	moveWindow("Scribble Image", 1,1);
	moveWindow("fg mask", inputImg.cols + 50, 1);
	moveWindow("bg mask", inputImg.cols + 50, 1);

	// set the callback on mouse
	setMouseCallback("Scribble Image", onMouse, 0);


	return 0;
}


int main(int argc, char *argv[])
{

	String image_name,numBinsStr,bhaSlopeStr;
	//cout<<"input Parameters:"<<endl;
	//cout<<"image name: ";
	//cin>>image_name;


	// get img name parameter
	char * imgFileName = "1.jpg";


	if (init(imgFileName)==-1)
	{
		cout <<  "Could not initialize" << endl ;
		return -1;
	}



	// Wait for a keystroke in the window
	for (;;)
	{
		char key = waitKey(0);                          
		switch (key)
		{
		case 'q':
			cout << "goodbye" << endl;
			destroyAll();
			return 0;
		case '-':
			//ËõÐ¡»­±ÊÖ±¾¶
			if (scribbleRadius > 2)
				scribbleRadius --;
			cout << "current radius is " << scribbleRadius << endl;
			break;
		case '+':
			if (scribbleRadius < 100)
				scribbleRadius ++;
			cout << "current radius is " << scribbleRadius << endl;
			break;
		case 's':
			{


				// this is where we store the results
				segMask = 0;
				inputImg.copyTo(segShowImg);
				//inputImg.copyTo(showImg);


				imwrite("bg.bmp",bgScribbleMask);

				break;

			}
		case 'r':
			{
				cout << "resetting" << endl;
				destroyAll();
				if (init(imgFileName)==-1)
				{
					cout <<  "could not initialize" << std::endl ;
					return -1;
				}
				break;
			}
		}
	}


	return 0;
}
