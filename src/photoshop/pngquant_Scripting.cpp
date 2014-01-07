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

#include "PIDefines.h"
#include "pngquant.h"

#include "pngquant_Terminology.h"


Boolean ReadScriptParamsOnWrite(GPtr globals)
{
	PIReadDescriptor			token = NULL;
	DescriptorKeyID				key = 0;
	DescriptorTypeID			type = 0;
	DescriptorKeyIDArray		array = { NULLID };
	int32						flags = 0;
	OSErr						stickyError = noErr;
	Boolean						returnValue = true;
	int32						storeValue;
	Boolean						boolStoreValue;

	if (DescriptorAvailable(NULL))
	{
		token = OpenReader(array);
		if (token)
		{
			while (PIGetKey(token, &key, &type, &flags))
			{
				switch (key)
				{
					case keypngquantquality:
							PIGetInt(token, &storeValue);
							gOptions.quality = storeValue;
							break;
				
					case keypngquantsaveMetadata:
							PIGetBool(token, &boolStoreValue);
							gOptions.save_metadata = boolStoreValue;
							break;
				}
			}

			stickyError = CloseReader(&token); // closes & disposes.

			if (stickyError)
			{
				if (stickyError == errMissingParameter) // missedParamErr == -1715
					;
					/* (descriptorKeyIDArray != NULL)
					   missing parameter somewhere.  Walk IDarray to find which one. */
				else
					gResult = stickyError;
			}
		}

		returnValue = PlayDialog();
		// return TRUE if want to show our Dialog
	}

	return returnValue;
}



OSErr WriteScriptParamsOnWrite(GPtr globals)
{
	PIWriteDescriptor			token = nil;
	OSErr						gotErr = noErr;

	if (DescriptorAvailable(NULL))
	{
		token = OpenWriter();
		if (token)
		{
			PIPutBool(token, keypngquantsaveMetadata, gOptions.save_metadata);

			gotErr = CloseWriter(&token); /* closes and sets dialog optional */
			/* done.  Now pass handle on to Photoshop */
		}
	}
	return gotErr;
}


