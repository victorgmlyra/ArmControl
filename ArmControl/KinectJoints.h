#pragma once
#ifndef KinectJoints_h	
#define KinectJoints_h

#include "resource.h"
#include <Kinect.h>

class KinectJoints
{
public:
	KinectJoints();		
	~KinectJoints();

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	int                     Run();

	double					getAngle();
private:
	INT64                   m_nStartTime;
	INT64                   m_nLastCounter;
	double                  m_fFreq;
	INT64                   m_nNextStatusTime;
	DWORD                   m_nFramesSinceUpdate;

	// Angle in right elbow
	double					angle;

	// Current Kinect
	IKinectSensor*          m_pKinectSensor;
	ICoordinateMapper*      m_pCoordinateMapper;

	// Body reader
	IBodyFrameReader*       m_pBodyFrameReader;

	/// <summary>
	/// Main processing function
	/// </summary>
	void                    Update();

	/// <summary>
	/// Initializes the default Kinect sensor
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT                 InitializeDefaultSensor();

	/// <summary>
	/// Handle new body data
	/// <param name="nTime">timestamp of frame</param>
	/// <param name="nBodyCount">body data count</param>
	/// <param name="ppBodies">body data in frame</param>
	/// </summary>
	void                    ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies);

	// Get right elbow angle
	void					elbowAngle(Joint joints[]);
};

#endif

