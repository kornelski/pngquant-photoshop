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

#include "pngquant.h"

#include "pngquant_version.h"
#include "pngquant_UI.h"
#include "libimagequant.h"

#include <stdio.h>
#include <assert.h>

#include "lodepng.h"
#include <stdlib.h>


// globals needed by a bunch of Photoshop SDK routines

SPBasicSuite * sSPBasic = NULL;
SPPluginRef gPlugInRef = NULL;


static void DoAbout(AboutRecordPtr aboutP)
{
	const char * const plugHndl = "org.pngquant.photoshop";
	const void *hwnd = aboutP;

	pngquant_About("Plug-in " pngquant_Version_String ", libpngquant 2.0.1.", plugHndl, hwnd);
}

#pragma mark-

static void InitGlobals(Ptr globalPtr)
{
	// create "globals" as a our struct global pointer so that any
	// macros work:
	GPtr globals = (GPtr)globalPtr;

	globals->fileH				= NULL;

	memset(&gOptions, 0, sizeof(gOptions));

	gOptions.quality			= 50;
	gOptions.save_metadata		= TRUE;
}


static Handle myNewHandle(GPtr globals, const int32 inSize)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->newProc != NULL)
	{
		return gStuff->handleProcs->newProc(inSize);
	}
	else
	{
		return PINewHandle(inSize);
	}
}

static Ptr myLockHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->lockProc)
	{
		return gStuff->handleProcs->lockProc(h, TRUE);
	}
	else
	{
		return PILockHandle(h, TRUE);
	}
}

static void myUnlockHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->unlockProc)
	{
		gStuff->handleProcs->unlockProc(h);
	}
	else
	{
		PIUnlockHandle(h);
	}
}

static int32 myGetHandleSize(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->getSizeProc)
	{
		return gStuff->handleProcs->getSizeProc(h);
	}
	else
	{
		return PIGetHandleSize(h);
	}
}

static void mySetHandleSize(GPtr globals, Handle h, const int32 inSize)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->setSizeProc)
	{
		gStuff->handleProcs->setSizeProc(h, inSize);
	}
	else
	{
		PISetHandleSize(h, inSize);
	}
}

static void myDisposeHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->disposeProc != NULL)
	{
		gStuff->handleProcs->disposeProc(h);
	}
	else
	{
		PIDisposeHandle(h);
	}
}

static OSErr myAllocateBuffer(GPtr globals, const int32 inSize, BufferID *outBufferID)
{
	*outBufferID = 0;

	if(gStuff->bufferProcs != NULL && gStuff->bufferProcs->numBufferProcs >= 4 && gStuff->bufferProcs->allocateProc != NULL)
		gResult = gStuff->bufferProcs->allocateProc(inSize, outBufferID);
	else
		gResult = memFullErr;

	return gResult;
}

static Ptr myLockBuffer(GPtr globals, const BufferID inBufferID, Boolean inMoveHigh)
{
	if(gStuff->bufferProcs != NULL && gStuff->bufferProcs->numBufferProcs >= 4 && gStuff->bufferProcs->lockProc != NULL)
		return gStuff->bufferProcs->lockProc(inBufferID, inMoveHigh);
	else
		return NULL;
}

static void myFreeBuffer(GPtr globals, const BufferID inBufferID)
{
	if(gStuff->bufferProcs != NULL && gStuff->bufferProcs->numBufferProcs >= 4 && gStuff->bufferProcs->freeProc != NULL)
		gStuff->bufferProcs->freeProc(inBufferID);
}


#pragma mark-



template <typename T>
static bool ReadParams(GPtr globals, T *options)
{
	bool found_revert = FALSE;

	if( gStuff->revertInfo != NULL )
	{
		if( myGetHandleSize(globals, gStuff->revertInfo) == sizeof(T) )
		{
			T *flat_options = (T *)myLockHandle(globals, gStuff->revertInfo);

			memcpy((char*)options, (char*)flat_options, sizeof(T) );

			myUnlockHandle(globals, gStuff->revertInfo);

			found_revert = TRUE;
		}
	}

	return found_revert;
}

template <typename T>
static void WriteParams(GPtr globals, T *options)
{
	T *flat_options = NULL;

	if(gStuff->hostNewHdl != NULL) // we have the handle function
	{
		if(gStuff->revertInfo == NULL)
		{
			gStuff->revertInfo = myNewHandle(globals, sizeof(T) );
		}
		else
		{
			if(myGetHandleSize(globals, gStuff->revertInfo) != sizeof(T)  )
				mySetHandleSize(globals, gStuff->revertInfo, sizeof(T) );
		}

		flat_options = (T *)myLockHandle(globals, gStuff->revertInfo);

		memcpy((char*)flat_options, (char*)options, sizeof(T) );


		myUnlockHandle(globals, gStuff->revertInfo);
	}
}

typedef struct {
	unsigned8	r;
	unsigned8	g;
	unsigned8	b;
	unsigned8	a;
} RGBApixel8;

#pragma mark-

static void DoNothing(GPtr globals)
{
}

static void DoOptionsStart(GPtr globals)
{
	ReadParams(globals, &gOptions);

	if( ReadScriptParamsOnWrite(globals) )
	{
		bool have_transparency = false;
		const char *alpha_name = NULL;

		if(gStuff->hostSig == '8BIM')
			have_transparency = (gStuff->documentInfo && gStuff->documentInfo->mergedTransparency);
		else
			have_transparency = (gStuff->planes == 2 || gStuff->planes == 4);


		if(gStuff->documentInfo && gStuff->documentInfo->alphaChannels)
			alpha_name = gStuff->documentInfo->alphaChannels->name;

		pngquant_OutUI_Data params;

		params.quality			= gOptions.quality;
		params.save_metadata	= gOptions.save_metadata;

		const char * const plugHndl = "org.pngquant.photoshop";
		const void *hwnd = globals;

		bool result = pngquant_OutUI(&params, have_transparency, alpha_name, plugHndl, hwnd);

		if(result)
		{
			gOptions.quality		= params.quality;
			gOptions.save_metadata	= params.save_metadata;

			WriteParams(globals, &gOptions);
			WriteScriptParamsOnWrite(globals);
		}
		else
			gResult = userCanceledErr;

	}
}

#pragma mark-

static void DoEstimateStart(GPtr globals)
{
	if(gStuff->HostSupports32BitCoordinates && gStuff->imageSize32.h && gStuff->imageSize32.v)
		gStuff->PluginUsing32BitCoordinates = TRUE;

	int width = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.h : gStuff->imageSize.h);
	int height = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.v : gStuff->imageSize.v);

	int64 dataBytes = (int64)width * (int64)height * (int64)gStuff->planes * (int64)(gStuff->depth >> 3);

	gStuff->minDataBytes = dataBytes;
	gStuff->maxDataBytes = dataBytes + 5000;

	gStuff->data = NULL;
}

#pragma mark-

static void DoWritePrepare(GPtr globals)
{
	gStuff->maxData = 0;
}

static int progress;
static int ProgressReport(int percent, GPtr globals)
{
    if (gResult != noErr) return 0;

    progress = percent;
	PIUpdateProgress(percent, 100);

    gResult = AdvanceState();
    if (gResult != noErr) return 0;

    gResult = TestAbort();
    if (gResult != noErr) return 0;

    return 1;
}

static void mycallback(const liq_attr *a, const char *message, void* user_info) {
    GPtr globals = (GPtr)user_info;

    PIUpdateProgress(progress++, 100);
    AdvanceState();

    MyLog(message);
}

static void rgb_to_rgba_callback(liq_color row_out[], int row_index, int width, void *user_info) {
    FormatRecordPtr g = (FormatRecordPtr)user_info;

    unsigned char *rgb_row = ((unsigned char *)g->data) + g->rowBytes * row_index;

    for(int i=0; i < width; i++) {
        row_out[i].r = rgb_row[i*3];
        row_out[i].g = rgb_row[i*3+1];
        row_out[i].b = rgb_row[i*3+2];
        row_out[i].a = 255;
    }
}

static BufferID bufferID = 0;

static void DoWriteStart(GPtr globals)
{
	ReadParams(globals, &gOptions);
	ReadScriptParamsOnWrite(globals);

	assert(gStuff->imageMode == plugInModeRGBColor);
	assert(gStuff->depth == 8);
	assert(gStuff->planes >= 3);

	bool have_transparency = (gStuff->planes >= 4);
	bool have_alpha_channel = (gStuff->channelPortProcs && gStuff->documentInfo && gStuff->documentInfo->alphaChannels);

	bool use_alpha = (have_transparency || have_alpha_channel);

	const int width = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.h : gStuff->imageSize.h);
	const int height = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.v : gStuff->imageSize.v);

	gStuff->loPlane = 0;
	gStuff->hiPlane = (have_transparency ? 3 : 2);
	gStuff->colBytes = sizeof(unsigned char) * (use_alpha ? 4 : 3);
	gStuff->rowBytes = gStuff->colBytes * width;
	gStuff->planeBytes = sizeof(unsigned char);

	gStuff->theRect.left = gStuff->theRect32.left = 0;
	gStuff->theRect.right = gStuff->theRect32.right = width;
	gStuff->theRect.top = gStuff->theRect32.top = 0;
	gStuff->theRect.bottom = gStuff->theRect32.bottom = height;

    progress=0;

	ReadPixelsProc ReadProc = NULL;
	ReadChannelDesc *alpha_channel = NULL;

	// ReadProc being non-null means we're going to get the channel from the channels palette
	if(use_alpha &&
		gStuff->channelPortProcs && gStuff->documentInfo && gStuff->documentInfo->alphaChannels)
	{
		ReadProc = gStuff->channelPortProcs->readPixelsProc;

		alpha_channel = gStuff->documentInfo->alphaChannels;
	}


	int32 buffer_size = gStuff->rowBytes * height;

	bufferID = 0;

	gResult = myAllocateBuffer(globals, buffer_size, &bufferID);

	if(gResult == noErr)
	{
		gStuff->data = myLockBuffer(globals, bufferID, TRUE);

        ProgressReport(1, globals);

		gResult = AdvanceState();

		if(gResult == noErr && ReadProc)
		{
			VRect wroteRect;
			VRect writeRect = { 0, 0, height, width };
			PSScaling scaling; scaling.sourceRect = scaling.destinationRect = writeRect;
			PixelMemoryDesc memDesc = { (char *)gStuff->data, gStuff->rowBytes * 8, gStuff->colBytes * 8, 3 * 8, gStuff->depth };

			gResult = ReadProc(alpha_channel->port, &scaling, &writeRect, &memDesc, &wroteRect);
		}
    }
}


static void DoWriteContinue(GPtr globals)
{
    liq_attr *attr = liq_attr_create();
    liq_set_log_callback(attr, mycallback, (void*)globals);

    const int width = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.h : gStuff->imageSize.h);
	const int height = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.v : gStuff->imageSize.v);

    if (width * height > 1024*1024) {
        liq_set_speed(attr, 5);
    } else if (width * height < 128*128) {
        liq_set_speed(attr, 2);
        liq_set_quality(attr, 0, 95);
    }

    liq_image *img;
    if (gStuff->planes == 4) {
        img = liq_image_create_rgba(attr, gStuff->data, width, height, 0);
    } else {
        img = liq_image_create_custom(attr, rgb_to_rgba_callback, gStuff, width, height, 0);
    }

    ProgressReport(2, globals);

    liq_result *res = liq_quantize_image(attr, img);

    size_t outbuf_size = width*height;
    unsigned char *outbuf = (unsigned char *)malloc(outbuf_size);

    liq_set_dithering_level(res, 1.0f);

    if (ProgressReport(50, globals)) liq_write_remapped_image(res, img, outbuf, outbuf_size);

    ProgressReport(80, globals);

    liq_image_destroy(img);

    const liq_palette *palette = liq_get_palette(res);

    LodePNGState state;
    lodepng_state_init(&state);

    state.info_raw.colortype = LCT_PALETTE;
    state.info_raw.bitdepth = 8;
    state.info_png.color.colortype = LCT_PALETTE;
    state.info_png.color.bitdepth = palette->count <= 16 ? 4 : 8;
    state.encoder.auto_convert = LAC_NO;

    for(int i=0; i < palette->count; i++) {
        lodepng_palette_add(&state.info_png.color, palette->entries[i].r, palette->entries[i].g, palette->entries[i].b, palette->entries[i].a);
        lodepng_palette_add(&state.info_raw, palette->entries[i].r, palette->entries[i].g, palette->entries[i].b, palette->entries[i].a);
    }

    unsigned char *compressed = NULL;
    size_t compressed_size = 0;
    lodepng_encode(&compressed, &compressed_size, outbuf, width, height, &state);

    free(outbuf);
    liq_result_destroy(res);

    if (ProgressReport(95, globals)) {
        ByteCount count = compressed_size;
        OSErr result = FSWriteFork(gStuff->dataFork, fsAtMark, 0, count, (const void *)compressed, &count);

        if (result != noErr || count != compressed_size) {
            gResult = formatBadParameters;
        }
    }
    free(compressed);

    liq_attr_destroy(attr);

    ProgressReport(100, globals);

    myFreeBuffer(globals, bufferID);
    gStuff->data = NULL;
    MyLog("Freed");
}


static void DoWriteFinish(GPtr globals)
{
	if(gStuff->hostSig != 'FXTC')
		WriteScriptParamsOnWrite(globals);
}

#pragma mark-


DLLExport MACPASCAL void PluginMain(const short selector,
						             FormatRecord *formatParamBlock,
						             intptr_t *data,
						             short *result)
{
	if (selector == formatSelectorAbout)
	{
		sSPBasic = ((AboutRecordPtr)formatParamBlock)->sSPBasic;

		DoAbout((AboutRecordPtr)formatParamBlock);
	}
	else
	{
		sSPBasic = formatParamBlock->sSPBasic;  //thanks Tom

		gPlugInRef = (SPPluginRef)formatParamBlock->plugInRef;

	 	static const FProc routineForSelector [] =
		{
			/* formatSelectorAbout  				DoAbout, */

			/* formatSelectorReadPrepare */			NULL,
			/* formatSelectorReadStart */			NULL,
			/* formatSelectorReadContinue */		NULL,
			/* formatSelectorReadFinish */			NULL,

			/* formatSelectorOptionsPrepare */		DoNothing,
			/* formatSelectorOptionsStart */		DoNothing,
			/* formatSelectorOptionsContinue */		NULL,
			/* formatSelectorOptionsFinish */		DoNothing,

			/* formatSelectorEstimatePrepare */		DoNothing,
			/* formatSelectorEstimateStart */		DoEstimateStart,
			/* formatSelectorEstimateContinue */	DoNothing,
			/* formatSelectorEstimateFinish */		DoNothing,

			/* formatSelectorWritePrepare */		DoWritePrepare,
			/* formatSelectorWriteStart */			DoWriteStart,
			/* formatSelectorWriteContinue */		DoWriteContinue,
			/* formatSelectorWriteFinish */			DoWriteFinish,
		};

		Ptr globalPtr = NULL;		// Pointer for global structure
		GPtr globals = NULL; 		// actual globals


		if(formatParamBlock->handleProcs)
		{
			bool must_init = false;

			if(!*data)
			{
				*data = (intptr_t)formatParamBlock->handleProcs->newProc(sizeof(Globals));

				must_init = true;
			}

			if(*data)
			{
				globalPtr = formatParamBlock->handleProcs->lockProc((Handle)*data, TRUE);

				if(must_init)
					InitGlobals(globalPtr);
			}
			else
			{
				*result = memFullErr;
				return;
			}

			globals = (GPtr)globalPtr;

			globals->result = result;
			globals->formatParamBlock = formatParamBlock;
		}
		else
		{
			// old lame way
			globalPtr = AllocateGlobals(result,
										 formatParamBlock,
										 formatParamBlock->handleProcs,
										 sizeof(Globals),
						 				 data,
						 				 InitGlobals);

			if(globalPtr == NULL)
			{ // Something bad happened if we couldn't allocate our pointer.
			  // Fortunately, everything's already been cleaned up,
			  // so all we have to do is report an error.

			  *result = memFullErr;
			  return;
			}

			// Get our "globals" variable assigned as a Global Pointer struct with the
			// data we've returned:
			globals = (GPtr)globalPtr;
		}


		// Dispatch selector
		if (selector > formatSelectorAbout && selector <= formatSelectorWriteFinish && routineForSelector[selector-1])
			(routineForSelector[selector-1])(globals); // dispatch using jump table
		else
			gResult = formatBadParameters;


		if((Handle)*data != NULL)
		{
			if(formatParamBlock->handleProcs)
			{
				formatParamBlock->handleProcs->unlockProc((Handle)*data);
			}
			else
			{
				PIUnlockHandle((Handle)*data);
			}
		}
	} // about selector special
}
