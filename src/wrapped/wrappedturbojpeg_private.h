#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif


//GO(tjAlloc, 
//GO(tjBufSize, 
GO(TJBUFSIZE, LFii)
//GO(tjBufSizeYUV, 
GO(TJBUFSIZEYUV, LFiii)
//GO(tjBufSizeYUV2, 
//GO(tjCompress, 
//GO(tjCompress2, 
//GO(tjCompressFromYUV, 
GO(tjCompressFromYUVPlanes, iFppipiippii)
//GO(tjDecodeYUV, 
//GO(tjDecodeYUVPlanes, 
//GO(tjDecompress, 
//GO(tjDecompress2, 
//GO(tjDecompressHeader, 
GO(tjDecompressHeader2, iFppLppp)
//GO(tjDecompressHeader3, 
GO(tjDecompressToYUV, iFppLpi)
//GO(tjDecompressToYUV2, 
//GO(tjDecompressToYUVPlanes, 
GO(tjDestroy, iFp)
//GO(tjEncodeYUV, 
//GO(tjEncodeYUV2, 
//GO(tjEncodeYUV3, 
//GO(tjEncodeYUVPlanes, 
//GO(tjFree, 
//GO(tjGetErrorCode, 
GO(tjGetErrorStr, pFv)
//GO(tjGetErrorStr2, 
//GO(tjGetScalingFactors, 
GO(tjInitCompress, pFv)
GO(tjInitDecompress, pFv)
//GO(tjInitTransform, 
//GO(tjLoadImage, 
//GO(tjPlaneHeight, 
//GO(tjPlaneSizeYUV, 
//GO(tjPlaneWidth, 
//GO(tjSaveImage, 
//GO(tjTransform, 