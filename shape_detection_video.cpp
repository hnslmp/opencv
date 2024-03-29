#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

Mat image; Mat imageS; Mat result; Mat canny_output; Mat imgHSV; Mat imgThresholded;
Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

int thresh = 100;

int max_thresh = 255;
int min_area = 600;
int roi_y = 0;
int min_x = 9999;
RNG rng(12345);
int largest_area=0;
int largest_contour_index=0;

int count_red = 0;

const char* window_name = "Shape Detection";

void contour_callback (int, void*);

int main (int argc, char** argv){
    
    VideoCapture cap(0); 

    // Check if camera opened successfully
    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    int iLowH = 0;
    int iHighH = 179;

    int iLowS = 0; 
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    //Create trackbars in "Control" window
    namedWindow(window_name, CV_WINDOW_AUTOSIZE);
    cvCreateTrackbar("LowH", window_name, &iLowH, 255); //Hue (0 - 179)
    cvCreateTrackbar("HighH", window_name, &iHighH, 255);


    cvCreateTrackbar("LowS", window_name, &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", window_name, &iHighS, 255);

    cvCreateTrackbar("LowV", window_name, &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", window_name, &iHighV, 255);

    while(1){

        cap >> image;
        flip(image, image, +1);

        if (image.empty())
            break;

        //resize(image, image, Size(), 0.1, 0.1 );

        cvtColor (image,imgHSV, CV_BGR2HSV);
        blur( imgHSV, imgHSV, Size(3,3) );

        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      
        //morphological opening (remove small objects from the foreground)

        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

        //morphological closing (fill small holes in the foreground)
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        //Find Contours of shape
        Canny(imgThresholded, canny_output, thresh, thresh*2, 3);
        findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );

        for (int i=0; i < contours.size();i++ ){
            
            
             //Find approx of the polyshape
            vector<Point> approx;
            vector<Point> cnt = contours.at(i);
            double area = contourArea(cnt);
            approxPolyDP(cnt, approx, 5, true);

            if(area>largest_area){
                largest_area=area;
                largest_contour_index=i;                //Store the index of largest contour
            }
            
            //Find X and Y Center of Contour
            Moments m = moments(approx,true);
            Point p(m.m10/m.m00, m.m01/m.m00);
            int x = p.x;
            int y = p.y;

            //Make a region of interest 
            //if (area > min_area && y > roi_y){}
            if (x > min_x && y > roi_y){
                //Search Shape
                if(3 <= approx.size() <= 6){
                    count_red += 1;
                    cout << "Detected" << endl;
                    //Find the most left circle
                    if (x < min_x){
                        min_x = x;
                    }
                } 
                
                else{
                    cout << "Not Detected" << endl;
                } 
            }         
   
        } 
        drawContours( image, contours, largest_contour_index, color, 2, 8, hierarchy, 0, Point() );
        imshow (window_name, imgThresholded);
        imshow ("Original",image);
        // Press  ESC on keyboard to exit
        char c=(char)waitKey(25);
        if(c==27) break;
    }
  
  // When everything done, release the video capture object
  cap.release();
 
  // Closes all the frames
  destroyAllWindows();
  return 0;
}