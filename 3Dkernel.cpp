/*When dealing with 3D scans of images
 * a 3D kernel to enhance images is probably a better
 * idea. That way a z axis of data is added instead
 * of local neighborhood data in x and y*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <dirent.h>

using namespace cv;


/*Define kernel*/
const int kWidth = 3;
const int kHeight = 3;
const int kDepth = 3;

double arr[kWidth][kHeight][kDepth] = { -1, -2, -1,
					-2, 15, -2,
					-1, -2, -1,
							-4, 1, -4,
							1 , 20, 1,
							-4, 1, -4,
					-1, -2, -1,
					-2, 15, -2,
					-1, -2, -1};



/*Uses x, y, z (dist, width, height) to enhance image*/
Mat *filteredImage(Mat image[], int count) {
	
 	//Mat new_image = Mat::zeros( src.size(), src.type() );
	double tmpValue;
	double summation = 0;
	Mat *newImage = new Mat[count-2*kDepth];
	std::cout << count-2*kDepth << std::endl;
	for (int d = 0; d < kDepth; ++d) {
		for (int w = 0; w < kWidth; ++w) {
			for (int h = 0; h < kHeight; ++h) {
				summation += arr[w][h][d];
			}
		}
	}


    /*Normalized the curve*/
	for (int z = kDepth; z < count-kDepth; z++ ) {
		newImage[z-kDepth] = Mat::zeros(image[z].size(), image[z].type());
 		for (int y = kHeight; y < image[z].rows-kHeight; y++ ) {
    			for( int x = kWidth; x < image[z].cols-kWidth; x++ ) {
				double value = sqrt(pow((x-(image[z].cols/2)),2)+pow((y-(image[z].rows/2)),2));
				//double value = 0;
				if (value > 525) {
					newImage[z-kDepth].at<unsigned char>(Point(x,y))=0;	
				} else {
					/*if (image[z].at<unsigned char>(Point(x,y)) > 30) {
					
					} else {*/	
						tmpValue = 0;
						
						for (int d = 0; d < kDepth; d++) {
							for (int w = 0; w < kWidth; w++) {
								for (int h = 0; h < kHeight; h++) {
									tmpValue += (arr[w][h][d])*image[z+(d-kDepth/2)].at<unsigned char>(Point(x+(w-kWidth/2),y+(h-kHeight/2)));
								}
							}
						}
					
                        /*thresehold of pixel value to consider on*/
						if (tmpValue < 150) {
							newImage[z-kDepth].at<unsigned char>(Point(x,y)) = 0;
						} else if (tmpValue > 255) {
							newImage[z-kDepth].at<unsigned char>(Point(x, y)) = 255;
						} else {
							newImage[z-kDepth].at<unsigned char>(Point(x, y)) = tmpValue;
						}
					//}
				}
			}
		}
		imshow("Testing", newImage[z-kDepth]);
		if(waitKey(1) >= 0) break;
	}
	return newImage;
}

struct files {
	char **f;
	char **fOut;
	int count;
};

/*Collects files in the correct z direction*/
void funFiles(char *charDir, struct files *f) {

	DIR *dir;
	struct dirent *ent;
	f->f = (char **)malloc(0);
	f->fOut = (char **)malloc(0);
	f->count = 0;
	int i = 1;
	char tmpC[1000];
	char realTemp;
	if ((dir = opendir (charDir)) != NULL) {
  	/* print all the files and directories within directory */
  		while ((ent = readdir (dir)) != NULL) {
			if (strstr(ent->d_name, ".tif")) {
				f->f = (char **)realloc(f->f, sizeof(char *)*i);
				f->fOut = (char **)realloc(f->fOut, sizeof(char *)*i);

				sprintf(tmpC, "%s%s", charDir, ent->d_name);
				f->f[i-1]=strdup(tmpC);

				sprintf(tmpC, "%s", ent->d_name);
				f->fOut[i-1]=strdup(tmpC);
				
				i++;
				f->count++;
			}
  		}
  		closedir (dir);
	} else {
		printf("BAD DIR NAME\n");
		exit(-1);
  		/* could not open directory */
  	//	return EXIT_FAILURE;
	}
}

int main( int argc, char** argv ) {
	/// Read image given by user
	struct files *allFiles = (struct files*)malloc(sizeof(struct files));
 	funFiles(argv[1], allFiles);
	Mat *image = new Mat[allFiles->count];
	for (int i = 0; i < allFiles->count-1; i++) {
		image[i] = imread(allFiles->f[i], CV_LOAD_IMAGE_GRAYSCALE);
 		for (int y = 0; y < image[i].rows; y++ ) {
    			for( int x = 0; x < image[i].cols; x++ ) {
				double value = sqrt(pow((x-(image[i].cols/2)),2)+pow((y-(image[i].rows/2)),2));
				if (value > 525) {
					image[i].at<unsigned char>(Point(x,y))=0;	
				}
				if (image[i].at<unsigned char>(Point(x,y)) < 25) {
					image[i].at<unsigned char>(Point(x,y))=0;	
				}	
			}
		}
	}
	
    /*Shows the image*/
	namedWindow("Testing",1);
	namedWindow("Original", 1);
	Mat *new_image = filteredImage(image, allFiles->count);
	
	Size size(700, 700);
	int i = 0;
	for(int i = 0; i < allFiles->count-kDepth/2;i++) {
		imshow("Testing", new_image[i]);
		imshow("Original", image[i]);
		imwrite(allFiles->fOut[i], new_image[i]);
		if(waitKey(1) >= 0) break;
	}

 	return 0;
}
