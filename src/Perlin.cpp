/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*	Vignette.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision History

	Version		Change													Engineer	Date
	=======		======													========	======
	1.0			(seemed like a good idea at the time)					bbb			6/1/2002

	1.0			Okay, I'm leaving the version at 1.0,					bbb			2/15/2006
				for obvious reasons; you're going to 
				copy these files directly! This is the
				first XCode version, though.

	1.0			Let's simplify this barebones sample					zal			11/11/2010

	1.0			Added new entry point									zal			9/18/2017

*/

#include "Perlin.h"

PF_Pixel32 changePropsToVUYA32f(PF_Pixel32 inputPixel) {
	PF_Pixel32 returnPixel;
	PF_Pixel_VUYA_32f convertedPixel;

	convertedPixel.luma = inputPixel.green;
	convertedPixel.Pb = inputPixel.red;
	convertedPixel.Pr = inputPixel.alpha;
	convertedPixel.alpha = inputPixel.blue;

	PF_FpLong R = convertedPixel.luma + 1.403 * convertedPixel.Pr;
	PF_FpLong G = convertedPixel.luma - 0.344 * convertedPixel.Pb - 0.714 * convertedPixel.Pr;
	PF_FpLong B = convertedPixel.luma + 1.77 * convertedPixel.Pb;

	if (R < 0) {
		R = 0;
	}
	if (R > 1.0) {
		R = 1.0;
	}

	if (G < 0) {
		G = 0;
	}
	if (G > 1.0) {
		G = 1.0;
	}

	if (B < 0) {
		B = 0;
	}
	if (B > 1.0) {
		B = 1.0;
	}

	returnPixel.red = R;
	returnPixel.green = G;
	returnPixel.blue = B;
	returnPixel.alpha = 1.0;

	return returnPixel;
}

static PF_Err
convertRGB32ToVUYA32f(
	PF_Pixel32 colour32, PF_Pixel_VUYA_32f* colourVUYA)
{
	PF_Err		err = PF_Err_NONE;


	colourVUYA->luma = 0.299 * colour32.red + 0.587 * colour32.green + 0.114 * colour32.blue;
	colourVUYA->Pb = (colour32.blue - colourVUYA->luma) * 0.565;
	colourVUYA->Pr = (colour32.red - colourVUYA->luma) * 0.713;
	colourVUYA->alpha = 1.0;

	return err;
}


static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;	// just 16bpc, not 32bpc

	out_data->out_flags2 = PF_OutFlag2_FLOAT_COLOR_AWARE |
		PF_OutFlag2_SUPPORTS_SMART_RENDER | PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

	if (in_data->appl_id == 'PrMr') {
		AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite = AEFX_SuiteScoper<PF_PixelFormatSuite1>(in_data, kPFPixelFormatSuite, kPFPixelFormatSuiteVersion1, out_data);

		// add supported pixel formats
		(*pixelFormatSuite->ClearSupportedPixelFormats)(in_data->effect_ref);

		(*pixelFormatSuite->AddSupportedPixelFormat) (in_data->effect_ref, PrPixelFormat_VUYA_4444_32f);  
		(*pixelFormatSuite->AddSupportedPixelFormat) (in_data->effect_ref, PrPixelFormat_BGRA_4444_32f);  
		(*pixelFormatSuite->AddSupportedPixelFormat) (in_data->effect_ref, PrPixelFormat_VUYA_4444_32f);  
		(*pixelFormatSuite->AddSupportedPixelFormat) (in_data->effect_ref, PrPixelFormat_BGRA_4444_8u);  
		
	}
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	/*AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOXX("Palette Outline", 0, NULL, OUTLINEENABLED_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_COLOR("Outline Colour", 255, 255, 255, OUTLINECOLOUR_DISK_ID);*/

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX("Scale", 0, 10000, 0, 10000, 5000, PF_Precision_INTEGER, 0, NULL, SCALE_DISK_ID);
	
	out_data->num_params = PERLIN_NUM_PARAMS;

	return err;
}

double lerp(double t, double a, double b) {
	return a + t * (b - a);
}

double fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

int p[256] = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

double noise(double x, double y, double z) {
	// Find the unit cube that contains the point
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)), lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))), lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)), lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
	return (res + 1.0) / 2.0;
}

static PF_Err
Perlin32(
	void* refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel32* inP,
	PF_Pixel32* outP)
{
	PF_Err		err = PF_Err_NONE;

	PerlinInfo* giP = reinterpret_cast<PerlinInfo*>(refcon);

	double x = (double(xL)+ rand() % 100) / double(1920.0);
	double y = (double(yL)+ rand() % 100) / double(1080.0);
	double n = noise(double(giP->scale / 100.0) * x, double(giP->scale / 100.0) * y, .2);

	//n = 20 * noise(x, y, giP->randomSeedInt);
	//n = n - floor(n);
	//n = (rand() % 100 + 1) * .01;
	//n = n - floor(n);

	outP->red = n;
	outP->green = n;
	outP->blue = n;
	outP->alpha = 1.0;



	return err;
}

static PF_Err
ActuallyRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_LayerDef* output,
	PF_EffectWorld* input,
	PF_ParamDef* params[])
{
	PF_Err				err = PF_Err_NONE;
	PF_Err				err2 = PF_Err_NONE;
	PF_PixelFormat format = PF_PixelFormat_INVALID;
	PF_WorldSuite2* wsP = NULL;

	A_Rect* src_rect;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	PerlinInfo			biP;
	AEFX_CLR_STRUCT(biP);
	A_long				linesL = 0;



	linesL = output->extent_hint.bottom - output->extent_hint.top;

	
	PF_NewWorldFlags	flags = PF_NewWorldFlag_CLEAR_PIXELS;
	PF_Boolean			deepB = PF_WORLD_IS_DEEP(output);

	if (deepB) {
		flags |= PF_NewWorldFlag_DEEP_PIXELS;
	}


	ERR(AEFX_AcquireSuite(in_data,
		out_data,
		kPFWorldSuite,
		kPFWorldSuiteVersion2,
		"Couldn't load suite.",
		(void**)&wsP));

	biP.scale = params[PERLIN_SCALE]->u.fs_d.value;


	

	if (!err) {
		// values reset

		ERR(wsP->PF_GetPixelFormat(input, &format));

		// pixel depth switch
		switch (format) {
		case PF_PixelFormat_ARGB128:

			ERR(suites.IterateFloatSuite1()->iterate(in_data,
				0,								// progress base
				output->height,							// progress final
				input,	// src 
				NULL,							// area - null for all pixels
				(void*)&biP,					// refcon - your custom data pointer
				Perlin32,				// pixel function pointer
				output));

			break;
		case PF_PixelFormat_ARGB64:
		
			
			break;
		case PF_PixelFormat_ARGB32:
			

			break;
		}

		
	}


	ERR2(AEFX_ReleaseSuite(in_data,
		out_data,
		kPFWorldSuite,
		kPFWorldSuiteVersion2,
		"Coludn't release suite."));

	return err;
}

static PF_Err
IterateFloat(
	PF_InData* in_data,
	long				progress_base,
	long				progress_final,
	PF_EffectWorld* src,
	void* refcon,
	PF_Err(*pix_fn)(void* refcon, A_long x, A_long y, PF_PixelFloat* in, PF_PixelFloat* out),
	PF_EffectWorld* dst)
{
	PF_Err	err = PF_Err_NONE;
	char* localSrc, * localDst;
	localSrc = reinterpret_cast<char*>(src->data);
	localDst = reinterpret_cast<char*>(dst->data);

	for (int y = progress_base; y < progress_final; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pix_fn(refcon,
				static_cast<A_long> (x),
				static_cast<A_long> (y),
				reinterpret_cast<PF_PixelFloat*>(localSrc),
				reinterpret_cast<PF_PixelFloat*>(localDst));
			localSrc += 16;
			localDst += 16;
		}
		localSrc += (src->rowbytes - src->width * 16);
		localDst += (dst->rowbytes - src->width * 16);
	}

	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	PerlinInfo			biP;
	AEFX_CLR_STRUCT(biP);
	A_long				linesL	= 0;

	linesL = output->extent_hint.bottom - output->extent_hint.top;

	PF_WorldSuite2	*wsP = NULL;

	PF_ParamDef			checkout;
	AEFX_CLR_STRUCT(checkout);

	ERR(AEFX_AcquireSuite(in_data,
		out_data,
		kPFWorldSuite,
		kPFWorldSuiteVersion2,
		"Couldn't load suite.",
		(void**)&wsP));

	ERR(PF_CHECKOUT_PARAM(in_data,
		PERLIN_INPUT,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&checkout));


	if (in_data->appl_id == 'PrMr') {
		AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite =
			AEFX_SuiteScoper<PF_PixelFormatSuite1>(in_data,
				kPFPixelFormatSuite,
				kPFPixelFormatSuiteVersion1,
				out_data);

		PrPixelFormat destinationPixelFormat = PrPixelFormat_BGRA_4444_8u;

		pixelFormatSuite->GetPixelFormat(output, &destinationPixelFormat);

		AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite =
			AEFX_SuiteScoper<PF_Iterate8Suite1>(in_data,
				kPFIterate8Suite,
				kPFIterate8SuiteVersion1,
				out_data);

		switch (destinationPixelFormat)
		{

		case PrPixelFormat_BGRA_4444_8u:


			//if (in_data->quality == 1) {


				

			break;
		case PrPixelFormat_VUYA_4444_8u:


			

			break;
		case PrPixelFormat_BGRA_4444_32f:
			

			break;
		case PrPixelFormat_VUYA_4444_32f:
			
			break;
		default:
			//	Return error, because we don't know how to handle the specified pixel type
			return PF_Err_UNRECOGNIZED_PARAM_TYPE;
		}

	}
	//ERR(PF_CHECKIN_PARAM(in_data, &params[PERLIN_INPUT]->u.ld));

	return err;
}

static PF_Err
PreRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_PreRenderExtra* extra)
{
	PF_Err err = PF_Err_NONE;
	PF_ParamDef channel_param;
	PF_RenderRequest req = extra->input->output_request;
	PF_CheckoutResult in_result;

	//AEFX_CLR_STRUCT(channel_param);

	AEGP_SuiteHandler suites(in_data->pica_basicP);


	// Mix in the background color of the comp, as a demonstration of GuidMixInPtr()
	// When the background color changes, the effect will need to be rerendered.
	// Note: This doesn't handle the collapsed comp case
	// Your effect can use a similar approach to trigger a rerender based on changes beyond just its effect parameters.

	req.channel_mask = PF_ChannelMask_RED | PF_ChannelMask_GREEN | PF_ChannelMask_BLUE;

	req.preserve_rgb_of_zero_alpha = FALSE;	//	Hey, we dont care.N


	ERR(extra->cb->checkout_layer(in_data->effect_ref,
		PERLIN_INPUT,
		PERLIN_INPUT,
		&req,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&in_result));

	UnionLRect(&in_result.result_rect, &extra->output->result_rect);
	UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);

	A_Time	thisTime;
	AEGP_ItemH item;
	
	
	/*for (int t = 0; t < 30; t++) {
		err = suites.AdvItemSuite1()->PF_MoveTimeStepActiveItem(PF_Step_FORWARD, 60);
	}*/
	/*err = suites.ItemSuite9()->AEGP_GetActiveItem(&item);
	do {
		err = suites.AdvItemSuite1()->PF_MoveTimeStepActiveItem(PF_Step_FORWARD, 20);
		err = suites.ItemSuite9()->AEGP_GetItemCurrentTime(item, &thisTime);
	} while (thisTime.value < in_data->total_time*.5);*/
	//err = suites.AdvItemSuite1()->PF_MoveTimeStepActiveItem(PF_Step_FORWARD, 0);
	//err = suites.AdvItemSuite1()->PF_ForceRerender(in_data, );
	/*err = suites.ItemSuite9()->AEGP_GetActiveItem(&item);
	err = suites.ItemSuite9()->AEGP_GetItemCurrentTime(item, &thisTime);
	thisTime.value += thisTime.scale*40;
	const A_Time newTime = thisTime;
	err = suites.ItemSuite9()->AEGP_SetItemCurrentTime(item, &newTime);*/

	//	Notice something missing, namely the PF_CHECKIN_PARAM to balance
	//	the old-fashioned PF_CHECKOUT_PARAM, above? 

	//	For SmartFX, AE automagically checks in any params checked out 
	//	during PF_Cmd_SMART_PRE_RENDER, new or old-fashioned.

	return err;
}

static PF_Err
SmartRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_SmartRenderExtra* extra)

{

	PF_Err			err = PF_Err_NONE,
		err2 = PF_Err_NONE;

	PF_EffectWorld* input_worldP = NULL;
	PF_EffectWorld* output_worldP = NULL;

	PF_ParamDef params[PERLIN_NUM_PARAMS];
	PF_ParamDef* paramsP[PERLIN_NUM_PARAMS];

	AEFX_CLR_STRUCT(params);

	for (int i = 0; i < PERLIN_NUM_PARAMS; i++) {
		paramsP[i] = &params[i];
	}

	ERR((extra->cb->checkout_layer_pixels(in_data->effect_ref, PERLIN_INPUT, &input_worldP)));
	ERR(extra->cb->checkout_output(in_data->effect_ref, &output_worldP));

	ERR(PF_CHECKOUT_PARAM(in_data,
		SCALE_DISK_ID,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&params[PERLIN_SCALE]));


	ERR(ActuallyRender(in_data,
		out_data,
		output_worldP,
		input_worldP,
		paramsP));

	ERR2(PF_CHECKIN_PARAM(in_data, &params[PERLIN_SCALE]));

	return err;

}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"Perlin", // Name
		"NT Perlin", // Match Name
		"NT Productions", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;

			case PF_Cmd_SMART_PRE_RENDER:
				err = PreRender(in_data, out_data, (PF_PreRenderExtra*)extra);
				break;

			case PF_Cmd_SMART_RENDER:
				err = SmartRender(in_data, out_data, (PF_SmartRenderExtra*)extra);
				break;

			
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

