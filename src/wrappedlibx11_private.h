#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// all those bss stuff are suspicious
//DATAB(__bss_end__, 4)
//DATAB(_bss_end__, 4)
//DATAB(__bss_start, 4)
//DATAB(__bss_start__, 4)
//DATA(__data_start, 4)
//DATA(_edata, 4)
//DATAB(_end, 4)
//DATAB(__end__, 4)
// __exidx_end  // type r
// __exidx_start    // type r
// _fini
// _init
// KeySymToUcs4
DATAB(_qfree, 4)
// _Utf8GetConvByName
//GO(XActivateScreenSaver
//GO(XAddConnectionWatch
//GO(XAddExtension
//GO(XAddHost
//GO(XAddHosts
//GO(XAddPixel
//GO(XAddToExtensionList
//GO(XAddToSaveSet
//GO(XAllocClassHint
GO(XAllocColor, iFppp)
//GO(XAllocColorCells
//GO(XAllocColorPlanes
//GO(XAllocIconSize
// _XAllocID
// _XAllocIDs
//GO(XAllocNamedColor
// _XAllocScratch
GO(XAllocSizeHints, pFv)
//GO(XAllocStandardColormap
// _XAllocTemp
GO(XAllocWMHints, pFv)
//GO(XAllowEvents
//GO(XAllPlanes
// _XAsyncErrorHandler
//GO(XAutoRepeatOff
//GO(XAutoRepeatOn
//GO(XBaseFontNameListOfFontSet
//GO(XBell
//GO(XBitmapBitOrder
//GO(XBitmapPad
//GO(XBitmapUnit
//GO(XBlackPixel
//GO(XBlackPixelOfScreen
//GO(XCellsOfScreen
//GO(XChangeActivePointerGrab
//GO(XChangeGC
//GO(XChangeKeyboardControl
//GO(XChangeKeyboardMapping
//GO(XChangePointerControl
GO(XChangeProperty, iFppppiipi)
//GO(XChangeSaveSet
//GO(XChangeWindowAttributes
//GO(XCheckIfEvent
GO(XCheckMaskEvent, iFpup)
GO(XCheckTypedEvent, iFpip)
GO(XCheckTypedWindowEvent, iFppip)
GO(XCheckWindowEvent, iFppup)
//GO(XCirculateSubwindows
//GO(XCirculateSubwindowsDown
//GO(XCirculateSubwindowsUp
GO(XClearArea, iFppiiuui)
GO(XClearWindow, iFpp)
//GO(XClipBox
GO(XCloseDisplay, iFp)
GO(XCloseIM, iFu)
// _XCloseLC
//GO(XCloseOM
// _XcmsAddCmapRec
//GO(XcmsAddColorSpace
//GO(XcmsAddFunctionSet
//GO(XcmsAllocColor
//GO(XcmsAllocNamedColor
// _XcmsArcTangent
//GO(XcmsCCCOfColormap
//GO(XcmsCIELabClipab
//GO(XcmsCIELabClipL
//GO(XcmsCIELabClipLab
DATA(XcmsCIELabColorSpace, 4)
// _XcmsCIELab_prefix   // type r
//GO(XcmsCIELabQueryMaxC
//GO(XcmsCIELabQueryMaxL
//GO(XcmsCIELabQueryMaxLC
// _XcmsCIELabQueryMaxLCRGB
//GO(XcmsCIELabQueryMinL
//GO(XcmsCIELabToCIEXYZ
//GO(XcmsCIELabWhiteShiftColors
//GO(XcmsCIELuvClipL
//GO(XcmsCIELuvClipLuv
//GO(XcmsCIELuvClipuv
DATA(XcmsCIELuvColorSpace, 4)
// _XcmsCIELuv_prefix   // type r
//GO(XcmsCIELuvQueryMaxC
//GO(XcmsCIELuvQueryMaxL
//GO(XcmsCIELuvQueryMaxLC
// _XcmsCIELuvQueryMaxLCRGB
//GO(XcmsCIELuvQueryMinL
//GO(XcmsCIELuvToCIEuvY
//GO(XcmsCIELuvWhiteShiftColors
DATA(XcmsCIEuvYColorSpace, 4)
// _XcmsCIEuvY_prefix   // type r
//GO(XcmsCIEuvYToCIELuv
//GO(XcmsCIEuvYToCIEXYZ
//GO(XcmsCIEuvYToTekHVC
// _XcmsCIEuvY_ValidSpec
DATA(XcmsCIExyYColorSpace, 4)
// _XcmsCIExyY_prefix   // type r
//GO(XcmsCIExyYToCIEXYZ
DATA(XcmsCIEXYZColorSpace, 4)
// _XcmsCIEXYZ_prefix   // type r
//GO(XcmsCIEXYZToCIELab
//GO(XcmsCIEXYZToCIEuvY
//GO(XcmsCIEXYZToCIExyY
//GO(XcmsCIEXYZToRGBi
// _XcmsCIEXYZ_ValidSpec
//GO(XcmsClientWhitePointOfCCC
//GO(XcmsConvertColors
// _XcmsConvertColorsWithWhitePt
// _XcmsCopyCmapRecAndFree
// _XcmsCopyISOLatin1Lowered
// _XcmsCopyPointerArray
// _XcmsCosine
//GO(XcmsCreateCCC
// _XcmsCubeRoot
DATA(_XcmsDDColorSpaces, 4)
DATA(_XcmsDDColorSpacesInit, 4)
// _XcmsDDConvertColors
//GO(XcmsDefaultCCC
// _XcmsDeleteCmapRec
DATA(_XcmsDIColorSpaces, 4)
DATA(_XcmsDIColorSpacesInit, 4)
// _XcmsDIConvertColors
//GO(XcmsDisplayOfCCC
// _XcmsEqualWhitePts
//GO(XcmsFormatOfPrefix
//GO(XcmsFreeCCC
// _XcmsFreeIntensityMaps
// _XcmsFreePointerArray
// _XcmsGetElement
// _XcmsGetIntensityMap
// _XcmsGetProperty
// _XcmsInitDefaultCCCs
// _XcmsInitScrnInfo
DATA(XcmsLinearRGBFunctionSet, 4)
//GO(XcmsLookupColor
// _XcmsLRGB_InitScrnDefault
//GO(XcmsPrefixOfFormat
// _XcmsPushPointerArray
//GO(XcmsQueryBlack
//GO(XcmsQueryBlue
//GO(XcmsQueryColor
//GO(XcmsQueryColors
//GO(XcmsQueryGreen
//GO(XcmsQueryRed
//GO(XcmsQueryWhite
DATA(_XcmsRegColorSpaces, 4)
// _XcmsRegFormatOfPrefix
// _XcmsResolveColor
// _XcmsResolveColorString
DATA(XcmsRGBColorSpace, 4)
DATA(XcmsRGBiColorSpace, 4)
// _XcmsRGBi_prefix // type r
//GO(XcmsRGBiToCIEXYZ
//GO(XcmsRGBiToRGB
// _XcmsRGB_prefix  // type r
//GO(XcmsRGBToRGBi
// _XcmsRGB_to_XColor
DATA(_XcmsSCCFuncSets, 4)
DATA(_XcmsSCCFuncSetsInit, 4)
//GO(XcmsScreenNumberOfCCC
//GO(XcmsScreenWhitePointOfCCC
//GO(XcmsSetCCCOfColormap
//GO(XcmsSetCompressionProc
// _XcmsSetGetColor
// _XcmsSetGetColors
//GO(XcmsSetWhiteAdjustProc
//GO(XcmsSetWhitePoint
// _XcmsSine
// _XcmsSquareRoot
//GO(XcmsStoreColor
//GO(XcmsStoreColors
// _XcmsTekHVC_CheckModify
//GO(XcmsTekHVCClipC
//GO(XcmsTekHVCClipV
//GO(XcmsTekHVCClipVC
DATA(XcmsTekHVCColorSpace, 4)
// _XcmsTekHVC_prefix   // type r
//GO(XcmsTekHVCQueryMaxC
//GO(XcmsTekHVCQueryMaxV
//GO(XcmsTekHVCQueryMaxVC
// _XcmsTekHVCQueryMaxVCRGB
//GO(XcmsTekHVCQueryMaxVSamples
//GO(XcmsTekHVCQueryMinV
//GO(XcmsTekHVCToCIEuvY
//GO(XcmsTekHVCWhiteShiftColors
DATA(XcmsUNDEFINEDColorSpace, 4)
// _XcmsUnresolveColor
//GO(XcmsVisualOfCCC
// _XColor_to_XcmsRGB
GO(XConfigureWindow, iFppup)
//GO(XConnectionNumber
// _XConnectXCB
//GO(XContextDependentDrawing
//GO(XContextualDrawing
//GO(XConvertCase
//GO(XConvertSelection
//GO(XCopyArea
GO(XCopyColormapAndFree, uFpu)
// _XCopyEventCookie
//GO(XCopyGC
//GO(XCopyPlane
// _XCopyToArg
GO(XCreateBitmapFromData, pFpppuu)
GO(XCreateColormap, uFpppi)
GO(XCreateFontCursor, pFpu)
//GO(XCreateFontSet
GO(XCreateGC, pFppup)
//GO(XCreateGlyphCursor
GO(XCreateIC, uFpppppp)     // use vararg, so putting an arbitrary number of args
GOM(XCreateImage, pFEppuiipuuii)
GO(dummy_XCreateImage, pFppuiipuuii)    // to have the wrapper
DATAB(_XCreateMutex_fn, 4)
//GO(XCreateOC
GO(XCreatePixmap, pFppuuu)
GO(XCreatePixmapCursor, pFpppppuu)
GO(XCreatePixmapFromBitmapData, pFpppuuuuu)
//GO(XCreateRegion
GO(XCreateSimpleWindow, pFppiiuuuuu)
GO(XCreateWindow, pFppiiuuuiupup)
DATAB(_Xdebug, 4)
GO(XDefaultColormap, pFpi)
//GO(XDefaultColormapOfScreen
GO(XDefaultDepth, iFpi)
//GO(XDefaultDepthOfScreen
// _XDefaultError
//GO(XDefaultGC
//GO(XDefaultGCOfScreen
// _XDefaultIOError
// _XDefaultOpenIM
// _XDefaultOpenOM
GO(XDefaultRootWindow, pFp)
GO(XDefaultScreen, pFp)
//GO(XDefaultScreenOfDisplay
//GO(XDefaultString
GO(XDefaultVisual, pFpi)
//GO(XDefaultVisualOfScreen
// _XDefaultWireError
GO(XDefineCursor, iFppp)
//GO(XDeleteContext
//GO(XDeleteModifiermapEntry
GO(XDeleteProperty, iFppp)
// _XDeq
// _XDeqAsyncHandler
GO(XDestroyIC, vFu)
//GOM(XDestroyImage, iFEp)  //need to unbridge
//GO(XDestroyOC
//GO(XDestroyRegion
GO(XDestroySubwindows, iFpp)
GO(XDestroyWindow, iFpp)
//GO(XDirectionalDependentDrawing
//GO(XDisableAccessControl
//GO(XDisplayCells
GO(XDisplayHeight, iFpi)
GO(XDisplayHeightMM, iFpi)
//GO(XDisplayKeycodes
//GO(XDisplayMotionBufferSize
//GO(XDisplayName
//GO(XDisplayOfIM
//GO(XDisplayOfOM
//GO(XDisplayOfScreen
//GO(XDisplayPlanes
//GO(XDisplayString
GO(XDisplayWidth, iFpi)
GO(XDisplayWidthMM, iFpi)
//GO(XDoesBackingStore
//GO(XDoesSaveUnders
//GO(XDrawArc
//GO(XDrawArcs
//GO(XDrawImageString
//GO(XDrawImageString16
GO(XDrawLine, iFpppiiii)
GO(XDrawLines, iFppppii)
//GO(XDrawPoint
//GO(XDrawPoints
GO(XDrawRectangle, iFpppiiuu)
GO(XDrawRectangles, iFppppi)
GO(XDrawSegments, iFppppi)
GO(XDrawString, iFpppiipi)
GO(XDrawString16, iFpppiipi)
//GO(XDrawText
//GO(XDrawText16
// _XEatData
GO(_XEatDataWords, vFpu)
//GO(XEHeadOfExtensionList
//GO(XEmptyRegion
//GO(XEnableAccessControl
// _XEnq
//GO(XEqualRegion
// _XError
DATAB(_XErrorFunction, 4)
//GO(XESetBeforeFlush
//GO(XESetCloseDisplay
//GO(XESetCopyEventCookie
//GO(XESetCopyGC
//GO(XESetCreateFont
//GO(XESetCreateGC
//GO(XESetError
//GO(XESetErrorString
//GO(XESetEventToWire
//GO(XESetFlushGC
//GO(XESetFreeFont
//GO(XESetFreeGC
//GO(XESetPrintErrorValues
//GO(XESetWireToError
//GO(XESetWireToEvent
//GO(XESetWireToEventCookie
//GO(XEventMaskOfScreen
GO(XEventsQueued, iFpi)
// _XEventsQueued
// _Xevent_to_mask  // type r
// _XEventToWire
//GO(XExtendedMaxRequestSize
//GO(XExtentsOfFontSet
// _XF86BigfontFreeFontMetrics
// _XF86LoadQueryLocaleFont
//GO(XFetchBuffer
//GO(XFetchBytes
// _XFetchEventCookie
//GO(XFetchName
//GO(XFillArc
//GO(XFillArcs
//GO(XFillPolygon
GO(XFillRectangle, iFpppiiuu)
GO(XFillRectangles, iFppppi)
GO(XFilterEvent, iFpp)
//GO(XFindContext
//GO(XFindOnExtensionList
GO(XFlush, iFp)
// _XFlush
GO(XFlushGC, vFpp)
// _XFlushGCCache
//GO(XFontsOfFontSet
GO(XForceScreenSaver, iFpi)
GO(XFree, iFp)
// _XFreeAtomTable
GO(XFreeColormap, iFpu)
GO(XFreeColors, iFpppiu)
GO(XFreeCursor, iFpp)
DATAB(_XFreeDisplayLock_fn, 4)
// _XFreeDisplayStructure
// _XFreeEventCookies
GO(XFreeEventData, vFpp)
// _XFreeExtData
GO(XFreeExtensionList, iFp)
GO(XFreeFont, iFpp)
GO(XFreeFontInfo, iFppi)
GO(XFreeFontNames, iFp)
GO(XFreeFontPath, iFp)
//GO(XFreeFontSet
GO(XFreeGC, iFpp)
GO(XFreeModifiermap, iFp)
DATAB(_XFreeMutex_fn, 4)
GO(XFreePixmap, iFpp)
//GO(XFreeStringList
// _XFreeTemp
// _XFreeX11XCBStructure
GO(XGContextFromGC, pFp)
GO(XGeometry, iFpippuuuiipppp)
// _XGetAsyncData
// _XGetAsyncReply
GO(XGetAtomName, pFpp)
//GO(XGetAtomNames
// _XGetBitsPerPixel
//GO(XGetClassHint
GO(XGetCommand, iFpppp)
//GO(XGetDefault
GO(XGetErrorDatabaseText, iFpppppi)
GO(XGetErrorText, iFpipi)
GO(XGetEventData, iFpp)
//GO(XGetFontPath
GO(XGetFontProperty, iFppp)
GO(XGetGCValues, iFppup)
GO(XGetGeometry, iFppppppppp)
// _XGetHostname
GO(XGetIconName, iFppp)
//GO(XGetIconSizes
GO(XGetICValues, pFpppppp)      // use varargs...
GO(XGetImage, pFppiiuuui)
//GO(XGetIMValues
GO(XGetInputFocus, iFppp)
GO(XGetKeyboardControl, iFpp)
//GO(XGetKeyboardMapping
// _XGetLCValues
//GO(XGetModifierMapping
//GO(XGetMotionEvents
//GO(XGetNormalHints
//GO(XGetOCValues
//GO(XGetOMValues
//GOM(XGetPixel, uFEpii)  // need unbridging
GO(dummy_XGetPixel, uFpii)     // for the wrapper
//GO(XGetPointerControl
GO(XGetPointerMapping, iFppi)
GO(_XGetRequest, pFpuu)
//GO(XGetRGBColormaps
// _XGetScanlinePad
GO(XGetScreenSaver, iFppppp)
//GO(XGetSelectionOwner
//GO(XGetSizeHints
//GO(XGetStandardColormap
GO(XGetSubImage, pFppiiuuuipii)
//GO(XGetTextProperty
GO(XGetTransientForHint, iFppp)
GO(XGetVisualInfo, pFpipp)
GO(XGetWindowAttributes, iFppp)
// _XGetWindowAttributes
GO(XGetWindowProperty, iFpppiiipppppp)
//GO(XGetWMClientMachine
GO(XGetWMColormapWindows, iFpppp)
//GO(XGetWMHints
//GO(XGetWMIconName
//GO(XGetWMName
GO(XGetWMNormalHints, iFpppp)
//GO(XGetWMProtocols
GO(XGetWMSizeHints, iFppppu)
//GO(XGetZoomHints
DATAB(_Xglobal_lock, 4)
GO(XGrabButton, iFpuupiuiipp)
GO(XGrabKey, iFpiupiii)
GO(XGrabKeyboard, iFppuiiu)
GO(XGrabPointer, iFppiuiippu)
GO(XGrabServer, iFp)
DATAB(_XHeadOfDisplayList, 4)
GO(XHeightMMOfScreen, iFp)
GO(XHeightOfScreen, iFp)
DATAB(_Xi18n_lock, 4)
GO(XIconifyWindow, iFppi)
GOM(XIfEvent, iFEpppp)
GO(XImageByteOrder, iFp)
// _XIMCompileResourceList
// _XimGetCharCode
// _XimGetLocaleCode
// _XimLookupMBText
// _XimLookupUTF8Text
// _XimLookupWCText
GO(XIMOfIC, pFu)
// _XimXTransBytesReadable
// _XimXTransClose
// _XimXTransCloseForCloning
// _XimXTransConnect
// _XimXTransDisconnect
// _XimXTransFreeConnInfo
// _XimXTransGetConnectionNumber
// _XimXTransGetHostname
// _XimXTransGetMyAddr
// _XimXTransGetPeerAddr
// _XimXTransIsLocal
// _XimXTransOpenCLTSClient
// _XimXTransOpenCOTSClient
// _XimXTransRead
// _XimXTransReadv
// _XimXTransSetOption
DATA(_XimXTransSocketINET6Funcs, 4)
DATA(_XimXTransSocketINETFuncs, 4)
DATA(_XimXTransSocketLocalFuncs, 4)
DATA(_XimXTransSocketTCPFuncs, 4)
DATA(_XimXTransSocketUNIXFuncs, 4)
// _XimXTransWrite
// _XimXTransWritev
// _XInitDefaultIM
// _XInitDefaultOM
DATAB(_XInitDisplayLock_fn, 4)
// _XInitDynamicIM
// _XInitDynamicOM
//GO(XInitExtension
//GO(XInitImage
// _XInitImageFuncPtrs
// _XInitKeysymDB
GO(XInitThreads, uFv)
//GO(XInsertModifiermapEntry
//GO(XInstallColormap
//GO(XInternalConnectionNumbers
GO(XInternAtom, pFppi)
GO(XInternAtoms, uFppiip)
//GO(XIntersectRegion
// _XIOError
DATAB(_XIOErrorFunction, 4)
// _XIsEventCookie
//GO(XkbAddDeviceLedInfo
//GO(XkbAddGeomColor
//GO(XkbAddGeomDoodad
//GO(XkbAddGeomKey
//GO(XkbAddGeomKeyAlias
//GO(XkbAddGeomOutline
//GO(XkbAddGeomOverlay
//GO(XkbAddGeomOverlayKey
//GO(XkbAddGeomOverlayRow
//GO(XkbAddGeomProperty
//GO(XkbAddGeomRow
//GO(XkbAddGeomSection
//GO(XkbAddGeomShape
//GO(XkbAddKeyType
//GO(XkbAllocClientMap
//GO(XkbAllocCompatMap
//GO(XkbAllocControls
//GO(XkbAllocDeviceInfo
//GO(XkbAllocGeomColors
//GO(XkbAllocGeomDoodads
//GO(XkbAllocGeometry
//GO(XkbAllocGeomKeyAliases
//GO(XkbAllocGeomKeys
//GO(XkbAllocGeomOutlines
//GO(XkbAllocGeomOverlayKeys
//GO(XkbAllocGeomOverlayRows
//GO(XkbAllocGeomOverlays
//GO(XkbAllocGeomPoints
//GO(XkbAllocGeomProps
//GO(XkbAllocGeomRows
//GO(XkbAllocGeomSectionDoodads
//GO(XkbAllocGeomSections
//GO(XkbAllocGeomShapes
//GO(XkbAllocIndicatorMaps
//GO(XkbAllocKeyboard
//GO(XkbAllocNames
//GO(XkbAllocServerMap
//GO(XkbApplyCompatMapToKey
//GO(XkbApplyVirtualModChanges
//GO(XkbBell
//GO(XkbBellEvent
//GO(XkbChangeDeviceInfo
//GO(XkbChangeEnabledControls
//GO(XkbChangeKeycodeRange
//GO(XkbChangeMap
//GO(XkbChangeNames
//GO(XkbChangeTypesOfKey
//GO(XkbComputeEffectiveMap
//GO(XkbComputeRowBounds
//GO(XkbComputeSectionBounds
//GO(XkbComputeShapeBounds
//GO(XkbComputeShapeTop
// _XkbCopyFromReadBuffer
//GO(XkbCopyKeyType
//GO(XkbCopyKeyTypes
//GO(XkbDeviceBell
//GO(XkbDeviceBellEvent
//GO(XkbFindOverlayForKey
//GO(XkbForceBell
//GO(XkbForceDeviceBell
//GO(XkbFreeClientMap
//GO(XkbFreeCompatMap
//GO(XkbFreeComponentList
//GO(XkbFreeControls
//GO(XkbFreeDeviceInfo
//GO(XkbFreeGeomColors
//GO(XkbFreeGeomDoodads
//GO(XkbFreeGeometry
//GO(XkbFreeGeomKeyAliases
//GO(XkbFreeGeomKeys
//GO(XkbFreeGeomOutlines
//GO(XkbFreeGeomOverlayKeys
//GO(XkbFreeGeomOverlayRows
//GO(XkbFreeGeomOverlays
//GO(XkbFreeGeomPoints
//GO(XkbFreeGeomProperties
//GO(XkbFreeGeomRows
//GO(XkbFreeGeomSections
//GO(XkbFreeGeomShapes
//GO(XkbFreeIndicatorMaps
//GO(XkbFreeKeyboard
//GO(XkbFreeNames
// _XkbFreeReadBuffer
//GO(XkbFreeServerMap
DATA(_XkbGetAtomNameFunc, 4)
//GO(XkbGetAutoRepeatRate
//GO(XkbGetAutoResetControls
// _XkbGetCharset
//GO(XkbGetCompatMap
//GO(XkbGetControls
// _XkbGetConverters
//GO(XkbGetDetectableAutoRepeat
//GO(XkbGetDeviceButtonActions
//GO(XkbGetDeviceInfo
//GO(XkbGetDeviceInfoChanges
//GO(XkbGetDeviceLedInfo
//GO(XkbGetGeometry
//GO(XkbGetIndicatorMap
//GO(XkbGetIndicatorState
//GO(XkbGetKeyActions
//GO(XkbGetKeyBehaviors
//GO(XkbGetKeyboard
//GO(XkbGetKeyboardByName
//GO(XkbGetKeyExplicitComponents
//GO(XkbGetKeyModifierMap
//GO(XkbGetKeySyms
//GO(XkbGetKeyTypes
//GO(XkbGetKeyVirtualModMap
//GO(XkbGetMap
//GO(XkbGetMapChanges
//GO(XkbGetNamedDeviceIndicator
//GO(XkbGetNamedGeometry
//GO(XkbGetNamedIndicator
//GO(XkbGetNames
//GO(XkbGetPerClientControls
// _XkbGetReadBufferCountedString
// _XkbGetReadBufferPtr
//GO(XkbGetState
//GO(XkbGetUpdatedMap
//GO(XkbGetVirtualMods
//GO(XkbGetXlibControls
//GO(XkbIgnoreExtension
//GO(XkbInitCanonicalKeyTypes
// _XkbInitReadBuffer
DATA(_XkbInternAtomFunc, 4)
GO(XkbKeycodeToKeysym, uFpuuu)
//GO(XkbKeysymToModifiers
//GO(XkbKeyTypesForCoreSymbols
//GO(XkbLatchGroup
//GO(XkbLatchModifiers
//GO(XkbLibraryVersion
//GO(XkbListComponents
//GO(XkbLockGroup
//GO(XkbLockModifiers
//GO(XkbLookupKeyBinding
//GO(XkbLookupKeySym
//GO(XkbNoteControlsChanges
// _XkbNoteCoreMapChanges
//GO(XkbNoteDeviceChanges
//GO(XkbNoteMapChanges
//GO(XkbNoteNameChanges
//GO(XkbOpenDisplay
// _XkbPeekAtReadBuffer
//GO(XkbQueryExtension
// _XkbReadBufferCopyKeySyms
// _XkbReadCopyKeySyms
// _XkbReadGetCompatMapReply
// _XkbReadGetGeometryReply
// _XkbReadGetIndicatorMapReply
// _XkbReadGetMapReply
// _XkbReadGetNamesReply
//GO(XkbRefreshKeyboardMapping
// _XkbReloadDpy
//GO(XkbResizeDeviceButtonActions
//GO(XkbResizeKeyActions
//GO(XkbResizeKeySyms
//GO(XkbResizeKeyType
//GO(XkbSelectEventDetails
//GO(XkbSelectEvents
//GO(XkbSetAtomFuncs
//GO(XkbSetAutoRepeatRate
//GO(XkbSetAutoResetControls
//GO(XkbSetCompatMap
//GO(XkbSetControls
//GO(XkbSetDebuggingFlags
//GO(XkbSetDetectableAutoRepeat
//GO(XkbSetDeviceButtonActions
//GO(XkbSetDeviceInfo
//GO(XkbSetDeviceLedInfo
//GO(XkbSetGeometry
//GO(XkbSetIgnoreLockMods
//GO(XkbSetIndicatorMap
//GO(XkbSetMap
//GO(XkbSetNamedDeviceIndicator
//GO(XkbSetNamedIndicator
//GO(XkbSetNames
//GO(XkbSetPerClientControls
//GO(XkbSetServerInternalMods
//GO(XkbSetXlibControls
// _XkbSkipReadBufferData
//GO(XkbToControl
//GO(XkbTranslateKey
//GO(XkbTranslateKeyCode
//GO(XkbTranslateKeySym
//GO(XkbUpdateActionVirtualMods
//GO(XkbUpdateKeyTypeVirtualMods
//GO(XkbUpdateMapFromCore
//GO(XkbUseExtension
//GO(XkbVirtualModsToReal
// _XkbWriteCopyKeySyms
//GO(XkbXlibControlsImplemented
GO(XKeycodeToKeysym, pFppi)
// _XKeycodeToKeysym
// _XKeyInitialize
GO(XKeysymToKeycode, uFpu)
// _XKeysymToKeycode
// _XKeysymToModifiers
GO(XKeysymToString, pFp)
// _XkeyTable   // type r
//GO(XKillClient
//GO(XLastKnownRequestProcessed
// _XlcAddCharSet
// _XlcAddCT
// _XlcAddGB18030LocaleConverters
// _XlcAddLoader
// _XlcAddUtf8Converters
// _XlcAddUtf8LocaleConverters
// _XlcCloseConverter
// _XlcCompareISOLatin1
// _XlcCompileResourceList
// _XlcConvert
// _XlcCopyFromArg
// _XlcCopyToArg
// _XlcCountVaList
// _XlcCreateDefaultCharSet
// _XlcCreateLC
// _XlcCreateLocaleDataBase
// _XlcCurrentLC
// _XlcDbg_printValue
// _XlcDefaultMapModifiers
// _XlcDeInitLoader
// _XlcDestroyLC
// _XlcDestroyLocaleDataBase
// _XlcDynamicLoad
// _XlcFileName
DATA(_XlcGenericMethods, 4)
// _XlcGetCharSet
// _XlcGetCharSetWithSide
// _XlcGetCSValues
// _XlcGetLocaleDataBase
// _XlcGetResource
// _XlcGetValues
// _XlcInitCTInfo
// _XlcInitLoader
// _XlcLocaleDirName
// _XlcLocaleLibDirName
// _XlcMapOSLocaleName
// _Xlcmbstoutf8
// _Xlcmbstowcs
// _Xlcmbtowc
// _XlcNCompareISOLatin1
// _XlcOpenConverter
// _XlcParseCharSet
// _XlcParse_scopemaps
DATA(_XlcPublicMethods, 4)
// _XlcRemoveLoader
// _XlcResetConverter
// _XlcResolveI18NPath
// _XlcResolveLocaleName
// _XlcSetConverter
// _XlcSetValues
// _XlcValidModSyntax
// _XlcVaToArgList
// _Xlcwcstombs
// _Xlcwctomb
//GO(XListDepths
GO(XListExtensions, pFpp)
//GO(XListFonts
//GO(XListFontsWithInfo
//GO(XListHosts
//GO(XListInstalledColormaps
//GO(XListPixmapFormats
GO(XListProperties, pFppp)
GO(XLoadFont, pFpp)
GO(XLoadQueryFont, pFpp)
// xlocaledir
//GO(XLocaleOfFontSet
//GO(XLocaleOfIM
//GO(XLocaleOfOM
GO(XLockDisplay, vFp)
DATAB(_XLockMutex_fn, 4)
GO(XLookupColor, iFppppp)
GO(XLookupKeysym, uFpi)
// _XLookupKeysym
GO(XLookupString, iFppipp)
// _XLookupString
GO(XLowerWindow, iFpp)
GO(XMapRaised, iFpp)
GO(XMapSubwindows, iFpp)
GO(XMapWindow, iFpp)
GO(XMaskEvent, iFpup)
//GO(XMatchVisualInfo
//GO(XMaxCmapsOfScreen
//GO(XMaxRequestSize
//GO(XmbDrawImageString
//GO(XmbDrawString
//GO(XmbDrawText
// _Xmblen
GO(XmbLookupString, iFuppipp)
//GO(XmbResetIC
GO(XmbSetWMProperties, vFpppppippp)
// _Xmbstoutf8
// _Xmbstowcs
//GO(XmbTextEscapement
//GO(XmbTextExtents
//GO(XmbTextListToTextProperty
// _XmbTextListToTextProperty
//GO(XmbTextPerCharExtents
//GO(XmbTextPropertyToTextList
// _XmbTextPropertyToTextList
// _Xmbtowc
//GO(XMinCmapsOfScreen
GO(XMoveResizeWindow, iFppiiuu)
GO(XMoveWindow, iFppii)
//GO(XNewModifiermap
GO(XNextEvent, iFpp)
//GO(XNextRequest
//GO(XNoOp
// _XNoticeCreateBitmap
// _XNoticePutBitmap
//GO(XOffsetRegion
//GO(XOMOfOC
GO(XOpenDisplay, pFp)
GO(XOpenIM, uFpppp)
// _XOpenLC
//GO(XOpenOM
// _XParseBaseFontNameList
GO(XParseColor, iFpppp)
//GO(XParseGeometry
GO(XPeekEvent, iFpp)
//GO(XPeekIfEvent
GO(XPending, iFp)
//GO(Xpermalloc
//GO(XPlanesOfScreen
GO(XPointInRegion, iFpp)
// _XPollfdCacheAdd
// _XPollfdCacheDel
// _XPollfdCacheInit
//GO(XPolygonRegion
//GO(XProcessInternalConnection
// _XProcessInternalConnection
// _XProcessWindowAttributes
//GO(XProtocolRevision
//GO(XProtocolVersion
//GO(XPutBackEvent
// _XPutBackEvent
GO(XPutImage, iFppppiiiiuu)
//GO(XPutPixel
//GO(XQLength
GO(XQueryBestCursor, iFppuupp)
//GO(XQueryBestSize
//GO(XQueryBestStipple
//GO(XQueryBestTile
GO(XQueryColor, iFppp)
GO(XQueryColors, iFpppi)
GO(XQueryExtension, iFppppp)
GO(XQueryFont, pFpp)
//GO(XQueryKeymap
GO(XQueryPointer, iFppppppppp)
GO(XQueryTextExtents, iFpppipppp)
GO(XQueryTextExtents16, iFpppipppp)
GO(XQueryTree, pFpppppp)
GO(XRaiseWindow, iFpp)
GO(_XRead, iFppi)
//GO(XReadBitmapFile
//GO(XReadBitmapFileData
// _XReadEvents
// _XReadPad
GO(XRebindKeysym, iFpupipi)
GO(XRecolorCursor, iFpppp)
//GO(XReconfigureWMWindow
GO(XRectInRegion, iFpiiuu)
GO(XRefreshKeyboardMapping, iFp)
// _XRefreshKeyboardMapping
// _XRegisterFilterByMask
// _XRegisterFilterByType
//GO(XRegisterIMInstantiateCallback
// _XRegisterInternalConnection
//GO(XRemoveConnectionWatch
//GO(XRemoveFromSaveSet
//GO(XRemoveHost
//GO(XRemoveHosts
//GO(XReparentWindow
GO(_XReply, iFppii)
GO(XResetScreenSaver, iFp)
GO(XResizeWindow, iFppuu)
//GO(XResourceManagerString
GO(XRestackWindows, iFppi)
// _XReverse_Bytes
//GO(XrmCombineDatabase
//GO(XrmCombineFileDatabase
// _XrmDefaultInitParseInfo
//GO(XrmDestroyDatabase
//GO(XrmEnumerateDatabase
//GO(XrmGetDatabase
//GO(XrmGetFileDatabase
//GO(XrmGetResource
//GO(XrmGetStringDatabase
//GO(XrmInitialize
// _XrmInitParseInfo
// _XrmInternalStringToQuark
//GO(XrmLocaleOfDatabase
//GO(XrmMergeDatabases
//GO(XrmParseCommand
//GO(XrmPermStringToQuark
//GO(XrmPutFileDatabase
//GO(XrmPutLineResource
//GO(XrmPutResource
//GO(XrmPutStringResource
//GO(XrmQGetResource
//GO(XrmQGetSearchList
//GO(XrmQGetSearchResource
//GO(XrmQPutResource
//GO(XrmQPutStringResource
//GO(XrmQuarkToString
//GO(XrmSetDatabase
//GO(XrmStringToBindingQuarkList
//GO(XrmStringToQuark
//GO(XrmStringToQuarkList
//GO(XrmUniqueQuark
//GO(XRootWindow
//GO(XRootWindowOfScreen
GO(XRotateBuffers, iFpi)
GO(XRotateWindowProperties, iFpppii)
//GO(XSaveContext
//GO(XScreenCount
//GO(XScreenNumberOfScreen
//GO(XScreenOfDisplay
// _XScreenOfWindow
//GO(XScreenResourceString
GO(XSelectInput, iFppi)
// _XSend
GO(XSendEvent, uFppiip)
//GO(XServerVendor
GO(XSetAccessControl, iFpi)
//GO(XSetAfterFunction
GO(XSetArcMode, iFppi)
//GO(XSetAuthorization
GO(XSetBackground, iFppu)
//GO(XSetClassHint
GO(XSetClipMask, iFppp)
GO(XSetClipOrigin, iFppii)
GO(XSetClipRectangles, iFppiipii)
// _XSetClipRectangles
GO(XSetCloseDownMode, iFpi)
GO(XSetCommand, iFpppi)
GO(XSetDashes, iFppipi)
GOM(XSetErrorHandler, pFEp)
GO(XSetFillRule, iFppi)
GO(XSetFillStyle, iFppp)
GO(XSetFont, iFppp)
GO(XSetFontPath, iFppi)
GO(XSetForeground, iFppu)
GO(XSetFunction, iFppi)
GO(XSetGraphicsExposures, iFppu)
GO(XSetICFocus, vFu)
//GO(XSetIconName
//GO(XSetIconSizes
GO(XSetICValues, pFpppppp)          // use vargarg
// _XSetImage
//GO(XSetIMValues
GO(XSetInputFocus, iFppii)
//GO(XSetIOErrorHandler
// _XSetLastRequestRead
//GO(XSetLineAttributes
GO(XSetLocaleModifiers, pFp)
//GO(XSetModifierMapping
//GO(XSetNormalHints
//GO(XSetOCValues
//GO(XSetOMValues
//GO(XSetPlaneMask
//GO(XSetPointerMapping
//GO(XSetRegion
//GO(XSetRGBColormaps
//GO(XSetScreenSaver
//GO(XSetSelectionOwner
//GO(XSetSizeHints
//GO(XSetStandardColormap
GO(XSetStandardProperties, iFpppppppp)
//GO(XSetState
//GO(XSetStipple
GO(XSetSubwindowMode, iFppi)
//GO(XSetTextProperty
//GO(XSetTile
GO(XSetTransientForHint, iFppp)
//GO(XSetTSOrigin
GO(XSetWindowBackground, iFppu)
GO(XSetWindowBackgroundPixmap, iFppp)
//GO(XSetWindowBorder
//GO(XSetWindowBorderPixmap
GO(XSetWindowBorderWidth, iFppu)
//GO(XSetWindowColormap
//GO(XSetWMClientMachine
GO(XSetWMColormapWindows, iFpppi)
GO(XSetWMHints, iFppp)
//GO(XSetWMIconName
GO(XSetWMName, vFppp)
GO(XSetWMNormalHints, iFpppp)
GO(XSetWMProperties, vFpppppippp)
GO(XSetWMProtocols, iFpppi)
GO(XSetWMSizeHints, vFpppu)
//GO(XSetZoomHints
//GO(XShrinkRegion
//GO(XStoreBuffer
//GO(XStoreBytes
//GO(XStoreColor
//GO(XStoreColors
// _XStoreEventCookie
GO(XStoreName, iFppp)
//GO(XStoreNamedColor
GO(XStringListToTextProperty, iFpip)
//GO(XStringToKeysym
//GOM(XSubImage, pFEpiiuu)    // need unbridging
GO(dummy_XSubImage, pFpiiuu)    // for the wrapper
//GO(XSubtractRegion
GO(XSupportsLocale, iFv)
GO(XSync, iFpu)
GO(XSynchronize, iFpi)
GO(XTextExtents, iFppipppp)
GO(XTextExtents16, iFppipppp)
// _XTextHeight
// _XTextHeight16
//GO(XTextPropertyToStringList
GO(XTextWidth, iFppi)
GO(XTextWidth16, iFppi)
DATAB(_Xthread_self_fn, 4)
GO(XTranslateCoordinates, iFpppiippp)
// _XTranslateKey
// _XTranslateKeySym
// _XTryShapeBitmapCursor
GO(XUndefineCursor, iFpp)
//GO(XUngrabButton
//GO(XUngrabKey
GO(XUngrabKeyboard, iFpu)
GO(XUngrabPointer, iFpu)
GO(XUngrabServer, iFp)
GO(XUninstallColormap, iFpp)
//GO(XUnionRectWithRegion
GO(XUnionRegion, iFppp)
// _XUnknownCopyEventCookie
// _XUnknownNativeEvent
// _XUnknownWireEvent
// _XUnknownWireEventCookie
GO(XUnloadFont, iFpp)
GO(XUnlockDisplay, vFp)
DATAB(_XUnlockMutex_fn, 4)
GO(XUnmapSubwindows, iFpp)
GO(XUnmapWindow, iFpp)
// _XUnregisterFilter
//GO(XUnregisterIMInstantiateCallback
// _XUnregisterInternalConnection
// _XUnresolveColor
GO(XUnsetICFocus, vFu)
// _XUpdateAtomCache
// _XUpdateGCCache
//GO(Xutf8DrawImageString
//GO(Xutf8DrawString
//GO(Xutf8DrawText
GO(Xutf8LookupString, iFuppipp)
//GO(Xutf8ResetIC
GO(Xutf8SetWMProperties, vFpppppippp)
//GO(Xutf8TextEscapement
//GO(Xutf8TextExtents
//GO(Xutf8TextListToTextProperty
// _Xutf8TextListToTextProperty
//GO(Xutf8TextPerCharExtents
//GO(Xutf8TextPropertyToTextList
// _Xutf8TextPropertyToTextList
//GO(XVaCreateNestedList
//GO(XVendorRelease
// _XVIDtoVisual
//GO(XVisualIDFromVisual
GO(XWarpPointer, iFpppiiuuii)
//GO(XwcDrawImageString
//GO(XwcDrawString
//GO(XwcDrawText
//GO(XwcFreeStringList
// _XwcFreeStringList
GO(XwcLookupString, iFuppipp)
//GO(XwcResetIC
// _Xwcscmp
// _Xwcscpy
// _Xwcslen
// _Xwcsncmp
// _Xwcsncpy
// _Xwcstombs
//GO(XwcTextEscapement
//GO(XwcTextExtents
//GO(XwcTextListToTextProperty
// _XwcTextListToTextProperty
//GO(XwcTextPerCharExtents
//GO(XwcTextPropertyToTextList
// _XwcTextPropertyToTextList
// _Xwctomb
//GO(XWhitePixel
//GO(XWhitePixelOfScreen
//GO(XWidthMMOfScreen
//GO(XWidthOfScreen
GO(XWindowEvent, iFppup)
// _XWireToEvent
GO(XWithdrawWindow, iFppi)
//GO(XWMGeometry
//GO(XWriteBitmapFile
//GO(XXorRegion
