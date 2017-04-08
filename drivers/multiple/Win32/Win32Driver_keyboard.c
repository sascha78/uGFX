/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

//--------------------------------------------------------------------------
// Configuration options for this driver
//--------------------------------------------------------------------------

#ifndef GKEYBOARD_WIN32_NO_LAYOUT
	/**
	 * Setting this to GFXON turns off the layout engine.
	 * In this situation "cooked" characters are returned but
	 * shift states etc are lost.
	 * As only a limited number of keyboard layouts are currently
	 * defined for Win32 in uGFX (currently only US English), setting this
	 * to GFXON enables the windows keyboard mapping to be pass non-English
	 * characters to uGFX or to handle non-standard keyboard layouts at
	 * the expense of losing special function keys etc.
	 */
	#define GKEYBOARD_WIN32_NO_LAYOUT			GFXOFF
#endif
#ifndef GKEYBOARD_WIN32_DEFAULT_LAYOUT
	#define GKEYBOARD_WIN32_DEFAULT_LAYOUT		KeyboardLayout_Win32_US
#endif

//--------------------------------------------------------------------------
// Keyboard Driver
//--------------------------------------------------------------------------

#define GKEYBOARD_DRIVER_VMT		GKEYBOARDVMT_Win32
#include "../../../src/ginput/ginput_driver_keyboard.h"

#if !GKEYBOARD_WIN32_NO_LAYOUT
	#if GKEYBOARD_LAYOUT_OFF
		#error "The Win32 keyboard driver is using the layout engine. Please set GKEYBOARD_LAYOUT_OFF=GFXOFF or GKEYBOARD_WIN32_NO_LAYOUT=GFXON."
	#endif

	#include "../../../src/ginput/ginput_keyboard_microcode.h"

	// Forward definitions
	extern uint8_t	GKEYBOARD_WIN32_DEFAULT_LAYOUT[];

	// This is the layout code for the English US keyboard.
	//	We make it public so that a user can switch to a different layout if required.
	uint8_t	KeyboardLayout_Win32_US[] = {
		KMC_HEADERSTART, KMC_HEADER_ID1, KMC_HEADER_ID2, KMC_HEADER_VER_1,

		// Transient Shifters: SHIFT, CTRL, ALT, WINKEY
		/*  1 */KMC_RECORDSTART, 9,													// SHIFT (left & Right)
			KMC_TEST_CODETABLE, 2, VK_SHIFT, VK_LSHIFT,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_SHIFT_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/*  2 */KMC_RECORDSTART, 9,
			KMC_TEST_CODETABLE, 2, VK_SHIFT, VK_LSHIFT,
			KMC_TEST_STATEBIT, GKEYSTATE_SHIFT_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_SHIFT_L_BIT,
			KMC_ACT_DONE,
		/*  3 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RSHIFT,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_SHIFT_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/*  4 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RSHIFT,
			KMC_TEST_STATEBIT, GKEYSTATE_SHIFT_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_SHIFT_R_BIT,
			KMC_ACT_DONE,
		/*  5 */KMC_RECORDSTART, 9,													// CONTROL (left & Right)
			KMC_TEST_CODETABLE, 2, VK_CONTROL, VK_LCONTROL,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_CTRL_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/*  6 */KMC_RECORDSTART, 9,
			KMC_TEST_CODETABLE, 2, VK_CONTROL, VK_LCONTROL,
			KMC_TEST_STATEBIT, GKEYSTATE_CTRL_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_CTRL_L_BIT,
			KMC_ACT_DONE,
		/*  7 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RCONTROL,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_CTRL_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/*  8 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RCONTROL,
			KMC_TEST_STATEBIT, GKEYSTATE_CTRL_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_CTRL_R_BIT,
			KMC_ACT_DONE,
		/*  9 */KMC_RECORDSTART, 9,													// ALT (left & Right)
			KMC_TEST_CODETABLE, 2, VK_MENU, VK_LMENU,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_ALT_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/* 10 */KMC_RECORDSTART, 9,
			KMC_TEST_CODETABLE, 2, VK_MENU, VK_LMENU,
			KMC_TEST_STATEBIT, GKEYSTATE_ALT_L_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_ALT_L_BIT,
			KMC_ACT_DONE,
		/* 11 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RMENU,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_ALT_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/* 12 */KMC_RECORDSTART, 7,
			KMC_TEST_CODE, VK_RMENU,
			KMC_TEST_STATEBIT, GKEYSTATE_ALT_R_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_ALT_R_BIT,
			KMC_ACT_DONE,
		/* 13 */KMC_RECORDSTART, 9,													// WinKey (left or right)
			KMC_TEST_CODETABLE, 2, VK_LWIN, VK_RWIN,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_WINKEY_BIT|KMC_BIT_CLEAR,
			KMC_ACT_DONE,
		/* 14 */KMC_RECORDSTART, 9,
			KMC_TEST_CODETABLE, 2, VK_LWIN, VK_RWIN,
			KMC_TEST_STATEBIT, GKEYSTATE_WINKEY_BIT|KMC_BIT_CLEAR,
			KMC_ACT_STATEBIT, GKEYSTATE_WINKEY_BIT,
			KMC_ACT_DONE,

		// Locking Shifters: CAPSLOCK, NUMLOCK and SCROLLLOCK
		/* 15 */KMC_RECORDSTART, 7,													// CAPSLOCK (keyup only)
			KMC_TEST_CODE, VK_CAPITAL,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_CAPSLOCK_BIT|KMC_BIT_INVERT,
			KMC_ACT_DONE,
		/* 16 */KMC_RECORDSTART, 7,													// NUMLOCK (keyup only)
			KMC_TEST_CODE, VK_NUMLOCK,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_NUMLOCK_BIT|KMC_BIT_INVERT,
			KMC_ACT_DONE,
		/* 17 */KMC_RECORDSTART, 7,													// SCROLLLOCK (keyup only)
			KMC_TEST_CODE, VK_SCROLL,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_SCROLLLOCK_BIT|KMC_BIT_INVERT,
			KMC_ACT_DONE,

		// Keyup, Repeat
		/* 18 */KMC_RECORDSTART, 18,												// Clear any shifter keys that got through
			KMC_TEST_CODETABLE, 14, VK_SHIFT, VK_LSHIFT, VK_RSHIFT,
									VK_CONTROL, VK_LCONTROL, VK_RCONTROL,
									VK_MENU, VK_LMENU, VK_RMENU,
									VK_LWIN, VK_RWIN,
									VK_CAPITAL, VK_NUMLOCK, VK_SCROLL,
			KMC_ACT_RESET,
			KMC_ACT_STOP,
		/* 19 */KMC_RECORDSTART, 4,													// Skip special codes 0x00 (Keyup) & 0x01 (Repeat)
			KMC_TEST_CODERANGE, 0x00, 0x01,
			KMC_ACT_STOP,
		/* 20 */KMC_RECORDSTART, 6,													// Keyup
			KMC_ACT_STATEBIT, GKEYSTATE_KEYUP_BIT|KMC_BIT_CLEAR,
			KMC_TEST_LASTCODE, 0x00,
			KMC_ACT_STATEBIT, GKEYSTATE_KEYUP_BIT,
		/* 21 */KMC_RECORDSTART, 6,													// Repeat
			KMC_ACT_STATEBIT, GKEYSTATE_REPEAT_BIT|KMC_BIT_CLEAR,
			KMC_TEST_LASTCODE, 0x01,
			KMC_ACT_STATEBIT, GKEYSTATE_REPEAT_BIT,

		// 0 - 9
		/* 22 */KMC_RECORDSTART, 7,													// Alt 0-9
			KMC_TEST_ALT,
			KMC_TEST_CODERANGE, '0', '9',
			KMC_ACT_CHARADD,  10,
			KMC_ACT_STOP,
		/* 23 */KMC_RECORDSTART, 17,												// Shifted 0-9
			KMC_TEST_SHIFT,
			KMC_TEST_CODERANGE, '0', '9',
			KMC_ACT_CHARTABLE,  10, ')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
			KMC_ACT_DONE,
		/* 24 */KMC_RECORDSTART, 5,													// 0 - 9
			KMC_TEST_CODERANGE, '0', '9',
			KMC_ACT_CHARCODE,
			KMC_ACT_DONE,

		// A - Z
		/* 25 */KMC_RECORDSTART, 7,													// Control A-Z
			KMC_TEST_CTRL,
			KMC_TEST_CODERANGE, 'A', 'Z',
			KMC_ACT_CHARRANGE, 1,
			KMC_ACT_DONE,
		/* 26 */KMC_RECORDSTART, 7,													// No Caps A-Z
			KMC_TEST_NOCAPS,
			KMC_TEST_CODERANGE, 'A', 'Z',
			KMC_ACT_CHARRANGE, 'a',
			KMC_ACT_DONE,
		/* 27 */KMC_RECORDSTART, 5,													// Caps A-Z
			KMC_TEST_CODERANGE, 'A', 'Z',
			KMC_ACT_CHARCODE,
			KMC_ACT_DONE,

		// Number pad
		/* 28 */KMC_RECORDSTART, 7,													// Alt Number pad
			KMC_TEST_ALT,
			KMC_TEST_CODERANGE, VK_NUMPAD0, VK_NUMPAD9,
			KMC_ACT_CHARADD,  10,
			KMC_ACT_STOP,
		/* 29 */KMC_RECORDSTART, 5,
			KMC_TEST_ALT,
			KMC_TEST_CODERANGE, VK_MULTIPLY, VK_DIVIDE,
			KMC_ACT_STOP,
		/* 30 */KMC_RECORDSTART, 7,													// Number pad with Numlock
			KMC_TEST_NUMLOCK,
			KMC_TEST_CODERANGE, VK_NUMPAD0, VK_NUMPAD9,
			KMC_ACT_CHARRANGE, '0',
			KMC_ACT_DONE,
		/* 31 */KMC_RECORDSTART, 13,
			KMC_TEST_NUMLOCK,
			KMC_TEST_CODERANGE, VK_MULTIPLY, VK_DIVIDE,
			KMC_ACT_CHARTABLE, 6, '*', '+', GKEY_ENTER, '-', '.', '/',
			KMC_ACT_DONE,
		/* 32 */KMC_RECORDSTART, 4,													// Number pad with no Numlock
			KMC_TEST_CODE, VK_NUMPAD5,
			KMC_ACT_RESET,
			KMC_ACT_STOP,
		/* 33 */KMC_RECORDSTART, 12,
			KMC_TEST_CODERANGE, VK_MULTIPLY, VK_DIVIDE,
			KMC_ACT_CHARTABLE, 6, '*', '+', GKEY_ENTER, '-', GKEY_DEL, '/',
			KMC_ACT_DONE,
		/* 34 */KMC_RECORDSTART, 18,
			KMC_TEST_CODERANGE, VK_NUMPAD0, VK_NUMPAD9,
			KMC_ACT_STATEBIT, GKEYSTATE_SPECIAL_BIT,
			KMC_ACT_CHARTABLE, 10, GKEY_INSERT, GKEY_END, GKEY_DOWN, GKEY_PAGEDOWN, GKEY_LEFT, '5', GKEY_RIGHT, GKEY_HOME, GKEY_UP, GKEY_PAGEUP,
			KMC_ACT_DONE,

		// Symbols
		/* 35 */KMC_RECORDSTART, 14,												// Shifted Symbols
			KMC_TEST_SHIFT,
			KMC_TEST_CODERANGE, VK_OEM_1, VK_OEM_3,
			KMC_ACT_CHARTABLE, 7, ':', '+', '<', '_', '>', '?', '~',
			KMC_ACT_DONE,
		/* 36 */KMC_RECORDSTART, 11,
			KMC_TEST_SHIFT,
			KMC_TEST_CODERANGE, VK_OEM_4, VK_OEM_7,
			KMC_ACT_CHARTABLE, 4, '{', '|', '}', '"',
			KMC_ACT_DONE,
		/* 37 */KMC_RECORDSTART, 13,												// Non-shifted Symbols
			KMC_TEST_CODERANGE, VK_OEM_1, VK_OEM_3,
			KMC_ACT_CHARTABLE, 7, ';', '=', ',', '-', '.', '/', '`',
			KMC_ACT_DONE,
		/* 38 */KMC_RECORDSTART, 10,
			KMC_TEST_CODERANGE, VK_OEM_4, VK_OEM_7,
			KMC_ACT_CHARTABLE, 4, '[', '\\', ']', '\'',
			KMC_ACT_DONE,

		// Special Keys
		// Extra special keys like Media and Browser keys are still to be implemented.
		/* 39 */KMC_RECORDSTART, 17,												// Normal Control Type Keys
			KMC_TEST_CODETABLE, 6, VK_BACK, VK_TAB, VK_RETURN, VK_ESCAPE, VK_SPACE, VK_DELETE,
			KMC_ACT_CHARTABLE,  6, GKEY_BACKSPACE, GKEY_TAB, GKEY_ENTER, GKEY_ESC, GKEY_SPACE, GKEY_DEL,
			KMC_ACT_DONE,
		/* 40 */KMC_RECORDSTART, 35,												// Special Keys
			KMC_TEST_CODETABLE, 14, VK_PRIOR, VK_NEXT,
									VK_HOME, VK_END,
									VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
									VK_INSERT,
									VK_SNAPSHOT, VK_SLEEP, VK_PAUSE, VK_CANCEL,
									VK_APPS,
			KMC_ACT_STATEBIT, GKEYSTATE_SPECIAL_BIT,
			KMC_ACT_CHARTABLE,  14, GKEY_PAGEUP, GKEY_PAGEDOWN,
									GKEY_HOME, GKEY_END,
									GKEY_LEFT, GKEY_RIGHT, GKEY_UP, GKEY_DOWN,
									GKEY_INSERT,
									GKEY_PRINTSCREEN, GKEY_SLEEP, GKEY_CTRLPAUSE, GKEY_CTRLBREAK,
									GKEY_RIGHTCLICKKEY,
			KMC_ACT_DONE,
		/* 41 */KMC_RECORDSTART, 8,													// F1 .. F15
			KMC_TEST_CODERANGE, VK_F1, VK_F15,
			KMC_ACT_STATEBIT, GKEYSTATE_SPECIAL_BIT,
			KMC_ACT_CHARRANGE, GKEY_FN1,
			KMC_ACT_DONE,

		// Anything else
		/* 40 */KMC_RECORDSTART, 1,													// Just send the scan code to the user
			KMC_ACT_DONE,

		// EOF
		KMC_RECORDSTART, 0
	};
#elif !GKEYBOARD_LAYOUT_OFF
	#warning "The WIN32 keyboard driver is not using the layout engine. If no other keyboard is using it consider defining GKEYBOARD_LAYOUT_OFF=GFXON to save code size."
#endif

// Forward definitions
static bool_t Win32KeyboardInit(GKeyboard *k, unsigned driverinstance);
static int Win32KeyboardGetData(GKeyboard *k, uint8_t *pch, int sz);

const GKeyboardVMT const GKEYBOARD_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_KEYBOARD,
		GKEYBOARD_VFLG_NOPOLL,			//  GKEYBOARD_VFLG_DYNAMICONLY
		sizeof(GKeyboard),
		_gkeyboardInitDriver, _gkeyboardPostInitDriver, _gkeyboardDeInitDriver
	},

 	// The Win32 keyboard layout
	#if GKEYBOARD_WIN32_NO_LAYOUT
		0,
	#else
		GKEYBOARD_WIN32_DEFAULT_LAYOUT,
	#endif

	Win32KeyboardInit,		// init
	0,						// deinit
	Win32KeyboardGetData,	// getdata
	0						// putdata		void	(*putdata)(GKeyboard *k, char ch);		Optional
}};

static int			keypos;
static uint8_t		keybuffer[8];
static GKeyboard	*keyboard;

static bool_t Win32KeyboardInit(GKeyboard *k, unsigned driverinstance) {
	(void)	driverinstance;

	// Only one please
	if (keyboard)
		return GFalse;

	keyboard = k;
	return GTrue;
}

static int Win32KeyboardGetData(GKeyboard *k, uint8_t *pch, int sz) {
	int		i, j;
	(void)	k;

	if (!keypos)
		return 0;

	for(i = 0; i < keypos && i < sz; i++)
		pch[i] = keybuffer[i];
	keypos -= i;
	for(j=0; j < keypos; j++)
		keybuffer[j] = keybuffer[i+j];
	return i;
}

static bool_t Win32KeyboardMsgHook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch(Msg) {
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		// A layout is being used: Send scan codes to the keyboard buffer
		if (keyboard && keyboard->pLayout && keypos < (int)sizeof(keybuffer)-1 && (wParam & 0xFF) > 0x01) {
			if (Msg == WM_KEYUP || Msg == WM_SYSKEYUP)
				keybuffer[keypos++] = 0x00;			// Keyup
			else if (HIWORD(lParam) & KF_REPEAT)
				keybuffer[keypos++] = 0x01;			// Repeat
			keybuffer[keypos++] = wParam;
			if ((gkvmt(keyboard)->d.flags & GKEYBOARD_VFLG_NOPOLL))		// For normal setup this is always GTrue
				_gkeyboardWakeup(keyboard);
		}
		return GTrue;
	case WM_CHAR:
		// A layout is not being used: Send character codes to the keyboard buffer
		if (keyboard && !keyboard->pLayout && keypos < (int)sizeof(keybuffer)) {
			wchar_t	w;
			int		len;

			// Convert from a UTF16 character to a UTF8 string.
			w = wParam;
			len = WideCharToMultiByte(CP_UTF8, 0, &w, 1, (char *)(keybuffer+keypos), sizeof(keybuffer)-keypos, 0, 0);
			keypos += len;
			if (len && (gkvmt(keyboard)->d.flags & GKEYBOARD_VFLG_NOPOLL))		// For normal setup this is always GTrue
				_gkeyboardWakeup(keyboard);
		}
		return GTrue;
	/*
	case WM_DEADCHAR:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
		break;
	*/
	}
	return GFalse;
}
