
#ifndef MOVE_UTILITIES_H_
#define MOVE_UTILITIES_H_

inline void printMatrixInfo(const cv::Mat& mat) {
    cv::string r;
    
    uchar depth = mat.type() & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (mat.type() >> CV_CN_SHIFT);
    
    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }
    
    r += "C";
    r += (chans+'0');
    
    printf("Matrix: %s %dx%d \n", r.c_str(), mat.cols, mat.rows );
}

#endif
