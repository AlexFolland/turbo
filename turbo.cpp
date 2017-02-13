#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
#include <Windows.h>
using namespace std;

int main()
{
	// declarations and definitions

	unsigned long long targetFPS = 50LL;										// target frames per second
	unsigned long long targetGranularity = 3LL;									// target number of times to poll per frame at target frame rate
	LARGE_INTEGER ticksPerSecond;												// timer frequency, defined by QueryPerformanceFrequency()
	LARGE_INTEGER recheckedFrequency;											// timer frequency, checked again each loop
	LARGE_INTEGER tick;															// tick to get on every loop for timing
	unsigned long long tickFrames;												// the above in target frame lengths

	char kbState[256];															// array for GetKeyboardState() to dump into
	HKL kbLayout;																// keyboard layout for GetKeyboardLayout()

	bool laltHeldLast = false;													// whether the space-mashing function key was held in the last loop
	LARGE_INTEGER laltStart;													// point at which the space-mashing function key began being held
	unsigned long long laltStartFrames;											// laltStart in WA frame lengths
	LARGE_INTEGER laltTick;														// point at which the space-mashing function key began being held
	unsigned long long laltTickFrames;											// laltTick in WA frame lengths
	unsigned long long laltHeldFrames;											// number of frames the space-mashing function key was held so far
	unsigned long long lastLaltHeldFrames;										//number of frames the space-mashing function key was held as of the previous loop

	bool leftHeldLast = false;													// whether the left skip-walk function key was held in the last loop
	bool leftDown = false;														// ..
	LARGE_INTEGER leftStart;													// point at which the left skip-walk function key began being held
	unsigned long long leftStartFrames;											// leftStart in WA frame lengths
	LARGE_INTEGER leftTick;														// for comparison between timer query points
	unsigned long long leftTickFrames;											// leftTick in WA frame lengths
	unsigned long long leftHeldFrames;											// length of time the left skip-walk function key was held for

	bool rightHeldLast = false;													// whether the right skip-walk function key was held in the last loop
	bool rightDown = false;														// ..
	LARGE_INTEGER rightStart;													// point at which the left skip-walk function key began being held
	unsigned long long rightStartFrames;											// rightStart in WA frame lengths
	LARGE_INTEGER rightTick;													// for comparison between timer query points
	unsigned long long rightTickFrames;											// rightTick in WA frame lengths
	unsigned long long rightHeldFrames;											// length of time the left skip-walk function key was held for

	bool alternateHeldLast = false;												// whether the arrow-key-alternating function key was held in the last loop
	bool alternateDown = false;													// ..
	LARGE_INTEGER alternateStart;												// point at which the arrow-key-alternating function key began being held
	unsigned long long alternateStartFrames;										// alternateStart in WA frame lengths
	LARGE_INTEGER alternateTick;												// for comparison between timer query points
	unsigned long long alternateTickFrames;										// alternateTick in WA frame lengths
	unsigned long long alternateHeldFrames;										// length of time the arrow-key-alternating function key was held for
	unsigned long long lastAlternateHeldFrames;									//number of frames the arrow-key-alternating function key was held as of the previous loop

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
		 << " numpad 4      | skip-walk left"													<< endl
		 << " qwerty Z      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 6      | skip-walk right"													<< endl
		 << " qwerty X      |"																	<< endl
		 << "---------------|---------------------------------------------------------------"	<< endl
		 << " numpad 5      | alternate left and right arrow keys"								<< endl
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
		//Sleep(1);																			// wait time between input-checking loops (in milliseconds)
		this_thread::sleep_for(chrono::microseconds((unsigned long long)floor(1000000LL/(targetGranularity*targetFPS))));									// same as above, but allowing microsecond precision
		
		QueryPerformanceFrequency(&recheckedFrequency);
		if(ticksPerSecond.QuadPart != recheckedFrequency.QuadPart)
		{
			cout << "QueryPerformanceFrequency returned a different frequency." << endl;
			ticksPerSecond = recheckedFrequency;
		}
		QueryPerformanceCounter(&tick);														// generic tick
		tickFrames = tick.QuadPart*targetFPS/ticksPerSecond.QuadPart;								// convert to WA frames

		kbLayout = GetKeyboardLayout(0);

		GetKeyState(0);																		// hackish GetKeyboardState() bug workaround.  thanks, CyberShadow!
		if(!GetKeyboardState((byte*)kbState))												// GetKeyboardState() execution and error check
		{
			cout << "Error getting keyboard state.  GetKeyboardState() returned false." << endl
				 << "Quitting." << endl;
			return 1;
		}
		
		// begin input checks
		
		/////////////// left alt - mash space ///////////////

		// VP 2007.02.27: relying on GetKeyboardState whether space is globally down or not is wrong
		// because keybd_event puts stuff in the global event queue - while GetKeyboardState only gets 
		// the state of the keys after the applications removed the corresponding messages from the message queue
			// Lex 2007.02.27: this would only matter if you relied on a key to be pressed to press itself in a different way
			// like if you used space as the space-mashing hotkey.
		
		// VP 2007.02.27: this will actually cause it to work 500 times per second, but it should work 
		// better because the periods when space is up or down will be ~even
			// Lex 2007.02.27: reverted to the old method because it works a lot better with the WA engine without the delay between release and press
		
		// VP 2007.02.27: also, the empty {} and 0 | ... were confusing and distracting
			// Lex 2007.02.27: i'm keeping the empty {}s for alignment, but removing the useless "0 | "s.
			// Lex 2012.03.11: removed empty {}s

		if(kbState[VK_LMENU]<0 || kbState[VK_NUMPAD0]<0)
		{
			if (!laltHeldLast)
			{
				cout << "space mashing started" << endl;
				QueryPerformanceCounter(&laltStart);
				laltStartFrames = (unsigned long long)(floor((unsigned long long)laltStart.QuadPart*targetFPS/(unsigned long long)ticksPerSecond.QuadPart));			// convert to WA frame lengths
				//keybd_event(VK_SPACE,0,0,0);
			}

			lastLaltHeldFrames = laltHeldFrames;
			laltHeldFrames = tickFrames - laltStartFrames;

			if (laltHeldLast && laltHeldFrames > lastLaltHeldFrames)
			{
				keybd_event(VK_SPACE,0,KEYEVENTF_KEYUP,0);
				keybd_event(VK_SPACE,0,0,0);
			}

			laltHeldLast = true;
		}
		else if(laltHeldLast)																// clean up if user released left alt
		{
			if(kbState[VK_SPACE]<0) keybd_event(VK_SPACE,0,KEYEVENTF_KEYUP,0);
			cout << "space mashing ended" << endl;
			laltHeldLast = false;
			laltHeldFrames = 0;
		}

		/////////////// numpad 4 - skip-walk left ///////////////

		if(kbState[VK_NUMPAD4]<0 || kbState[MapVirtualKeyEx(0x2C,3,kbLayout)]<0)
		{
			if(!leftHeldLast)
			{
				cout << "4 pressed" << endl;
				QueryPerformanceCounter(&leftStart);
				// VP 2007.02.27: reusing variables is BAD!
				leftStartFrames = (unsigned long long)(floor((unsigned long long)leftStart.QuadPart*targetFPS/(unsigned long long)ticksPerSecond.QuadPart));			// convert to WA frame lengths
				keybd_event(VK_LEFT,0,0,0);
				cout << "left pressed" << endl;
			}
			// VP 2007.02.27: this addition caused leftHeldFrames to grow with steady progression - it should be an assignment instead
			leftHeldFrames = tickFrames - leftStartFrames;									// get however much time was spent looping, converted into WA frame lengths (0.02 seconds each)
			bool targetLeftState = (leftHeldFrames % 11 != 10);								// at the 9th frame, let go of left for a frame
			if(leftDown != targetLeftState)													// VP 2007.02.27: synchronize desired state with actual state
			{
				keybd_event(VK_LEFT, 0, targetLeftState ? 0 : KEYEVENTF_KEYUP, 0);
				cout << "left " << (targetLeftState ? "pressed" : "released") << endl;
				leftDown = targetLeftState;
			}
			leftHeldLast = true;
		}
		else if(leftHeldLast)																// clean up if user released numpad 4
		{
			cout << "4 released" << endl;
			if(kbState[VK_LEFT]<0)
			{
				keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
				cout << "left released (during clean-up)" << endl;
			}
			leftHeldLast = false;
			leftHeldFrames = 0;
		}

		/////////////// numpad 6 - skip-walk right ///////////////

		if(kbState[VK_NUMPAD6]<0 || kbState[MapVirtualKeyEx(0x2D,3,kbLayout)]<0)
		{
			if(!rightHeldLast)
			{
				cout << "6 pressed" << endl;
				QueryPerformanceCounter(&rightStart);
				// VP 2007.02.27: reusing variables is BAD!
				rightStartFrames = (unsigned long long)(floor((unsigned long long)rightStart.QuadPart*targetFPS/(unsigned long long)ticksPerSecond.QuadPart));			// convert to WA frame lengths
				keybd_event(VK_RIGHT,0,0,0);
				cout << "right pressed" << endl;
			}
			// VP 2007.02.27: this addition caused rightHeldFrames to grow with steady progression - it should be an assignment instead
			rightHeldFrames = tickFrames - rightStartFrames;								// get however much time was spent looping, converted into WA frame lengths (0.02 seconds each)
			bool targetRightState = (rightHeldFrames % 11 != 10);							// at the 9th frame, let go of left for a frame
			if(rightDown != targetRightState)												// VP 2007.02.27: synchronize desired state with actual state
			{
				keybd_event(VK_RIGHT, 0, targetRightState ? 0 : KEYEVENTF_KEYUP, 0);
				cout << "right " << (targetRightState ? "pressed" : "released") << endl;
				rightDown = targetRightState;
			}
			rightHeldLast = true;
		}
		else if(rightHeldLast)																// clean up if user released numpad 6
		{
			cout << "6 released" << endl;
			if(kbState[VK_RIGHT]<0)
			{
				keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
				cout << "right released (during clean-up)" << endl;
			}
			rightHeldLast = false;
			rightHeldFrames = 0;
		}

		/////////////// numpad 5 - alternate left and right arrow keys ///////////////

		if(kbState[VK_NUMPAD5]<0 || kbState[MapVirtualKeyEx(0x2E,3,kbLayout)]<0)
		{
			if(!alternateHeldLast)
			{
				cout << "5 pressed" << endl;
				QueryPerformanceCounter(&alternateStart);
				alternateStartFrames = (unsigned long long)(floor((unsigned long long)alternateStart.QuadPart*targetFPS/(unsigned long long)ticksPerSecond.QuadPart));	// convert to WA frame lengths
				keybd_event(VK_LEFT,0,0,0);
				cout << "left pressed" << endl;
			}

			lastAlternateHeldFrames = alternateHeldFrames;
			alternateHeldFrames = tickFrames - alternateStartFrames;						// get however much time was spent looping, converted into WA frame lengths (0.02 seconds each)
			
			if(alternateHeldFrames > lastAlternateHeldFrames)								// only if a frame has passed
			{
				bool goingRight = (alternateHeldFrames % 2 == 1);								// every other frame
				if(goingRight)
				{
					keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_RIGHT, 0, 0, 0);
				}
				else
				{
					keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_LEFT, 0, 0, 0);
				}
				cout << (goingRight ? "right pressed" : "left pressed") << endl;
			}
			alternateHeldLast = true;
		}
		else if(alternateHeldLast)															// clean up if user released numpad 5
		{
			cout << "5 released" << endl;
			if(kbState[VK_LEFT]<0)
			{
				keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
				cout << "left released (during clean-up)" << endl;
			}
			if(kbState[VK_RIGHT]<0)
			{
				keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
				cout << "right released (during clean-up)" << endl;
			}
			alternateHeldLast = false;
			alternateHeldFrames = 0;
		}

		/////////////// ctrl+break - quit ///////////////

	} while(kbState[VK_CANCEL]>=0);															// check ctrl+break and quit if it's held; otherwise, loop again
	
	// clean-up

	cout << "quitting..." << endl;
	return 0;
}
