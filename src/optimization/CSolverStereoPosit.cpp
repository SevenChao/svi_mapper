#include "CSolverStereoPosit.h"
#include "exceptions/CExceptionPoseOptimization.h"
#include "vision/CMiniVisionToolbox.h"
#include "utility/CTimer.h"



const Eigen::Isometry3d CSolverStereoPosit::getTransformationWORLDtoLEFT( const Eigen::Isometry3d& p_matTransformationWORLDtoLEFTLAST,
                                                                          const Eigen::Vector3d& p_vecTranslationIMU,
                                                                          const Eigen::Isometry3d& p_matTransformationWORLDtoLEFTESTIMATE,
                                                                          const std::vector< CSolverStereoPosit::CMatch >& p_vecMeasurements )
{
    const double dTimeStartSeconds = CTimer::getTimeSeconds( );

    //ds number of points
    const std::vector< CSolverStereoPosit::CMatch >::size_type uNumberOfMeasurements = p_vecMeasurements.size( );

    //ds check if we have a sufficient number of points to optimize
    if( m_uMinimumPointsForPoseOptimization < uNumberOfMeasurements )
    {
        //ds refresh LS setup
        m_matTransformationWORLDtoLEFT     = p_matTransformationWORLDtoLEFTESTIMATE;
        m_dErrorTotalPreviousSquaredPixels = 0.0;

        //ds run least-squares maximum 100 times
        for( uint32_t uLS = 0; uLS < m_uCapIterationsPoseOptimization; ++uLS )
        {
            //ds error
            double dErrorTotalPixelsL2 = 0.0;
            std::vector< CSolverStereoPosit::CMatch >::size_type uInliersCurrent = 0;

            //ds initialize setup
            m_matH.setZero( );
            m_vecB.setZero( );

            //ds for all the points
            for( const CSolverStereoPosit::CMatch& cMatch: p_vecMeasurements )
            {
                //ds compute projection into current frame
                const CPoint3DCAMERA vecPointXYZLEFT( m_matTransformationWORLDtoLEFT*cMatch.vecPointXYZWORLD );
                if( 0.0 < vecPointXYZLEFT.z( ) )
                {
                    //ds apply the projection to the transformed point
                    const Eigen::Vector4d vecPointHomogeneous( vecPointXYZLEFT.x( ), vecPointXYZLEFT.y( ), vecPointXYZLEFT.z( ), 1.0 );
                    const Eigen::Vector3d vecABCLEFT  = m_matProjectionLEFT*vecPointHomogeneous;
                    const Eigen::Vector3d vecABCRIGHT = m_matProjectionRIGHT*vecPointHomogeneous;

                    //ds buffer c value
                    const double dCLEFT  = vecABCLEFT.z( );
                    const double dCRIGHT = vecABCRIGHT.z( );

                    //ds compute error
                    const Eigen::Vector2d vecUVLEFT( vecABCLEFT.x( )/dCLEFT, vecABCLEFT.y( )/dCLEFT );
                    const Eigen::Vector2d vecUVRIGHT( vecABCRIGHT.x( )/dCRIGHT, vecABCRIGHT.y( )/dCRIGHT );
                    const Eigen::Vector4d vecError( vecUVLEFT.x( )-cMatch.ptUVLEFT.x,
                                                    vecUVLEFT.y( )-cMatch.ptUVLEFT.y,
                                                    vecUVRIGHT.x( )-cMatch.ptUVRIGHT.x,
                                                    vecUVRIGHT.y( )-cMatch.ptUVRIGHT.y );

                    //ds current error
                    const double dErrorSquaredPixels = vecError.transpose( )*vecError;

                    //ds weight optimized points higher
                    double dWeight = 1.0;

                    //ds check if outlier
                    if( m_dMaximumErrorInlierPixelsL2 < dErrorSquaredPixels )
                    {
                        dWeight = m_dMaximumErrorInlierPixelsL2/dErrorSquaredPixels;
                    }
                    else
                    {
                        ++uInliersCurrent;
                    }
                    dErrorTotalPixelsL2 += dWeight*dErrorSquaredPixels;

                    //ds get the jacobian of the transform part
                    Eigen::Matrix< double, 4, 6 > matJacobianTransform;
                    matJacobianTransform.setZero( );
                    matJacobianTransform.block<3,3>(0,0).setIdentity( );
                    matJacobianTransform.block<3,3>(0,3) = -2*CMiniVisionToolbox::getSkew( vecPointXYZLEFT );

                    //ds jacobian of the homogeneous division
                    Eigen::Matrix< double, 2, 3 > matJacobianLEFT;
                    matJacobianLEFT << 1/dCLEFT,          0, -vecABCLEFT.x( )/( dCLEFT*dCLEFT ),
                                              0,   1/dCLEFT, -vecABCLEFT.y( )/( dCLEFT*dCLEFT );

                    Eigen::Matrix< double, 2, 3 > matJacobianRIGHT;
                    matJacobianRIGHT << 1/dCRIGHT,           0, -vecABCRIGHT.x( )/( dCRIGHT*dCRIGHT ),
                                                0,   1/dCRIGHT, -vecABCRIGHT.y( )/( dCRIGHT*dCRIGHT );

                    //ds final jacobian
                    Eigen::Matrix< double, 4, 6 > matJacobian;
                    matJacobian.setZero( );
                    matJacobian.block< 2,6 >(0,0) = matJacobianLEFT*m_matProjectionLEFT*matJacobianTransform;
                    matJacobian.block< 2,6 >(2,0) = matJacobianRIGHT*m_matProjectionRIGHT*matJacobianTransform;

                    //ds precompute transposed
                    const Eigen::Matrix< double, 6, 4 > matJacobianTransposed( matJacobian.transpose( ) );

                    //ds accumulate
                    m_matH += dWeight*matJacobianTransposed*matJacobian;
                    m_vecB += dWeight*matJacobianTransposed*vecError;
                }
            }

            //ds solve the system and update the estimate
            m_matTransformationWORLDtoLEFT = CMiniVisionToolbox::getTransformationFromVector( m_matH.ldlt( ).solve( -m_vecB ) )*m_matTransformationWORLDtoLEFT;

            //ds enforce rotation symmetry
            const Eigen::Matrix3d matRotation         = m_matTransformationWORLDtoLEFT.linear( );
            Eigen::Matrix3d matRotationSquared        = matRotation.transpose( )*matRotation;
            matRotationSquared.diagonal( ).array( )  -= 1.0;
            m_matTransformationWORLDtoLEFT.linear( ) -= 0.5*matRotation*matRotationSquared;

            //ds check if converged (descent not required, at least one inlier otherwise it drifted off)
            if( m_dConvergenceDelta > std::fabs( m_dErrorTotalPreviousSquaredPixels-dErrorTotalPixelsL2 ) )
            {
                //ds simplicity
                m_dDurationTotalSeconds += CTimer::getTimeSeconds( )-dTimeStartSeconds;

                //ds average error
                const double dErrorAveragePixelsL2 = dErrorTotalPixelsL2/uNumberOfMeasurements;

                //ds escape if not acceptable
                if( m_dMaximumErrorAveragePixelsL2 < dErrorAveragePixelsL2 && m_uMinimumInliersPoseOptimization > uInliersCurrent )
                {
                    throw CExceptionPoseOptimization( "insufficient accuracy (average error: "+std::to_string( dErrorAveragePixelsL2 )+" inliers: " + std::to_string( uInliersCurrent )+")" );
                }

                //ds compute quality identifiers
                const Eigen::Vector3d vecDeltaTranslationOptimized( m_matTransformationWORLDtoLEFT.translation( )-p_matTransformationWORLDtoLEFTLAST.translation( ) );
                const double dNormOptimizationTranslation = vecDeltaTranslationOptimized.squaredNorm( );

                //ds check translational change
                if( m_dMinimumTranslationMetersL2 > dNormOptimizationTranslation )
                {
                    //ds don't integrate translational part
                    m_matTransformationWORLDtoLEFT.translation( ) = p_matTransformationWORLDtoLEFTLAST.translation( );
                }

                //ds log resulting trajectory and delta to initial
                const Eigen::Isometry3d matTransformationLEFTtoWORLD( m_matTransformationWORLDtoLEFT.inverse( ) );
                const double dOptimizationRISK = ( matTransformationLEFTtoWORLD.translation( )-p_matTransformationWORLDtoLEFTESTIMATE.inverse( ).translation( )-p_vecTranslationIMU ).squaredNorm( );

                //ds check optimization risk (relate computed transformation to prior -> if they differ too much we discard the result)
                if( m_dMaximumRISK < dOptimizationRISK )
                {
                    throw CExceptionPoseOptimization( "inconsistent with prior (HIGH RISK: " + std::to_string( dOptimizationRISK ) + ")" );
                }

                //ds return with pose if still here - all checks passed
                return m_matTransformationWORLDtoLEFT;
            }
            else
            {
                m_dErrorTotalPreviousSquaredPixels = dErrorTotalPixelsL2;
            }
        }

        //ds system did not converge
        m_dDurationTotalSeconds += CTimer::getTimeSeconds( )-dTimeStartSeconds;
        throw CExceptionPoseOptimization( "system did not converge" );
    }
    else
    {
        m_dDurationTotalSeconds += CTimer::getTimeSeconds( )-dTimeStartSeconds;
        throw CExceptionPoseOptimization( "insufficient number of points: " + std::to_string( uNumberOfMeasurements ) );
    }
}
