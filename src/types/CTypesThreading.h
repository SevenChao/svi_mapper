#ifndef TYPESTHREADING_H
#define TYPESTHREADING_H

#include <mutex>
#include <condition_variable>
#include <vector>

#include "CKeyFrame.h"



struct CHandleMapping
{
    CHandleMapping( ): vecKeyFramesToAdd( std::vector< CKeyFrame* >( 0 ) )
    {
        vecKeyFramesToAdd.clear( );
    }
    ~CHandleMapping( )
    {
        vecKeyFramesToAdd.clear( );
    }

    //ds communication
    std::mutex cMutex;
    std::condition_variable cConditionVariable;
    bool bActive               = true;
    bool bTerminationRequested = false;
    bool bWaitingForOptimizationReception = false;

    //ds data
    std::vector< CKeyFrame* > vecKeyFramesToAdd;
};

//ds optimization
struct COptimizationRequest
{
    //ds data to be set
    UIDFrame uFrame;
    std::vector< CKeyFrame* >::size_type uIDBeginKeyFrame;
    std::vector< CLandmark* >::size_type uIDBeginLandmark;
    Eigen::Vector3d vecTranslationToG2o;
    std::vector< CKeyFrame* >::size_type uLoopClosureKeyFrames;
};

struct CHandleOptimization
{
    CHandleOptimization( )
    {

    }
    ~CHandleOptimization( )
    {

    }

    //ds communication
    std::mutex cMutex;
    std::condition_variable cConditionVariable;
    bool bActive               = true;
    bool bTerminationRequested = false;
    bool bRequestProcessed     = true; //ds initial request is none and therefore always processed

    //ds data
    COptimizationRequest cRequest;
};

//ds landmark sharing: owned by Tracker
struct CHandleLandmarks
{
    CHandleLandmarks( ): vecLandmarks( std::make_shared< std::vector< CLandmark* > >( ) )
    {
        vecLandmarks->clear( );
    }
    ~CHandleLandmarks( )
    {
        //ds lock - blocking - RAII
        std::lock_guard< std::mutex > cLockGuard( cMutex );

        //ds deallocation count
        std::vector< CLandmark* >::size_type uNumberOfLandmarksDeallocated = 0;

        //ds free all landmarks
        for( const CLandmark* pLandmark: *vecLandmarks )
        {
            //ds write final state to file before deleting
            //CLogger::CLogLandmarkFinal::addEntry( pLandmark );

            //ds save optimized landmarks to separate file
            if( pLandmark->bIsOptimal && 1 < pLandmark->uNumberOfKeyFramePresences )
            {
                //CLogger::CLogLandmarkFinalOptimized::addEntry( pLandmark );
            }

            assert( 0 != pLandmark );
            delete pLandmark;
            ++uNumberOfLandmarksDeallocated;
        }

        //ds info TODO compute real memory usage
        std::printf( "<CHandleLandmarks>(~CHandleLandmarks) dealloacted landmarks: %lu/%lu (%.0fMB)\n", uNumberOfLandmarksDeallocated, vecLandmarks->size( ), vecLandmarks->size( )*sizeof( CLandmark )/1e6 );
        vecLandmarks->clear( );
    }

    std::mutex cMutex;
    std::shared_ptr< std::vector< CLandmark* > > vecLandmarks;
};

//ds key frame sharing: owned by Mapper
struct CHandleKeyFrames
{
    CHandleKeyFrames( ): vecKeyFrames( std::make_shared< std::vector< CKeyFrame* > >( ) )
    {
        vecKeyFrames->clear( );
    }
    ~CHandleKeyFrames( )
    {
        //ds lock - blocking - RAII
        std::lock_guard< std::mutex > cLockGuard( cMutex );

        //ds info
        std::vector< CKeyFrame* >::size_type uNumberOfKeyFramesDeallocated = 0;

        //ds free keyframes
        for( const CKeyFrame* pKeyFrame: *vecKeyFrames )
        {
            assert( 0 != pKeyFrame );
            delete pKeyFrame;
            ++uNumberOfKeyFramesDeallocated;
        }

        //ds info TODO compute real memory usage
        std::printf( "<CHandleKeyFrames>(~CHandleKeyFrames) dealloacted key frames: %lu/%lu (%.0fMB)\n", uNumberOfKeyFramesDeallocated, vecKeyFrames->size( ), vecKeyFrames->size( )*sizeof( CKeyFrame )/1e6 );
        vecKeyFrames->clear( );
    }

    std::mutex cMutex;
    std::shared_ptr< std::vector< CKeyFrame* > > vecKeyFrames;
};

//ds map update broadcasted by optimizer
struct CHandleMapUpdate
{
    //ds communication
    std::mutex cMutex;
    bool bAvailable       = false;
    bool bMapperReceived  = false; //ds the mapper handles the update first
    bool bTrackerReceived = false; //ds then the tracker briefly integrates the update and moves on

    //ds data
    CKeyFrame* pKeyFrameOptimizedLAST = 0;
};

#endif //TYPESTHREADING_H
