/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "gfx.h"

/* The variables we need */
static font_t		font;
static GListener	gl;
static GHandle		ghConsole;
static GHandle		ghKeyboard;


/**
 * Create the widgets.
 */
static void createWidgets(void) {
	GWidgetInit		wi;

	gwinWidgetClearInit(&wi);

	// Create the console - set colors before making it visible
	wi.g.show = GFalse;
	wi.g.x = 0; wi.g.y = 0;
	wi.g.width = gdispGetWidth(); wi.g.height = gdispGetHeight()/2;
	ghConsole = gwinConsoleCreate(0, &wi.g);
	gwinSetColor(ghConsole, Black);
	gwinSetBgColor(ghConsole, HTML2COLOR(0xF0F0F0));
	gwinShow(ghConsole);
	gwinClear(ghConsole);

	// Create the keyboard
	wi.g.show = GTrue;
	wi.g.x = 0; wi.g.y = gdispGetHeight()/2;
	wi.g.width = gdispGetWidth(); wi.g.height = gdispGetHeight()/2;
	ghKeyboard = gwinKeyboardCreate(0, &wi);
}

int main(void) {
	GEvent *			pe;
	GEventKeyboard *	pk;
	unsigned			i;

	// Initialize the display
	gfxInit();

	// Set the widget defaults
	font = gdispOpenFont("*");			// Get the first defined font.
	gwinSetDefaultFont(font);
	gwinSetDefaultStyle(&WhiteWidgetStyle, GFalse);
	gdispClear(White);

	// Create the gwin windows/widgets
	createWidgets();

    // We want to listen for widget events
	geventListenerInit(&gl);
	gwinAttachListener(&gl);

	// We also want to listen to keyboard events from the virtual keyboard
	geventAttachSource(&gl, gwinKeyboardGetEventSource(ghKeyboard), GLISTEN_KEYTRANSITIONS|GLISTEN_KEYUP);

	while(1) {
		// Get an Event
		pe = geventEventWait(&gl, TIME_INFINITE);

		switch(pe->type) {
		case GEVENT_GWIN_KEYBOARD:
			// This is a widget event generated on the standard gwin event source
			gwinPrintf(ghConsole, "Keyboard visibility has changed\n");
			break;

		case GEVENT_KEYBOARD:
			// This is a keyboard event from a keyboard source which must be separately listened to.
			// It is not sent on the gwin event source even though in this case it was generated by a gwin widget.
			pk = (GEventKeyboard *)pe;

			gwinPrintf(ghConsole, "KEYSTATE: 0x%04X [ %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s]",
				pk->keystate,
				(!pk->keystate ? "NONE " : ""),
				((pk->keystate & GKEYSTATE_KEYUP) ? "KEYUP " : ""),
				((pk->keystate & GKEYSTATE_REPEAT) ? "REPEAT " : ""),
				((pk->keystate & GKEYSTATE_SPECIAL) ? "SPECIAL " : ""),
				((pk->keystate & GKEYSTATE_RAW) ? "RAW " : ""),
				((pk->keystate & GKEYSTATE_SHIFT_L) ? "LSHIFT " : ""),
				((pk->keystate & GKEYSTATE_SHIFT_R) ? "RSHIFT " : ""),
				((pk->keystate & GKEYSTATE_CTRL_L) ? "LCTRL " : ""),
				((pk->keystate & GKEYSTATE_CTRL_R) ? "RCTRL " : ""),
				((pk->keystate & GKEYSTATE_ALT_L) ? "LALT " : ""),
				((pk->keystate & GKEYSTATE_ALT_R) ? "RALT " : ""),
				((pk->keystate & GKEYSTATE_FN) ? "FN " : ""),
				((pk->keystate & GKEYSTATE_COMPOSE) ? "COMPOSE " : ""),
				((pk->keystate & GKEYSTATE_WINKEY) ? "WINKEY " : ""),
				((pk->keystate & GKEYSTATE_CAPSLOCK) ? "CAPSLOCK " : ""),
				((pk->keystate & GKEYSTATE_NUMLOCK) ? "NUMLOCK " : ""),
				((pk->keystate & GKEYSTATE_SCROLLLOCK) ? "SCROLLLOCK " : "")
				);
			if (pk->bytecount) {
				gwinPrintf(ghConsole, " Keys:");
				for (i = 0; i < pk->bytecount; i++)
					gwinPrintf(ghConsole, " 0x%02X", (uint8_t)pk->c[i]);
				gwinPrintf(ghConsole, " [");
				for (i = 0; i < pk->bytecount; i++)
					gwinPrintf(ghConsole, "%c", pk->c[i] >= ' ' && pk->c[i] <= '~' ? pk->c[i] : ' ');
				gwinPrintf(ghConsole, "]");
			}
			gwinPrintf(ghConsole, "\n");
			break;

		default:
			gwinPrintf(ghConsole, "Unknown %d\n", pe->type);
			break;
		}
	}
	return 0;
}

