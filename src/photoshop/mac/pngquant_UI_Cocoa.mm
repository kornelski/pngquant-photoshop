///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013, Brendan Bolles
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *	   Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *	   Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
//
// pngquant Photoshop plug-in
//
// by Kornel Lesinski <kornel@pngquant.org>
// based on code by Brendan Bolles <brendan@fnordware.com>
//
// ------------------------------------------------------------------------

#include "pngquant_UI.h"

#import "pngquant_About_Controller.h"


// ==========
// Only building this on 64-bit (Cocoa) architectures
// ==========
#if __LP64__


void MyLog(const char *str) {
    NSLog(@"%s", str);
}

void
pngquant_About(
	const char		*pngquant_version_string,
	const void		*plugHndl,
	const void		*mwnd)
{

		pngquant_About_Controller *about_controller = [[pngquant_About_Controller alloc] init:pngquant_version_string];

		if(about_controller)
		{
			NSWindow *the_window = [about_controller getWindow];

			if(the_window)
			{
				[NSApp runModalForWindow:the_window];

				[the_window close];
			}  

			[about_controller release];
		}
}

#endif // __LP64__
