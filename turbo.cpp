#include <iostream>
#include <Windows.h>
#include <thread>
using namespace std;

// declarations and definitions

double targetFPS = 50.0;															// target frames per second; set to target application's input polling rate
double targetGranularity = 8.0;														// target number of times to poll timer per frame at target frame rate
double sleepLength = (1000000.0/(targetGranularity*targetFPS));						// rough time between timing loops

LARGE_INTEGER ticksPerSecond;														// timer frequency, defined by QueryPerformanceFrequency()
LARGE_INTEGER tick;																	// tick to get on every loop for timing
double tickFrames;																	// the above in target frame lengths
double nextEntryTime = 0.0;															// for time comparison with previous loop

bool mashHeldLast = false;															// whether the space-mashing function key was held in the last loop

bool leftHeldLast = false;															// whether the left skip-walk function key was held in the last loop
unsigned long long leftHeldFrames;													// length of time the left skip-walk function key was held for
bool targetLeftState;

bool rightHeldLast = false;															// whether the right skip-walk function key was held in the last loop
unsigned long long rightHeldFrames;													// length of time the right skip-walk function key was held for
bool targetRightState;

bool alternateHeldLast = false;														// whether the arrow-key-alternating function key was held in the last loop
bool goingRight = false;															// whether currently going right

HKL kbLayout = GetKeyboardLayout(0);

namespace turbo
{
	void SendKeyboardInput(WORD code, bool isKeyUp = false, bool isScanCode = false)
	{
		INPUT inputToSend;															// input to send using SendInput()
		inputToSend.type = INPUT_KEYBOARD;											// 0 == INPUT_MOUSE; 1 == INPUT_KEYBOARD; 2 == INPUT_HARDWARE
		inputToSend.ki.wVk = (isScanCode ? 0 : code);								// virtual key code to send
		inputToSend.ki.wScan = (isScanCode ? code : 0);								// hardware scan code to send
		inputToSend.ki.dwFlags = (isKeyUp ? KEYEVENTF_KEYUP : 0) + (isScanCode ? KEYEVENTF_SCANCODE : 0);// add KEVENTF_KEYUP for keyup; 0 for keydown; add KEYEVENTF_SCANCODE if it's a scancode instead of virtual-key code
		inputToSend.ki.time = 0;													// timestamp for the event, in milliseconds		
		inputToSend.ki.dwExtraInfo = 0;												// some additional crap; idk
		SendInput(1, &inputToSend, sizeof(INPUT));
	}

	bool KeyIsPressed(WORD code, bool isScanCode = false)
	{
		if(isScanCode)
		{
			code = MapVirtualKeyEx(code,MAPVK_VSC_TO_VK_EX,kbLayout);
		}
		return GetKeyState(code) & 0x8000;
	}

	void ReleaseKey(WORD code)
	{
		if (KeyIsPressed(code))
		{
			SendKeyboardInput(code, true);
		}
	}

	void quit(int code)
	{
		cout << "quitting..." << endl;
		std::exit(code);
	}
}

using namespace turbo;

int main()
{

	if(!QueryPerformanceFrequency(&ticksPerSecond))									// QueryPerformanceFrequency() execution and error check
	{
		cout << "Error: QueryPerformanceFrequency() returned false.  This system has no" << endl
			 << "high-resolution timer." << endl;
		quit(EXIT_FAILURE);
	}

	// user instructions

	cout << " input         | effect"															<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " alt (either)  | mash space repeatedly, re-pressing each frame"					<< endl
		 << " numpad 0      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 4      | flip-walk left (left 9 frames, right 1 frame, repeat)"			<< endl
		 << " qwerty Z      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 5      | alternate left and right arrow keys each frame"					<< endl
		 << " qwerty X      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 6      | flip-walk right (right 9 frames, left 1 frame, repeat)"			<< endl
		 << " qwerty C      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " ctrl+break    | quit"																<< endl
		 << " ctrl+qwerty Q |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 <<																						endl
		 << "debug output"																		<< endl
		 << "------------"																		<< endl
		 << "target frame rate is " << targetFPS << " Hz"										<< endl
		 << "ticksPerSecond is " << ticksPerSecond.QuadPart										<< endl
		 << "polling timer " << targetGranularity << " times per frame (<" << targetFPS*targetGranularity << " Hz)" << endl;

	QueryPerformanceCounter(&tick);															// time since system start in CPU cycles, grabbed here for synchronization
	tickFrames = (double)tick.QuadPart*targetFPS/(double)ticksPerSecond.QuadPart;			// convert to number of frames at target frame rate
	nextEntryTime = tickFrames;

	// main loop

	do
	{
		this_thread::sleep_for(chrono::microseconds((long long)sleepLength));				// sleep between checks (number of times per frame dictated by fps and granularity)

		QueryPerformanceCounter(&tick);														// time since system start in CPU cycles
		tickFrames = ((double)tick.QuadPart*targetFPS/(double)ticksPerSecond.QuadPart);		// convert to number of frames at target frame rate

		// this line is the gateway, preventing the loop from entering the input checks unless an entire frame at the target frame rate has passed
		//cout << "tickFrames is " << to_string(tickFrames) << endl;
		if (!(tickFrames > nextEntryTime)) continue;
		//cout << "frame " << to_string((int)floor(nextEntryTime)) << endl;
		//cout << "entered at    " << to_string(tickFrames) << endl;
		nextEntryTime = nextEntryTime + 1.0;												// wait 1 frame before next input checks
		// begin input checks
		if(KeyIsPressed(VK_CANCEL) || (KeyIsPressed(VK_CONTROL) && KeyIsPressed(0x10,true))) quit(EXIT_SUCCESS);										// check ctrl+break and quit if it's held
		
		/////////////// left alt - mash space ///////////////

		kbLayout = GetKeyboardLayout(0);

		if(KeyIsPressed(VK_MENU) || KeyIsPressed(VK_NUMPAD0))
		{
			if (!mashHeldLast)
			{
				cout << "space mashing started" << endl;
				SendKeyboardInput(VK_SPACE);
				//cout << "space pressed" << endl;
			}

			SendKeyboardInput(VK_SPACE,true);
			//cout << "space released" << endl;
			SendKeyboardInput(VK_SPACE);
			//cout << "space pressed" << endl;

			mashHeldLast = true;
		}
		else if(mashHeldLast)																// clean up if user released the space-mashing key
		{
			ReleaseKey(VK_SPACE);
			cout << "space mashing ended" << endl;
			mashHeldLast = false;
		}

		/////////////// numpad 4 - flip-walk left ///////////////

		if(KeyIsPressed(VK_NUMPAD4) || KeyIsPressed(0x2C,true))
		{
			if(!leftHeldLast)
			{
				cout << "flip-walking left started" << endl;
				SendKeyboardInput(VK_LEFT);
				//cout << "left pressed" << endl;
			}

			leftHeldFrames = leftHeldFrames + 1;											// get however many frames this functionality's bind has been held
			targetLeftState = (leftHeldFrames % 11 != 10);									// at the 9th frame, let go of left for a frame
			if(KeyIsPressed(VK_LEFT) != targetLeftState)									// VP 2007.02.27: synchronize desired state with actual state
			{
				SendKeyboardInput((targetLeftState ? VK_RIGHT : VK_LEFT), true);
				//cout << (targetLeftState ? "right " : "left ") << "released" << endl;
				SendKeyboardInput((targetLeftState ? VK_LEFT : VK_RIGHT));
				//cout << (targetLeftState ? "left " : "right ") << "pressed" << endl;
			}
			leftHeldLast = true;
		}
		else if(leftHeldLast)																// clean up if user released numpad 4
		{
			cout << "flip-walking left ended" << endl;
			ReleaseKey(VK_LEFT);
			ReleaseKey(VK_RIGHT);
			leftHeldLast = false;
			leftHeldFrames = 0;
		}

		/////////////// numpad 6 - flip-walk right ///////////////

		if(KeyIsPressed(VK_NUMPAD6) || KeyIsPressed(0x2E,true))
		{
			if(!rightHeldLast)
			{
				cout << "flip-walking right started" << endl;
				SendKeyboardInput(VK_RIGHT);
				//cout << "right pressed" << endl;
			}

			rightHeldFrames = rightHeldFrames + 1;											// get however many frames this functionality's bind has been held
			targetRightState = (rightHeldFrames % 11 != 10);								// at the 9th frame, let go of left for a frame
			if(KeyIsPressed(VK_RIGHT) != targetRightState)									// VP 2007.02.27: synchronize desired state with actual state
			{
				SendKeyboardInput((targetRightState ? VK_LEFT : VK_RIGHT), true);
				//cout << (targetRightState ? "left " : "right ") << "released" << endl;
				SendKeyboardInput((targetRightState ? VK_RIGHT : VK_LEFT));
				//cout << (targetRightState ? "right " : "left ") << "pressed" << endl;
			}
			rightHeldLast = true;
		}
		else if(rightHeldLast)																// clean up if user released numpad 6
		{
			cout << "flip-walking right ended" << endl;
			ReleaseKey(VK_LEFT);
			ReleaseKey(VK_RIGHT);
			rightHeldLast = false;
			rightHeldFrames = 0;
		}

		/////////////// numpad 5 - alternate left and right arrow keys ///////////////

		if(KeyIsPressed(VK_NUMPAD5) || KeyIsPressed(0x2D,true))
		{
			if(!alternateHeldLast)
			{
				cout << "alternating arrow keys started" << endl;
				SendKeyboardInput(goingRight ? VK_LEFT : VK_RIGHT);
				goingRight = !goingRight;
			}

			goingRight = !goingRight;								// every other frame
			SendKeyboardInput((goingRight ? VK_LEFT : VK_RIGHT), true);
			SendKeyboardInput((goingRight ? VK_RIGHT : VK_LEFT));

			alternateHeldLast = true;
			//cout << "pressed: " << (KeyIsPressed(VK_LEFT) ? (string)"left" : (string)"") << (KeyIsPressed(VK_RIGHT) ? (string)"right" : (string)"") << endl;
			//cout << "tick.QuadPart is " << to_string(tick.QuadPart) << endl;
			//cout << "nextEntryTime is " << to_string(nextEntryTime) << endl;
		}
		else if(alternateHeldLast)															// clean up if user released numpad 5
		{
			cout << "alternating arrow keys ended" << endl;
			ReleaseKey(VK_LEFT);
			ReleaseKey(VK_RIGHT);
			alternateHeldLast = false;
		}
	} while(1);
}
