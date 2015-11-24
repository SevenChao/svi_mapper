#ifndef CTRIANGULATOR_H
#define CTRIANGULATOR_H

#include "vision/CStereoCamera.h"

class CTriangulator
{

//ds ctor/dtor
public:

    CTriangulator( const std::shared_ptr< CStereoCamera > p_pStereoCamera,
                   const std::shared_ptr< cv::DescriptorExtractor > p_pExtractor,
                   const std::shared_ptr< cv::DescriptorMatcher > p_pMatcher,
                   const float& p_fMatchingDistanceCutoff );
    ~CTriangulator( );

//ds defines
public:

    static constexpr float fMinimumSearchRangePixels = 60.0;

//ds members
private:

    //ds cameras
    const std::shared_ptr< CStereoCamera > m_pCameraSTEREO;
    const std::shared_ptr< CPinholeCamera > m_pCameraLEFT;
    const std::shared_ptr< CPinholeCamera > m_pCameraRIGHT;

    //ds matching
    const std::shared_ptr< cv::DescriptorExtractor > m_pExtractor;
    const std::shared_ptr< cv::DescriptorMatcher > m_pMatcher;
    const float m_fMatchingDistanceCutoff;

    //ds triangulation
    const uint32_t m_uLimitedSearchRangeToLEFT  = 50;
    const uint32_t m_uLimitedSearchRangeToRIGHT = 10;
    const uint32_t m_uLimitedSearchRange;
    const uint32_t m_uAdaptiveSteps;
    const uint32_t m_uMinimumSearchRangePixels = 50;

//ds api
public:

    //ds triangulation methods
    const CPoint3DCAMERA getPointTriangulatedFull( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CPoint3DCAMERA getPointTriangulatedLimited( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CPoint3DCAMERA getPointTriangulatedAdaptive( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;

    const CMatchTriangulation getPointTriangulatedCompactInLEFT( const cv::Mat& p_matImageLEFT, const cv::KeyPoint& p_cKeyPointRIGHT, const CDescriptor& p_matReferenceDescriptorRIGHT ) const;
    const CMatchTriangulation getPointTriangulatedCompactInRIGHT( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CMatchTriangulation getPointTriangulatedInRIGHT( const cv::Mat& p_matImageRIGHT,
            const float& p_fUTopLeft,
            const float& p_fVTopLeft,
            const float& p_fKeyPointSizePixels,
            const cv::Point2f& p_ptUVLEFT,
            const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CMatchTriangulation getPointTriangulatedInLEFT( const cv::Mat& p_matImageLEFT,
            const float& p_fUTopLeft,
            const float& p_fVTopLeft,
            const float& p_fKeyPointSizePixels,
            const cv::Point2f& p_ptUVRIGHT,
            const CDescriptor& p_matReferenceDescriptorRIGHT ) const;

    //ds testing
    const CPoint3DCAMERA getPointTriangulatedLimitedSVDLS( cv::Mat& p_matDisplayRIGHT, const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CPoint3DCAMERA getPointTriangulatedLimitedQRLS( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;
    const CPoint3DCAMERA getPointTriangulatedLimitedSVDDLT( const cv::Mat& p_matImageRIGHT, const cv::KeyPoint& p_cKeyPointLEFT, const CDescriptor& p_matReferenceDescriptorLEFT ) const;

private:

    friend class CMatcherEpipolar;
    friend class CFundamentalMatcher;
};

#endif //#define CTRIANGULATOR_H
