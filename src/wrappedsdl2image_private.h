#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA)

DATA(__data_start, 4)
DATA(_edata, 4)
// _fini
GO(IMG_Init,iFi)
GO(IMG_InvertAlpha,iFi)
GO(IMG_Linked_Version,pFv)
GO(IMG_Load,pFp)
GO(IMG_LoadBMP_RW,pFp)
GO(IMG_LoadCUR_RW,pFp)
GO(IMG_LoadGIF_RW,pFp)
GO(IMG_LoadICO_RW,pFp)
GO(IMG_LoadJPG_RW,pFp)
GO(IMG_LoadLBM_RW,pFp)
GO(IMG_LoadPCX_RW,pFp)
GO(IMG_LoadPNG_RW,pFp)
GO(IMG_LoadPNM_RW,pFp)
GO(IMG_LoadTGA_RW,pFp)
GO(IMG_LoadTIF_RW,pFp)
GO(IMG_LoadTyped_RW,pFpip)
GO(IMG_LoadWEBP_RW,pFp)
GO(IMG_LoadXCF_RW,pFp)
GO(IMG_LoadXPM_RW,pFp)
GO(IMG_LoadXV_RW,pFp)
GO(IMG_Load_RW,pFpi)
GO(IMG_Quit,vFv)
GO(IMG_ReadXPMFromArray,pFp)
GO(IMG_isBMP,iFp)
GO(IMG_isCUR,iFp)
GO(IMG_isGIF,iFp)
GO(IMG_isICO,iFp)
GO(IMG_isJPG,iFp)
GO(IMG_isLBM,iFp)
GO(IMG_isPCX,iFp)
GO(IMG_isPNG,iFp)
GO(IMG_isPNM,iFp)
GO(IMG_isTIF,iFp)
GO(IMG_isWEBP,iFp)
GO(IMG_isXCF,iFp)
GO(IMG_isXPM,iFp)
GO(IMG_isXV,iFp)
// IMG_InitJPG
// IMG_InitPNG
// IMG_InitTIF
// IMG_InitWEBP
// IMG_isSVG
// IMG_LoadSVG_RW
// IMG_LoadTexture
// IMG_LoadTexture_RW
// IMG_LoadTextureTyped_RW
// IMG_QuitJPG
// IMG_QuitPNG
// IMG_QuitTIF
// IMG_QuitWEBP
// IMG_SaveJPG
// IMG_SaveJPG_RW
// IMG_SavePNG
// IMG_SavePNG_RW
// _init
DATA(nsvg__colors, 4)
// nsvgCreateRasterizer
// nsvgDelete
// nsvgDeleteRasterizer
// nsvgParse
// nsvg__parseXML
// nsvgRasterize

#endif