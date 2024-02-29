//--------------------------------------------------------------------
//
//   VBinDiff
//   Copyright 1997-2017 by Christopher J. Madsen
//
//   Support class for Win32 console mode applications
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <https://www.gnu.org/licenses/>.
//--------------------------------------------------------------------

#include "config.h"

#include "ConWin.hpp"

void exitMsg(int status, const char* message); // From vbindiff.cpp

//====================================================================
// Colors:
//--------------------------------------------------------------------

#define F_BLACK 0
#define F_RED   FOREGROUND_RED
#define F_WHITE (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define F_YELLOW (FOREGROUND_GREEN|FOREGROUND_RED)
#define B_BLUE  BACKGROUND_BLUE
#define B_WHITE (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)

static const WORD colorStyle[] = {
  F_WHITE|B_BLUE,                       // cBackground
  F_WHITE|B_BLUE,                       // cPromptWin
  F_WHITE|B_BLUE|FOREGROUND_INTENSITY,  // cPromptKey
  F_WHITE|B_BLUE|FOREGROUND_INTENSITY,  // cPromptBdr
  F_BLACK|B_WHITE,                      // cCurrentMode
  F_BLACK|B_WHITE,                      // cFileName
  F_WHITE|B_BLUE,                       // cFileWin
  F_RED|B_BLUE|FOREGROUND_INTENSITY,    // cFileDiff
  F_YELLOW|B_BLUE|FOREGROUND_INTENSITY  // cFileEdit
};

// CP437 to Unicode table
static const wchar_t char_to_wc[256] = {
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302,
  0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
  0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
  0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
  0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
  0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
  0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

//====================================================================
// Class ConWindow:
//--------------------------------------------------------------------
HANDLE  ConWindow::inBuf  = INVALID_HANDLE_VALUE;
HANDLE  ConWindow::scrBuf = INVALID_HANDLE_VALUE;

static DWORD  origInMode = 0;    // The original input mode

//////////////////////////////////////////////////////////////////////
// Static Member Functions:
//--------------------------------------------------------------------
// Start up the window system:
//
// Allocates a screen buffer and sets input mode:
//
// Returns:
//   true:   Everything set up properly
//   false:  Unable to initialize

bool ConWindow::startup()
{
  inBuf = GetStdHandle(STD_INPUT_HANDLE);
  if (inBuf == INVALID_HANDLE_VALUE)
    return false;

  scrBuf = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
                                     0, NULL, // No sharing
                                     CONSOLE_TEXTMODE_BUFFER, NULL);

  if (scrBuf == INVALID_HANDLE_VALUE)
    return false;

  if (!SetConsoleActiveScreenBuffer(scrBuf) ||
      !SetConsoleMode(scrBuf, 0) ||
      !GetConsoleMode(inBuf, &origInMode) ||
      !SetConsoleMode(inBuf, 0)) {
    CloseHandle(scrBuf);
    return false;
  }

  return true;
} // end ConWindow::startup

//--------------------------------------------------------------------
// Shut down the window system:
//
// Deallocate the screen buffer and restore the original input mode.

void ConWindow::shutdown()
{
  if (origInMode)
    SetConsoleMode(inBuf, origInMode);
  if (scrBuf != INVALID_HANDLE_VALUE)
    CloseHandle(scrBuf);
  scrBuf = INVALID_HANDLE_VALUE;
} // end ConWindow::shutdown

//--------------------------------------------------------------------
// Return the current screen size:

void ConWindow::getScreenSize(int& x, int& y)
{
  CONSOLE_SCREEN_BUFFER_INFO  info;

  if (GetConsoleScreenBufferInfo(scrBuf, &info)) {
    x = info.dwSize.X;
    y = info.dwSize.Y;
  } else {
    x = y = 0;
  }
} // end ConWindow::getScreenSize

//--------------------------------------------------------------------
// Make the cursor invisible:

void ConWindow::hideCursor()
{
  CONSOLE_CURSOR_INFO  info;

  if (GetConsoleCursorInfo(scrBuf, &info)) {
    info.bVisible = FALSE;
    SetConsoleCursorInfo(scrBuf, &info);
  }
} // end ConWindow::hideCursor

//--------------------------------------------------------------------
// Read the next key down event:
//
// Output:
//   event:  Contains a key down event

void ConWindow::readKey(KEY_EVENT_RECORD& event)
{
  INPUT_RECORD  e;

  for (;;) {
    DWORD  count = 0;
    while (!count)
      ReadConsoleInput(inBuf, &e, 1, &count);

    if ((e.EventType == KEY_EVENT) && e.Event.KeyEvent.bKeyDown) {
      event = e.Event.KeyEvent;
      return;
    }
  } // end forever
} // end ConWindow::readKey

//--------------------------------------------------------------------
// Curses-compatible readKey function:

int ConWindow::readKey()
{
  KEY_EVENT_RECORD  e;

  readKey(e);
  switch (e.wVirtualKeyCode) {
   case VK_ESCAPE:  return KEY_ESCAPE;
   case VK_TAB:     return KEY_TAB;
   case VK_BACK:    return KEY_BACKSPACE;
   case VK_RETURN:  return KEY_RETURN;
   case VK_DELETE:  return KEY_DC;
   case VK_INSERT:  return KEY_IC;
   case VK_HOME:    return KEY_HOME;
   case VK_END:     return KEY_END;
   case VK_UP:      return KEY_UP;
   case VK_DOWN:    return KEY_DOWN;
   case VK_LEFT:    return KEY_LEFT;
   case VK_RIGHT:   return KEY_RIGHT;
  }

  return e.uChar.AsciiChar;
} // end ConWindow::readKey

//--------------------------------------------------------------------
// Make the cursor visible:

void ConWindow::showCursor(bool insert)
{
  CONSOLE_CURSOR_INFO  info;

  if (GetConsoleCursorInfo(scrBuf, &info)) {
    info.bVisible = TRUE;
    info.dwSize = (insert ? 10 : 100);
    SetConsoleCursorInfo(scrBuf, &info);
  }
} // end ConWindow::showCursor

//////////////////////////////////////////////////////////////////////
// Member Functions:
//--------------------------------------------------------------------
// Constructor:

ConWindow::ConWindow()
: data(NULL)
{
} // end ConWindow::ConWindow

//--------------------------------------------------------------------
// Destructor:

ConWindow::~ConWindow()
{
  delete[] data;
} // end ConWindow::~ConWindow

//--------------------------------------------------------------------
// Initialize the window:
//
// Must be called only once, before any other functions are called.
// Allocates the data structures and clears the window buffer, but
// does not display anything.
//
// Input:
//   x,y:           The position of the window in the screen buffer
//   width,height:  The size of the window
//   attrib:        The default attributes for the window

void ConWindow::init(short x, short y, short width, short height, Style style)
{
  ASSERT(data == NULL);
  attribs = colorStyle[style];
  pos.X  = x;
  pos.Y  = y;

  resize(width, height);
} // end ConWindow::init

//--------------------------------------------------------------------
// Draw a box in the window:
//
// Input:
//   x,y:           The upper left corner of the box in the window
//   width,height:  The size of the box
//   tl,tr:         The characters for the top left & top right corners
//   bl,br:         The characters for the bottom left & right corners
//   horiz:         The character for horizontal lines
//   vert:          The character for vertical lines

void ConWindow::box(short x, short y, short width, short height,
                    wchar_t tl, wchar_t tr, wchar_t bl, wchar_t br, wchar_t horiz, wchar_t vert)
{
  ASSERT((height > 1) && (width > 1));

  PCHAR_INFO  c1 = data + x + size.X * y;
  PCHAR_INFO  c  = c1;
  PCHAR_INFO  c2 = c1 + width - 1;

  c1->Char.UnicodeChar = tl;
  c1->Attributes = attribs;
  c2->Char.UnicodeChar = tr;
  c2->Attributes = attribs;

  while (++c < c2) {
    c->Char.UnicodeChar = horiz;
    c->Attributes = attribs;
  }

  c  =  c1 + size.X * (height-1);
  for (;;) {
    c1 += size.X;
    c2 += size.X;
    if (c1 == c) break;
    c1->Char.UnicodeChar = c2->Char.UnicodeChar = vert;
    c1->Attributes = c2->Attributes = attribs;
  }

  c1->Char.UnicodeChar = bl;
  c1->Attributes = attribs;
  c2->Char.UnicodeChar = br;
  c2->Attributes = attribs;

  while (++c1 < c2) {
    c1->Char.UnicodeChar = horiz;
    c1->Attributes = attribs;
  }
} // end ConWindow::box

//--------------------------------------------------------------------
// Draw a box with a single line around the window's border:

void ConWindow::border()
{
  box(0,0,size.X,size.Y, 9484,9488, 9492,9496, 9472, 9474);
} // end ConWindow::border

//--------------------------------------------------------------------
// Clear the window:

void ConWindow::clear()
{
  PCHAR_INFO  c = data;
  for (int i = size.X * size.Y; i > 0; ++c, --i) {
    c->Char.UnicodeChar = L' ';
    c->Attributes = attribs;
  }
} // end ConWindow::clear

//--------------------------------------------------------------------
// Write a string using the current attributes:
//
// Input:
//   x,y:  The start of the string in the window
//   s:    The string to write

void ConWindow::put(short x, short y, const char* s)
{
  PCHAR_INFO  out = data + x + size.X * y;

  while (*s) {
    out->Char.UnicodeChar = char_to_wc[static_cast<unsigned char>(*s)];
    out->Attributes = attribs;
    ++out;
    ++s;
  }
} // end ConWindow::put

///void ConWindow::put(short x, short y, const String& s)
///{
///  PCHAR_INFO  out = data + x + size.X * y;
///  StrConstItr  c = s.begin();
///
///  while (c != s.end()) {
///    out->Char.UnicodeChar = char_to_wc[static_cast<unsigned char>(*c)];
///    out->Attributes = attribs;
///    ++out;
///    ++c;
///  }
///} // end ConWindow::put

//--------------------------------------------------------------------
// Change the attributes of characters in the window:
//
// Input:
//   x,y:    The position in the window to start changing attributes
//   color:  The attribute to set
//   count:  The number of characters to change

void ConWindow::putAttribs(short x, short y, Style color, short count)
{
  PCHAR_INFO  c = data + x + size.X * y;

  while (count--)
    (c++)->Attributes = colorStyle[color];
} // end ConWindow::putAttribs

//--------------------------------------------------------------------
// Write a character using the current attributes:
//
// Input:
//   x,y:    The position in the window to start writing
//   c:      The character to write
//   count:  The number of characters to write

void ConWindow::putChar(short x, short y, char c, short count)
{
  PCHAR_INFO  ci = data + x + size.X * y;

  while (count--) {
    ci->Char.UnicodeChar = char_to_wc[static_cast<unsigned char>(c)];
    (ci++)->Attributes = attribs;
  }
} // end ConWindow::putAttribs

//--------------------------------------------------------------------
void ConWindow::resize(short width, short height)
{
  if ((size.X != width) || (size.Y != height)) {
    size.X = width;
    size.Y = height;

    delete [] data;
    data = new CHAR_INFO[size.X * size.Y];
  } // end if new size

  clear();
} // end ConWindow::resize

//--------------------------------------------------------------------
void ConWindow::setAttribs(Style color)
{
  attribs = colorStyle[color];
} // end ConWindow::setAttribs

//--------------------------------------------------------------------
// Position the cursor in the window:
//
// There is only one cursor, and each window does not maintain its own
// cursor position.
//
// Input:
//   x,y:    The position in the window for the cursor

void ConWindow::setCursor(short x, short y)
{
  ASSERT((x>=0)&&(x<size.X)&&(y>=0)&&(y<size.Y));
  COORD c;
  c.X = x + pos.X;
  c.Y = y + pos.Y;
  SetConsoleCursorPosition(scrBuf, c);
} // end ConWindow::setCursor

//--------------------------------------------------------------------
// Display the window on the screen:
//
// This is the only function that actually displays anything.
//
// Input:
//   margin:  Exclude this many characters around the edge (default 0)

void ConWindow::update(unsigned short margin)
{
  SMALL_RECT r;
  r.Left   = pos.X + margin;
  r.Top    = pos.Y + margin;
  r.Right  = pos.X + size.X - 1 - margin;
  r.Bottom = pos.Y + size.Y - 1 - margin;

  const COORD offset = {margin, margin};

  WriteConsoleOutputW(scrBuf, data, size, offset, &r);
} // end ConWindow::update

//--------------------------------------------------------------------
// Local Variables:
// cjm-related-file: "ConWin.hpp"
//     c-file-style: "cjm"
// End:
