/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

//--------------------------------------------------------------------------
// Configuration options for this driver
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Toggle Driver
//--------------------------------------------------------------------------

/* Include toggle support code */
#include "../../../src/ginput/ginput_driver_toggle.h"

static uint8_t	toggles;

#define WIN32_TOGGLE_TITLE		"uGFX Toggles"
#define WIN32_TOGGLE_WIDTH		320
#define WIN32_TOGGLE_HEIGHT		16

#if GINPUT_TOGGLE_CONFIG_ENTRIES > 1
	#error "GDISP Win32: GINPUT_TOGGLE_CONFIG_ENTRIES must be 1 until Toggles can use GDriver"
#endif

const GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES];

void ginput_lld_toggle_init(const GToggleConfig *ptc) {
	// Save the associated window struct
	//ptc->id = &GDISP_WIN32[ptc - GInputToggleConfigTable];
	((GToggleConfig *)ptc)->id = 0;

	// We have 8 buttons per window.
	((GToggleConfig *)ptc)->mask = 0xFF;

	// No inverse or special mode
	((GToggleConfig *)ptc)->invert = 0x00;
	((GToggleConfig *)ptc)->mode = 0;
}

unsigned ginput_lld_toggle_getbits(const GToggleConfig *ptc) {
	(void)		ptc;

	return toggles;
}

static LRESULT Win32ToggleWinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				dc;
	PAINTSTRUCT		ps;
	HBRUSH			hbrOn, hbrOff;
	HPEN			pen;
	RECT			rect;
	HGDIOBJ			old;
	POINT 			p;
	gCoord			pos;
	uint8_t			bit;

	switch (Msg) {
	case WM_CREATE:
		break;

	case WM_LBUTTONDOWN:
		bit = 1 << ((gCoord)LOWORD(lParam)*8/WIN32_TOGGLE_WIDTH);
		toggles ^= bit;
		rect.left = 0;
		rect.right = WIN32_TOGGLE_WIDTH;
		rect.top = 0;
		rect.bottom = WIN32_TOGGLE_HEIGHT;
		InvalidateRect(hWnd, &rect, FALSE);
		UpdateWindow(hWnd);
		#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
			ginputToggleWakeup();
		#endif
		break;

	case WM_LBUTTONUP:
		// Handle mouse up on the toggle area
		if ((toggles & 0x0F)) {
			toggles &= ~0x0F;
			rect.left = 0;
			rect.right = WIN32_TOGGLE_WIDTH;
			rect.top = 0;
			rect.bottom = WIN32_TOGGLE_HEIGHT;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
				ginputToggleWakeup();
			#endif
		}
		break;

	case WM_ERASEBKGND:
		// Pretend we have erased the background.
		// We know we don't really need to do this as we
		// redraw the entire surface in the WM_PAINT handler.
		return TRUE;

	case WM_PAINT:
		// Paint the main window area
		dc = BeginPaint(hWnd, &ps);
		pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		hbrOn = CreateSolidBrush(RGB(0, 0, 255));
		hbrOff = CreateSolidBrush(RGB(128, 128, 128));
		old = SelectObject(dc, pen);
		for(pos = 0, bit=1; pos < WIN32_TOGGLE_WIDTH; pos=rect.right, bit <<= 1) {
			rect.left = pos;
			rect.right = pos + WIN32_TOGGLE_WIDTH/8;
			rect.top = 0;
			rect.bottom = WIN32_TOGGLE_HEIGHT;
			FillRect(dc, &rect, (toggles & bit) ? hbrOn : hbrOff);
			if (pos > 0) {
				MoveToEx(dc, rect.left, rect.top, &p);
				LineTo(dc, rect.left, rect.bottom);
			}
		}
		DeleteObject(hbrOn);
		DeleteObject(hbrOff);
		SelectObject(dc, old);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		// Quit the application
		PostQuitMessage(0);

		// Actually the above doesn't work (who knows why)
		ExitProcess(0);
		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}
