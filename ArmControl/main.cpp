#include <iostream>
#include <thread>
#include "server.h"
#include "KinectJoints.h"

#define STEP_DISTANCE 1								// Step for moving to destination
#define NUM_SAVES 2									// Number of saves that will be utilized
const int limit[6] = { 0, 80, 140, 0, 140, 0 };		// Limits for robot joints

using namespace std;

// Declaring functions
void keyPress(int joint[], int& jointChoice, bool& keyCam, bool& play, int savePoints[][6]);
void movetoDestination(int& angle, int joints[], int jointChoice);
void playRecording(int joint[], int savePoints[][6]);
bool decideWithLimit(int& angle, int joints[], int jointChoice);

int main()
{
	int angle = 0, jointChoice = 0;						// Variables for arm-kinect manipulation
	bool keyCam = false, play = false;					// keyCam - false for kinect, true for keyboard
	int joint[6] = { 0, 0, 0, 0, 0, 0 };				// Value of robot joints
	int savePoints[5][6] = { 0 };						// Joint values for recording

	Server server("127.0.0.1", 8889);					// Create server class (ip, port)
	thread t1(&Server::initServer, ref(server), joint);	// Init server in a thread
	
	// Kinect
	KinectJoints kinect;
	thread t2(&KinectJoints::Run, ref(kinect));

	// Keyboard
	thread t3(keyPress, ref(joint), ref(jointChoice), ref(keyCam), ref(play), ref(savePoints));
	
	while (true)
	{
		angle = (int)(kinect.getAngle());
		if (!keyCam && !play) {
			movetoDestination(angle, joint, jointChoice);
		}
		if (play) {
			playRecording(joint, savePoints);
		}
	}
	
	t1.join();
	t2.join();
	t3.join();
	return 0;
}

// Move arm to destination angle
void movetoDestination(int& angle, int joints[], int jointChoice)
{
	int anglef = angle;						// Destination
	int angle0 = joints[jointChoice];		// Actual
	int distance = anglef - angle0;

	int step = STEP_DISTANCE;
	if (distance < step)
		step = 1;
	else
		step = STEP_DISTANCE;

	// Move to destination without limitations
	if (jointChoice == 0 || jointChoice == 3 || jointChoice == 5)
	{
		// Closer path to destination
		if (distance > 0) {
			if (distance > 180)
				joints[jointChoice] -= step;
			else
				joints[jointChoice] += step;
		}
		else if (distance < 0) {
			if (distance < -180)
				joints[jointChoice] += step;
			else
				joints[jointChoice] -= step;
		}
	}
	// Move to destination with limitations
	else
	{
		if (distance > 0) {
			if (distance > 180) {
				if(decideWithLimit(angle, joints, jointChoice))
					joints[jointChoice] -= step;
				else
					joints[jointChoice] += step;
			}	
			else {
				joints[jointChoice] += step;
			}		
		}
		else if (distance < 0) {
			if (distance < -180) {
				if (decideWithLimit(angle, joints, jointChoice))
					joints[jointChoice] += step;
				else
					joints[jointChoice] -= step;
			}
			else {
				joints[jointChoice] -= step;
			}
		}
		if (joints[jointChoice] >= limit[jointChoice])
			joints[jointChoice] = limit[jointChoice];
		if (joints[jointChoice] <= -limit[jointChoice])
			joints[jointChoice] = -limit[jointChoice];
	}

	// Going through -180
	if (joints[jointChoice] >= 180)
		joints[jointChoice] = -180;
	if (joints[jointChoice] <= -181)
		joints[jointChoice] = 179;
	//cout << joints[jointChoice] << endl;
	Sleep(1);
}

bool decideWithLimit(int& angle, int joints[], int jointChoice)
{
	if (joints[jointChoice] >= 0)
	{
		if (angle > joints[jointChoice])
		{
			return true;
		}
		else
		{
			cout << "oi" << endl;
			return false;
		}
	}
	else
	{
		if (angle < joints[jointChoice])
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

// Function to control de robot by keyboard
void keyPress(int joint[], int& jointChoice, bool& keyCam, bool& play, int savePoints[][6])
{
	static char keys[13] = "AQSWDEFRGTHY";
	while (true)
	{
		// Select joint to use with kinect
		if (GetKeyState('1') & 0x8000) {
			jointChoice = 0;
		}
		if (GetKeyState('2') & 0x8000) {
			jointChoice = 1;
		}
		if (GetKeyState('3') & 0x8000) {
			jointChoice = 2;
		}
		if (GetKeyState('4') & 0x8000) {
			jointChoice = 3;
		}
		if (GetKeyState('5') & 0x8000) {
			jointChoice = 4;
		}
		if (GetKeyState('6') & 0x8000) {
			jointChoice = 5;
		}

		// Choose keyboard or camera
		if (GetKeyState('K') & 0x8000) {
			keyCam = true;
		}
		if (GetKeyState('C') & 0x8000) {
			keyCam = false;
		}
		// Control with keyboard
		if (keyCam && !play)
		{
			int angle;
			for (int i = 0; i < 6; i++)
			{

				if (GetKeyState(keys[2 * i]) & 0x8000) {
						angle = joint[i] + 1;
						movetoDestination(angle, joint, i);
				}
				if (GetKeyState(keys[(2 * i) + 1]) & 0x8000) {
					angle = joint[i] - 1;
					movetoDestination(angle, joint, i);
				}
			}
		}

		// Save current position
		if (!play)
		{
			if (GetKeyState(VK_F1) & 0x8000) {
				copy(joint, joint + 6, savePoints[0]);
			}
			if (GetKeyState(VK_F2) & 0x8000) {
				copy(joint, joint + 6, savePoints[1]);
			}
			if (GetKeyState(VK_F3) & 0x8000) {
				copy(joint, joint + 6, savePoints[2]);
			}
			if (GetKeyState(VK_F4) & 0x8000) {
				copy(joint, joint + 6, savePoints[3]);
			}
			if (GetKeyState(VK_F5) & 0x8000) {
				copy(joint, joint + 6, savePoints[4]);
			}
		}

		// Play Recording in loop
		if (GetKeyState('P') & 0x8000) {
			play = true;
		}
		// Stop recording loop
		if (GetKeyState('O') & 0x8000) {
			play = false;
		}
		Sleep(4);
	}
}

// Loop recording
void playRecording(int joint[], int savePoints[][6])
{
	static int point = 0;

	// Loop
	for (int i = 0; i < 6; i++)
	{
		movetoDestination(savePoints[point][i], joint, i);
	}

	// Verify if arrived to destination
	if (equal(joint, joint + 6, savePoints[point])) {
		cout << "Got to position " << point << endl;
		point++;
	}
	if (point == NUM_SAVES) {
		point = 0;
	}
}