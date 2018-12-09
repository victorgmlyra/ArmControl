#include "KinectJoints.h"
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include <iostream>
#include <math.h>

#define PI 3.14159265

/// <summary>
/// Constructor
/// </summary>
KinectJoints::KinectJoints() :
	m_nStartTime(0),
	m_nLastCounter(0),
	m_nFramesSinceUpdate(0),
	m_fFreq(0),
	m_nNextStatusTime(0LL),
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL)
{
	LARGE_INTEGER qpf = { 0 };
	this->angle = 0;
	if (QueryPerformanceFrequency(&qpf))
	{
		m_fFreq = double(qpf.QuadPart);
	}
}

/// <summary>
/// Destructor
/// </summary>
KinectJoints::~KinectJoints()
{
	// done with body frame reader
	SafeRelease(m_pBodyFrameReader);

	// done with coordinate mapper
	SafeRelease(m_pCoordinateMapper);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	SafeRelease(m_pKinectSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int KinectJoints::Run()
{
	this->InitializeDefaultSensor();
	// Main message loop
	while (1)
	{
		Update();
	}

	return 1;
}

double KinectJoints::getAngle()
{
	return this->angle;
}

/// <summary>
/// Main processing function
/// </summary>
void KinectJoints::Update()
{
	if (!m_pBodyFrameReader)
	{
		return;
	}
	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;

		hr = pBodyFrame->get_RelativeTime(&nTime);

		IBody* ppBodies[BODY_COUNT] = { 0 };

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		if (SUCCEEDED(hr))
		{
			ProcessBody(nTime, BODY_COUNT, ppBodies);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}

	SafeRelease(pBodyFrame);
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectJoints::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		std::cerr << "Error initializing default sensor!" << std::endl;
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		SafeRelease(pBodyFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cerr << "No ready Kinect found!" << std::endl;
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Handle new body data
/// <param name="nTime">timestamp of frame</param>
/// <param name="nBodyCount">body data count</param>
/// <param name="ppBodies">body data in frame</param>
/// </summary>
void KinectJoints::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
	HRESULT hr;
	Joint joints[JointType_Count];

	for (int i = 0; i < nBodyCount; ++i) // Only one person in camera
	{
		IBody* pBody = ppBodies[i];
		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				// Find Joint points here 
				//Joint joints[JointType_Count]; 
				HandState leftHandState = HandState_Unknown;
				HandState rightHandState = HandState_Unknown;

				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				hr = pBody->GetJoints(_countof(joints), joints);
				// TODO
				elbowAngle(joints);
			}
		}
	}

	// Get fps
	if (!m_nStartTime)
	{
		m_nStartTime = nTime;
	}

	double fps = 0.0;

	LARGE_INTEGER qpcNow = { 0 };
	if (m_fFreq)
	{
		if (QueryPerformanceCounter(&qpcNow))
		{
			if (m_nLastCounter)
			{
				m_nFramesSinceUpdate++;
				fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
			}
		}
	}
	m_nLastCounter = qpcNow.QuadPart;
	m_nFramesSinceUpdate = 0;
	//std::cout << fps << std::endl;
}

// Get right elbow angle
void KinectJoints::elbowAngle(Joint joints[])
{
	D2D1_POINT_2F wrist;
	D2D1_POINT_2F elbow;
	D2D1_POINT_2F shoulder;
	elbow.x = joints[JointType_ElbowRight].Position.X;
	elbow.y = joints[JointType_ElbowRight].Position.Y;

	// Translocate de coordinate system
	wrist.x = joints[JointType_WristRight].Position.X - elbow.x;
	wrist.y = joints[JointType_WristRight].Position.Y - elbow.y;
	shoulder.x = joints[JointType_ShoulderRight].Position.X - elbow.x;
	shoulder.y = joints[JointType_ShoulderRight].Position.Y - elbow.y;

	// Rotate the coodinate system
	float theta = atan2(wrist.y, wrist.x);
	D2D1_POINT_2F anglePoint;
	anglePoint.x = shoulder.x * cos(theta) + shoulder.y * sin(theta);
	anglePoint.y = -shoulder.x * sin(theta) + shoulder.y * cos(theta);

	// Get elbow angle
	angle = atan2(anglePoint.y, anglePoint.x) * 180 / PI;
	//std::cout << angle << std::endl;
}