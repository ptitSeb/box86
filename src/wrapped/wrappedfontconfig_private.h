#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(FcAtomicCreate, pFp)
GO(FcAtomicDeleteNew, vFp)
GO(FcAtomicDestroy, vFp)
GO(FcAtomicLock, iFp)
//GO(FcAtomicNewFile, 
//GO(FcAtomicOrigFile, 
//GO(FcAtomicReplaceOrig, 
//GO(FcAtomicUnlock, 
//GO(FcBlanksAdd, 
//GO(FcBlanksCreate, 
//GO(FcBlanksDestroy, 
//GO(FcBlanksIsMember, 
//GO(FcCacheCopySet, 
//GO(FcCacheDir, 
//GO(FcCacheNumFont, 
//GO(FcCacheNumSubdir, 
//GO(FcCacheSubdir, 
//GO(FcCharSetAddChar, 
GO(FcCharSetCopy, pFp)
//GO(FcCharSetCount, 
//GO(FcCharSetCoverage, 
//GO(FcCharSetCreate, 
GO(FcCharSetDestroy, vFp)
//GO(FcCharSetEqual, 
GO(FcCharSetFirstPage, iFppp)   // second p is a fixed sized array
GO(FcCharSetHasChar, iFpi)
//GO(FcCharSetIntersect, 
//GO(FcCharSetIntersectCount, 
//GO(FcCharSetIsSubset, 
//GO(FcCharSetMerge, 
//GO(FcCharSetNew, 
GO(FcCharSetNextPage, iFppp)    // second p is a fixed sized array
GO(FcCharSetSubtract, pFpp)
//GO(FcCharSetSubtractCount, 
GO(FcCharSetUnion, pFpp)
//GO(FcConfigAppFontAddDir, 
GO(FcConfigAppFontAddFile, iFpp)
//GO(FcConfigAppFontClear, 
//GO(FcConfigBuildFonts, 
//GO(FcConfigCreate, 
//GO(FcConfigDestroy, 
//GO(FcConfigEnableHome, 
//GO(FcConfigFilename, 
GO(FcConfigGetBlanks, pFp)
//GO(FcConfigGetCache, 
//GO(FcConfigGetCacheDirs, 
//GO(FcConfigGetConfigDirs, 
//GO(FcConfigGetConfigFiles, 
//GO(FcConfigGetCurrent, 
//GO(FcConfigGetFontDirs, 
GO(FcConfigGetFonts, pFpi)
//GO(FcConfigGetRescanInterval, 
//GO(FcConfigGetRescanInverval, 
//GO(FcConfigHome, 
//GO(FcConfigParseAndLoad, 
//GO(FcConfigReference, 
//GO(FcConfigSetCurrent, 
//GO(FcConfigSetRescanInterval, 
//GO(FcConfigSetRescanInverval, 
GO(FcConfigSubstitute, iFppi)
//GO(FcConfigSubstituteWithPat, 
//GO(FcConfigUptoDate, 
GO(FcDefaultSubstitute, vFp)
//GO(FcDirCacheLoad, 
//GO(FcDirCacheLoadFile, 
//GO(FcDirCacheRead, 
//GO(FcDirCacheUnlink, 
//GO(FcDirCacheUnload, 
//GO(FcDirCacheValid, 
//GO(FcDirSave, 
//GO(FcDirScan, 
//GO(FcFileIsDir, 
//GO(FcFileScan, 
//GO(FcFini, 
GO(FcFontList, pFppp)
GO(FcFontMatch, pFppp)
//GO(FcFontRenderPrepare, 
GO(FcFontSetAdd, iFpp)
//GO(FcFontSetCreate, 
GO(FcFontSetDestroy, vFp)
//GO(FcFontSetList, 
//GO(FcFontSetMatch, 
//GO(FcFontSetPrint, 
//GO(FcFontSetSort, 
//GO(FcFontSetSortDestroy, 
GO(FcFontSort, pFppipp)
//GO(FcFreeTypeCharIndex, 
//GO(FcFreeTypeCharSet, 
//GO(FcFreeTypeCharSetAndSpacing, 
GO(FcFreeTypeQuery, pFpipp)
GO(FcFreeTypeQueryFace, pFppip)
//GO(FcGetLangs, 
//GO(FcGetVersion, 
GO(FcInit, iFv)
//GO(FcInitBringUptoDate, 
//GO(FcInitLoadConfig, 
//GO(FcInitLoadConfigAndFonts, 
//GO(FcInitReinitialize, 
//GO(FcLangGetCharSet, 
GO(FcLangSetAdd, iFpp)
//GO(FcLangSetCompare, 
//GO(FcLangSetContains, 
//GO(FcLangSetCopy, 
GO(FcLangSetCreate, pFv)
GO(FcLangSetDestroy, vFp)
//GO(FcLangSetEqual, 
//GO(FcLangSetGetLangs, 
//GO(FcLangSetHash, 
GO(FcLangSetHasLang, iFpp)
//GO(FcMatrixCopy, 
//GO(FcMatrixEqual, 
//GO(FcMatrixMultiply, 
//GO(FcMatrixRotate, 
//GO(FcMatrixScale, 
//GO(FcMatrixShear, 
//GO(FcNameConstant, 
//GO(FcNameGetConstant, 
//GO(FcNameGetObjectType, 
//GO(FcNameParse, 
//GO(FcNameRegisterConstants, 
//GO(FcNameRegisterObjectTypes, 
//GO(FcNameUnparse, 
//GO(FcNameUnregisterConstants, 
//GO(FcNameUnregisterObjectTypes, 
GO(FcObjectSetAdd, iFpp)
GO2(FcObjectSetBuild, pFpV, FcObjectSetVaBuild)
GO(FcObjectSetCreate, pFv)
GO(FcObjectSetDestroy, vFp)
GO(FcObjectSetVaBuild, pFpp)
GOM(FcPatternAdd, iFEppiuui)  // FcValue is a typedef with int+union, with biggest part is a double => so 3 "u32" on the stack
GO(FcPatternAddBool, iFppi)
GO(FcPatternAddCharSet, iFppp)
GO(FcPatternAddDouble, iFppd)
GO(FcPatternAddFTFace, iFppp)
GO(FcPatternAddInteger, iFppi)
GO(FcPatternAddLangSet, iFppp)
GO(FcPatternAddMatrix, iFppp)
GO(FcPatternAddString, iFppp)
GO(FcPatternAddWeak, iFppiuui)
GO2(FcPatternBuild, pFpV, FcPatternVaBuild)
GO(FcPatternCreate, pFv)
GO(FcPatternDel, iFpp)
GO(FcPatternDestroy, vFp)
//GO(FcPatternDuplicate, 
//GO(FcPatternEqual, 
//GO(FcPatternEqualSubset, 
//GO(FcPatternFilter, 
//GO(FcPatternFormat, 
GO(FcPatternGet, iFppip)
GO(FcPatternGetBool, iFppip)
GO(FcPatternGetCharSet, iFppip)
GO(FcPatternGetDouble, iFppip)
GO(FcPatternGetFTFace, iFppip)
GO(FcPatternGetInteger, iFppip)
GO(FcPatternGetLangSet, iFppip)
GO(FcPatternGetMatrix, iFppip)
GO(FcPatternGetString, iFppip)
//GO(FcPatternHash, 
//GO(FcPatternPrint, 
//GO(FcPatternReference, 
//GO(FcPatternRemove, 
GO(FcPatternVaBuild, pFpp)
//GO(FcStrBasename, 
GO(FcStrCmp, iFpp)
//GO(FcStrCmpIgnoreCase, 
//GO(FcStrCopy, 
//GO(FcStrCopyFilename, 
//GO(FcStrDirname, 
//GO(FcStrDowncase, 
//GO(FcStrFree, 
//GO(FcStrListCreate, 
//GO(FcStrListDone, 
//GO(FcStrListNext, 
//GO(FcStrPlus, 
//GO(FcStrSetAdd, 
//GO(FcStrSetAddFilename, 
//GO(FcStrSetCreate, 
//GO(FcStrSetDel, 
//GO(FcStrSetDestroy, 
//GO(FcStrSetEqual, 
//GO(FcStrSetMember, 
//GO(FcStrStr, 
//GO(FcStrStrIgnoreCase, 
//GO(FcUcs4ToUtf8, 
//GO(FcUtf16Len, 
//GO(FcUtf16ToUcs4, 
//GO(FcUtf8Len, 
//GO(FcUtf8ToUcs4, 
//GO(FcValueDestroy, 
//GO(FcValueEqual, 
//GO(FcValuePrint, 
//GO(FcValueSave, 
//GO(_fini, 
//GO(_init, 
