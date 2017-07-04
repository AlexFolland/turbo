# turbo

This is a Windows tool with custom hard-coded keyboard macros for Worms Armageddon.  However, it works for anything, as it simply sends keyboard input with precise timing at a target frame rate.  In the case of WA, that frame rate is 50Hz.  The target frame rate can be edited at the top of the code and the program rebuilt easily with GCC from msys2's mingw-w64.  The GCC command to build it is ```g++ -O0 -o turbo.exe turbo.cpp```.

## Usage

Use Alt or Numpad0 to mash the space bar each frame, Z or Numpad4 to flip-walk left, C or Numpad6 to flip-walk right, and X or Numpad5 to alternate left and right arrow keys each frame.  This program supports any keyboard layout.  The Z, C, and X keybinds are actually just the positions of those keys on the qwerty layout, but pressing those key positions on any layout (Dvorak or qwertz, for example) works the same way.

## Disclaimer

I don't condone cheating.  I don't use this to cheat, either.  I made this for fun, to see if I could align keyboard input with the game's frame rate without hooking the game's input directly, and so I could wiggle back and forth amusingly.  If you use this for cheating, it's completely obvious anyway, so you shouldn't if you care about your reputation and if you want to feel good about your accomplishments.
