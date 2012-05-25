#include <iostream>
#include <Windows.h>
using namespace std;


int main()
{
    // declarations and definitions

    LARGE_INTEGER ticksPerSecond;   // timer frequency, defined by QueryPerformanceFrequency()
    char kbState[256];              // array for GetKeyboardState() to dump into
    bool laltHeldLast = false;      // whether left alt was held in the last loop
    bool leftHeldLast = false;      // whether the left skip-walk function key was held in the last loop
    LARGE_INTEGER leftStart;        // point at which the left skip-walk function key began being held
    LARGE_INTEGER leftTick;         // for comparison between timer query points
    LARGE_INTEGER leftHeldTime;     // length of time the left skip-walk function key was held for
    leftHeldTime.QuadPart = 0;

    if(!QueryPerformanceFrequency(&ticksPerSecond))                                 // QueryPerformanceFrequency() execution and error check
    {
        cout << "Error: QueryPerformanceFrequency() returned false.  This system has no" << endl
             << "high-resolution timer.  Quitting." << endl;
        return 0;
    }

    // instructions

    cout << " input         | effect"                                                         << endl
         << "---------------|---------------------------------------------------------------" << endl
         << " left alt      | mash space 1000 times per second, minus processing time"        << endl
         << "---------------|---------------------------------------------------------------" << endl
         << " numpad 4      | skip-walk left"                                                 << endl
         << "---------------|---------------------------------------------------------------" << endl
         << " ctrl+break    | quit"                                                           << endl
         << "---------------|---------------------------------------------------------------" << endl
         <<                                                                                      endl
         << "debug output"                                                                    << endl
         << "------------"                                                                    << endl;

    // input-checking loop

    do
    {
        Sleep(1);                                                                   // wait time between input-checking loops (in milliseconds)
        GetKeyState(0);                                                             // hackish GetKeyboardState() bug workaround.  thanks, CyberShadow!
        if(!GetKeyboardState((byte*)kbState))                                       // GetKeyboardState() execution and error check
        {
            cout << "Error getting keyboard state.  GetKeyboardState() returned false." << endl
                 << "Quitting." << endl;
            return 0;
        }
        
        // begin input checks
        
        /////////////// left alt - mash space ///////////////

        if(kbState[VK_LMENU]<0)
        {
            if (laltHeldLast) keybd_event(VK_SPACE,0,KEYEVENTF_KEYUP,0);
            {} {            } keybd_event(VK_SPACE,0,0,0);
            laltHeldLast = true;
        }
        else if(laltHeldLast)                                                       // clean up if user released left alt
        {
            if(kbState[VK_SPACE]<0) keybd_event(VK_SPACE,0,KEYEVENTF_KEYUP,0);
            laltHeldLast = false;
        }

        /////////////// numpad 4 - skip-walk left ///////////////  note: the cout functions are for debugging
        /////////////////////////////////////////////////////////        there is something seriously wrong here and i haven't figured out what it is yet

        if(kbState[VK_NUMPAD4]<0)
        {
            if(!leftHeldLast)
            {
                cout << "4 pressed" << endl;
                QueryPerformanceCounter(&leftStart);
                leftStart.QuadPart = leftStart.QuadPart*50/ticksPerSecond.QuadPart; // convert to WA frame lengths
            }
            QueryPerformanceCounter(&leftTick);
            leftTick.QuadPart = leftTick.QuadPart*50/ticksPerSecond.QuadPart;       // convert to WA frame lengths
            leftHeldTime.QuadPart += leftTick.QuadPart - leftStart.QuadPart;        // add however much time was taken to loop, converted into WA frame lengths (0.02 seconds each)
            if(leftHeldTime.QuadPart % 10 == 0 && kbState[VK_LEFT]>=0)              // at the beginning of the 10-frame cycle, start holding left
            {
                keybd_event(VK_LEFT,0,0,0);
                cout << "left pressed" << endl;
            }
            else if(leftHeldTime.QuadPart % 10 == 9 && kbState[VK_LEFT]<0)          // at the 9th frame, let go of left for a frame, then repeat
            {
                keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
                cout << "left released" << endl;
            }
            leftHeldLast = true;
        }
        else if(leftHeldLast)                                                       // clean up if user released numpad 4
        {
            cout << "4 released" << endl;
            if(kbState[VK_LEFT]<0)
            {
                keybd_event(VK_LEFT,0,0 | KEYEVENTF_KEYUP,0);
                cout << "left released (during clean-up)" << endl;
            }
            leftHeldLast = false;
            leftHeldTime.QuadPart = 0;
        }

        /////////////// ctrl+break - quit ///////////////

    } while(kbState[VK_CANCEL]>=0);                                                 // check ctrl+break and quit if it's held; otherwise, loop again
    
    // clean-up

    cout << "quitting..." << endl;
    return 0;
}
