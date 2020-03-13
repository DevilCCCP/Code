#pragma once

//#define UseVdpFunctionOne(VdpId, VdpFunc) ...

#define UseVdpFunctionList \
  UseVdpFunctionOne(VDP_FUNC_ID_GET_ERROR_STRING, VdpGetErrorString) \
  UseVdpFunctionOne(VDP_FUNC_ID_GET_API_VERSION, VdpGetApiVersion) \
  UseVdpFunctionOne(VDP_FUNC_ID_GET_INFORMATION_STRING, VdpGetInformationString) \
  UseVdpFunctionOne(VDP_FUNC_ID_DEVICE_DESTROY, VdpDeviceDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_GENERATE_CSC_MATRIX, VdpGenerateCSCMatrix) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES, VdpVideoSurfaceQueryCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES, VdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities ) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_CREATE, VdpVideoSurfaceCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_DESTROY, VdpVideoSurfaceDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS, VdpVideoSurfaceGetParameters) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, VdpVideoSurfaceGetBitsYCbCr) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR, VdpVideoSurfacePutBitsYCbCr) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES, VdpOutputSurfaceQueryCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES, VdpOutputSurfaceQueryGetPutBitsNativeCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES, VdpOutputSurfaceQueryPutBitsIndexedCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES, VdpOutputSurfaceQueryPutBitsYCbCrCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, VdpOutputSurfaceCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY, VdpOutputSurfaceDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS, VdpOutputSurfaceGetParameters) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE, VdpOutputSurfaceGetBitsNative) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE, VdpOutputSurfacePutBitsNative) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED, VdpOutputSurfacePutBitsIndexed) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR, VdpOutputSurfacePutBitsYCbCr) \
  UseVdpFunctionOne(VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES, VdpBitmapSurfaceQueryCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_BITMAP_SURFACE_CREATE, VdpBitmapSurfaceCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_BITMAP_SURFACE_DESTROY, VdpBitmapSurfaceDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS, VdpBitmapSurfaceGetParameters) \
  UseVdpFunctionOne(VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE, VdpBitmapSurfacePutBitsNative) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE, VdpOutputSurfaceRenderOutputSurface) \
  UseVdpFunctionOne(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE, VdpOutputSurfaceRenderBitmapSurface) \
  UseVdpFunctionOne(VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES, VdpDecoderQueryCapabilities) \
  UseVdpFunctionOne(VDP_FUNC_ID_DECODER_CREATE, VdpDecoderCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_DECODER_DESTROY, VdpDecoderDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_DECODER_GET_PARAMETERS, VdpDecoderGetParameters) \
  UseVdpFunctionOne(VDP_FUNC_ID_DECODER_RENDER, VdpDecoderRender) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT, VdpVideoMixerQueryFeatureSupport) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT, VdpVideoMixerQueryParameterSupport) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT, VdpVideoMixerQueryAttributeSupport) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE, VdpVideoMixerQueryParameterValueRange) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE, VdpVideoMixerQueryAttributeValueRange) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_CREATE, VdpVideoMixerCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES, VdpVideoMixerSetFeatureEnables) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES, VdpVideoMixerSetAttributeValues) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT, VdpVideoMixerGetFeatureSupport) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES, VdpVideoMixerGetFeatureEnables) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES, VdpVideoMixerGetParameterValues) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES, VdpVideoMixerGetAttributeValues) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_DESTROY, VdpVideoMixerDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_VIDEO_MIXER_RENDER, VdpVideoMixerRender) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11, VdpPresentationQueueTargetCreateX11) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY, VdpPresentationQueueTargetDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, VdpPresentationQueueCreate) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, VdpPresentationQueueDestroy) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR, VdpPresentationQueueSetBackgroundColor) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR, VdpPresentationQueueGetBackgroundColor) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME, VdpPresentationQueueGetTime) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY, VdpPresentationQueueDisplay) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, VdpPresentationQueueBlockUntilSurfaceIdle) \
  UseVdpFunctionOne(VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS, VdpPresentationQueueQuerySurfaceStatus) \
  UseVdpFunctionOne(VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER, VdpPreemptionCallbackRegister)
