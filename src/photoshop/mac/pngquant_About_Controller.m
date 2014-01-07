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

#import "pngquant_About_Controller.h"

@implementation pngquant_About_Controller

- (id)init:(const char *)pngquantVersion
{
	self = [super init];

	if(!([NSBundle loadNibNamed:@"pngquant_About_Dialog" owner:self]))
		return nil;

    [theWindow setDelegate:self];

    [about setString:@"LibImageQuant (http://pngquant.org):\n"
     "\n"
     "© 2009-2014 by Kornel Lesiński.\n"
     "© 1989, 1991 by Jef Poskanzer.\n"
     "© 1997, 2000, 2002 by Greg Roelofs; based on an idea by Stefan Schneider.\n"
     "\n"
     "Permission to use, copy, modify, and distribute this software and its documentation for any purpose and without fee is hereby granted, provided that the above copyright notice appear in all copies and that both that copyright notice and this permission notice appear in supporting documentation.  This software is provided \"as is\" without express or implied warranty.\n\n"
     "The photoshop plug-in:\n\n"
     "© 2014 Kornel Lesiński\n"
     "© 2013 Brendan Bolles\n"
     "\n"
     "All rights reserved.\n"
     "\n"
     "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n"
     "\n"
     "*  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n"
     "*  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n"
     "\n"
     "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."];

	[pngquantVersionString setStringValue:[NSString stringWithUTF8String:pngquantVersion]];
	[theWindow center];

	return self;
}

- (void)windowWillClose:(NSNotification *)notification {
	[NSApp stopModal];
}

- (NSWindow *)getWindow {
	return theWindow;
}

@end
