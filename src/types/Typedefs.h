#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv/cv.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>
#include <cassert>
#include <vector>
#include <map>



enum EPlaybackMode
{
    ePlaybackInteractive,
    ePlaybackStepwise,
    ePlaybackBenchmark
};



//typedef Eigen::Vector3d CPoint2DNormalized;
typedef Eigen::Vector3d CPoint2DHomogenized;
typedef Eigen::Vector3d CPoint2DInCameraFrameHomogenized;
typedef Eigen::Vector4d CPoint3DHomogenized;
typedef Eigen::Vector2d CPoint2DInCameraFrame;
typedef Eigen::Vector3d CPoint3DCAMERA;
typedef Eigen::Vector3d CPoint3DWORLD;
typedef Eigen::Vector4d CPoint3DInWorldFrameHomogenized;
typedef cv::Scalar      CColorCodeBGR;
//typedef cv::Matx< uchar, 1, 64 > CDescriptor;
typedef cv::Mat         CDescriptor;
typedef uint64_t        UIDLandmark;
typedef uint64_t        UIDDescriptor;
typedef uint64_t        UIDDetectionPoint;
typedef uint64_t        UIDKeyFrame;
typedef uint64_t        UIDCloud;
typedef uint64_t        UIDDescriptorPoint3D;
typedef uint64_t        UIDFrame;
typedef Eigen::Matrix< double, 3, 4 > MatrixProjection;
typedef Eigen::Matrix< double, 1, 3 > Vector3dT;
typedef Eigen::Vector3d CLinearAccelerationIMU;
typedef Eigen::Vector3d CLinearAccelerationLEFT;
typedef Eigen::Vector3d CAngularVelocityIMU;
typedef Eigen::Vector3d CAngularVelocityLEFT;
typedef Eigen::Vector3d CAngularVelocityWORLD;
typedef Eigen::Vector3d CLinearAccelerationWORLD;

typedef Eigen::Matrix< double, 4, 4 > EigenMatrix4d;
typedef Eigen::Matrix< double, 6, 1 > EigenVector6d;
typedef Eigen::Matrix< double, 6, 6 > EigenMatrix6d;

/*ds multiple descriptors
template< uint32_t uRows >
using CDescriptors = cv::Matx< uchar, uRows, 64 >;*/
typedef cv::Mat CDescriptors;



#endif //TYPEDEFS_H
