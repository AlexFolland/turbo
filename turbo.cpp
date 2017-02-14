#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
#include <Windows.h>
using namespace std;

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
			HKL kbLayout = GetKeyboardLayout(0);
			code = MapVirtualKeyEx(code,MAPVK_VSC_TO_VK_EX,kbLayout);
		}
		return GetKeyState(code) & 0x8000;
	}
}

using namespace turbo;

int main()
{
	// declarations and definitions

	unsigned int targetFPS = 50;												// target frames per second
	unsigned int targetGranularity = 2;											// target number of times to poll per frame at target frame rate
	LARGE_INTEGER ticksPerSecond;												// timer frequency, defined by QueryPerformanceFrequency()
	LARGE_INTEGER tick;															// tick to get on every loop for timing
	unsigned long long tickFrames;												// the above in target frame lengths

	bool laltHeldLast = false;													// whether the space-mashing function key was held in the last loop
	LARGE_INTEGER laltStart;													// point at which the space-mashing function key began being held
	unsigned long long laltStartFrames = 0;										// laltStart in target frame lengths
	LARGE_INTEGER laltTick;														// point at which the space-mashing function key began being held
	unsigned long long laltTickFrames = 0;										// laltTick in target frame lengths
	unsigned long long laltHeldFrames = 0;										// number of frames the space-mashing function key was held so far
	unsigned long long lastLaltHeldFrames = 0;									// number of frames the space-mashing function key was held as of the previous loop

	bool leftHeldLast = false;													// whether the left skip-walk function key was held in the last loop
	bool leftDown = false;														// ..
	LARGE_INTEGER leftStart;													// point at which the left skip-walk function key began being held
	unsigned long long leftStartFrames;											// leftStart in target frame lengths
	LARGE_INTEGER leftTick;														// for comparison between timer query points
	unsigned long long leftTickFrames;											// leftTick in target frame lengths
	unsigned long long leftHeldFrames;											// length of time the left skip-walk function key was held for

	bool rightHeldLast = false;													// whether the right skip-walk function key was held in the last loop
	bool rightDown = false;														// ..
	LARGE_INTEGER rightStart;													// point at which the left skip-walk function key began being held
	unsigned long long rightStartFrames;										// rightStart in target frame lengths
	LARGE_INTEGER rightTick;													// for comparison between timer query points
	unsigned long long rightTickFrames;											// rightTick in target frame lengths
	unsigned long long rightHeldFrames;											// length of time the left skip-walk function key was held for

	bool alternateHeldLast = false;												// whether the arrow-key-alternating function key was held in the last loop
	bool alternateDown = false;													// ..
	LARGE_INTEGER alternateStart;												// point at which the arrow-key-alternating function key began being held
	unsigned long long alternateStartFrames;									// alternateStart in target frame lengths
	LARGE_INTEGER alternateTick;												// for comparison between timer query points
	unsigned long long alternateTickFrames;										// alternateTick in target frame lengths
	unsigned long long alternateHeldFrames;										// length of time the arrow-key-alternating function key was held for
	unsigned long long lastAlternateHeldFrames;									// number of frames the arrow-key-alternating function key was held as of the previous loop
	bool goingRight = false;													// whether currently going right

	if(!QueryPerformanceFrequency(&ticksPerSecond))								// QueryPerformanceFrequency() execution and error check
	{
		cout << "Error: QueryPerformanceFrequency() returned false.  This system has no" << endl
			 << "high-resolution timer.  Quitting." << endl;
		return 1;
	}

	// user instructions

	cout << " input         | effect"															<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 0      | mash space repeatedly, re-pressing each frame"					<< endl
		 << "               |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 4      | skip-walk left (left 9 frames, release 1 frame, repeat)"			<< endl
		 << " qwerty Z      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 6      | skip-walk right (right 9 frames, release 1 frame, repeat)"		<< endl
		 << " qwerty X      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 5      | alternate left and right arrow keys each frame"					<< endl
		 << " qwerty C      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " ctrl+break    | quit"																<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 <<																						endl
		 << "debug output"																		<< endl
		 << "------------"																		<< endl;

	// main loop

	do
	{
		this_thread::sleep_for(chrono::microseconds((unsigned long long)floor(1000000/(targetGranularity*targetFPS))));									// sleep between checks (number of times per frame dictated by fps and granularity)

		QueryPerformanceCounter(&tick);																													// generic tick
		tickFrames = (unsigned long long)floor((unsigned long long)tick.LowPart*targetFPS/(unsigned long long)ticksPerSecond.LowPart);					// convert to WA frames
	
		// begin input checks
		
		/////////////// left alt - mash space ///////////////

		// VP 2007.02.27: relying on GetKeyboardState whether space is globally down or not is wrong
		// because keybd_event puts stuff in the global event queue - while GetKeyboardState only gets 
		// the state of the keys after the applications removed the corresponding messages from the message queue
			// Lex 2007.02.27: this would only matter if you relied on a key to be pressed to press itself in a different way
			// like if you used space as the space-mashing hotkey.

		if(KeyIsPressed(VK_MENU) || KeyIsPressed(VK_NUMPAD0))
		{
			if (!laltHeldLast)
			{
				cout << "space mashing started" << endl;
				QueryPerformanceCounter(&laltStart);
				laltStartFrames = (unsigned long long)floor((unsigned long long)laltStart.LowPart*targetFPS/(unsigned long long)ticksPerSecond.LowPart);// convert to target frame lengths
				SendKeyboardInput(VK_SPACE);
				//cout << "space pressed" << endl;
			}

			lastLaltHeldFrames = laltHeldFrames;
			laltHeldFrames = tickFrames - laltStartFrames;

			if (laltHeldFrames > lastLaltHeldFrames)
			{
				SendKeyboardInput(VK_SPACE,true);
				//cout << "space released" << endl;
				SendKeyboardInput(VK_SPACE);
				//cout << "space pressed" << endl;
			}

			laltHeldLast = true;
		}
		else if(laltHeldLast)																// clean up if user released left alt
		{
			if(KeyIsPressed(VK_SPACE))
			{
				SendKeyboardInput(VK_SPACE,true);
				//cout << "space released (during clean-up)" << endl;
			}
			cout << "space mashing ended" << endl;
			laltHeldFrames = 0;
			laltHeldLast = false;
		}

		/////////////// numpad 4 - skip-walk left ///////////////

		if(KeyIsPressed(VK_NUMPAD4) || KeyIsPressed(0x2C,true))
		{
			if(!leftHeldLast)
			{
				cout << "skip-walking left started" << endl;
				QueryPerformanceCounter(&leftStart);
				// VP 2007.02.27: reusing variables is BAD!
				leftStartFrames = (unsigned long long)floor((unsigned long long)leftStart.LowPart*targetFPS/(unsigned long long)ticksPerSecond.LowPart);			// convert to WA frame lengths
				SendKeyboardInput(VK_LEFT);
				//cout << "left pressed" << endl;
				leftDown = true;
			}
			// VP 2007.02.27: this addition caused leftHeldFrames to grow with steady progression - it should be an assignment instead
			leftHeldFrames = tickFrames - leftStartFrames;									// get however much time was spent looping, converted into target frame lengths (0.02 seconds each for WA)
			bool targetLeftState = (leftHeldFrames % 11 != 10);								// at the 9th frame, let go of left for a frame
			if(leftDown != targetLeftState)													// VP 2007.02.27: synchronize desired state with actual state
			{
				SendKeyboardInput(VK_LEFT, !targetLeftState);
				//cout << "left " << (targetLeftState ? "pressed" : "released") << endl;
				leftDown = targetLeftState;
			}
			leftHeldLast = true;
		}
		else if(leftHeldLast)																// clean up if user released numpad 4
		{
			cout << "skip-walking left ended" << endl;
			if(KeyIsPressed(VK_LEFT))
			{
				SendKeyboardInput(VK_LEFT,true);
				//cout << "left released (during clean-up)" << endl;
			}
			leftHeldLast = false;
			leftHeldFrames = 0;
		}

		/////////////// numpad 6 - skip-walk right ///////////////

		if(KeyIsPressed(VK_NUMPAD6) || KeyIsPressed(0x2D,true))
		{
			if(!rightHeldLast)
			{
				cout << "skip-walking right started" << endl;
				QueryPerformanceCounter(&rightStart);
				// VP 2007.02.27: reusing variables is BAD!
				rightStartFrames = (unsigned long long)floor((unsigned long long)rightStart.LowPart*targetFPS/(unsigned long long)ticksPerSecond.LowPart);			// convert to WA frame lengths
				SendKeyboardInput(VK_RIGHT);
				//cout << "right pressed" << endl;
				rightDown = true;
			}
			// VP 2007.02.27: this addition caused rightHeldFrames to grow with steady progression - it should be an assignment instead
			rightHeldFrames = tickFrames - rightStartFrames;								// get however much time was spent looping, converted into WA frame lengths (0.02 seconds each)
			bool targetRightState = (rightHeldFrames % 11 != 10);							// at the 9th frame, let go of left for a frame
			if(rightDown != targetRightState)												// VP 2007.02.27: synchronize desired state with actual state
			{
				SendKeyboardInput(VK_RIGHT, !targetRightState);
				//cout << "right " << (targetRightState ? "pressed" : "released") << endl;
				rightDown = targetRightState;
			}
			rightHeldLast = true;
		}
		else if(rightHeldLast)																// clean up if user released numpad 6
		{
			cout << "skip-walking right ended" << endl;
			if(KeyIsPressed(VK_RIGHT))
			{
				SendKeyboardInput(VK_RIGHT,true);
				//cout << "right released (during clean-up)" << endl;
			}
			rightHeldLast = false;
			rightHeldFrames = 0;
		}

		/////////////// numpad 5 - alternate left and right arrow keys ///////////////

		if(KeyIsPressed(VK_NUMPAD5) || KeyIsPressed(0x2E,true))
		{
			if(!alternateHeldLast)
			{
				cout << "alternating arrow keys started" << endl;
				QueryPerformanceCounter(&alternateStart);
				alternateStartFrames = (unsigned long long)floor((unsigned long long)alternateStart.LowPart*targetFPS/(unsigned long long)ticksPerSecond.LowPart);	// convert to WA frame lengths
				SendKeyboardInput(goingRight ? VK_LEFT : VK_RIGHT);
				//cout << (goingRight ? "left" : "right") << " pressed" << endl;
				goingRight = !goingRight;
			}

			lastAlternateHeldFrames = alternateHeldFrames;
			alternateHeldFrames = tickFrames - alternateStartFrames;						// get however much time was spent looping, converted into WA frame lengths (0.02 seconds each)
			
			if(alternateHeldFrames > lastAlternateHeldFrames)								// only if a frame has passed
			{
				goingRight = !goingRight;								// every other frame
				if(goingRight)
				{
					SendKeyboardInput(VK_LEFT,true);
					SendKeyboardInput(VK_RIGHT);
				}
				else
				{
					SendKeyboardInput(VK_RIGHT,true);
					SendKeyboardInput(VK_LEFT);
				}
				//cout << (goingRight ? "left released" : "right released") << endl;
				//cout << (goingRight ? "right pressed" : "left pressed") << endl;
			}
			alternateHeldLast = true;
		}
		else if(alternateHeldLast)															// clean up if user released numpad 5
		{
			lastAlternateHeldFrames = alternateHeldFrames;
			alternateHeldFrames = tickFrames - alternateStartFrames;	
			if(alternateHeldFrames > lastAlternateHeldFrames);								// keep checking whether a frame is done before cleaning up
			{
				cout << "alternating arrow keys ended" << endl;
				if(KeyIsPressed(VK_LEFT))
				{
					SendKeyboardInput(VK_LEFT,true);
					//cout << "left released (during clean-up)" << endl;
				}
				if(KeyIsPressed(VK_RIGHT))
				{
					SendKeyboardInput(VK_RIGHT,true);
					//cout << "right released (during clean-up)" << endl;
				}
				alternateHeldLast = false;
				alternateHeldFrames = 0;
			}
		}

		/////////////// ctrl+break - quit ///////////////

	} while(!KeyIsPressed(VK_CANCEL));															// check ctrl+break and quit if it's held; otherwise, loop again

	// clean-up

	cout << "quitting..." << endl;
	return 0;
}
