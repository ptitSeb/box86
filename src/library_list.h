#ifdef GO

GO("libc.so.6", libc)
GO("libpthread.so.0", libpthread)
GO("librt.so.1", librt)
GO("libGL.so.1", libgl)
GO("libX11.so.6", libx11)
GO("libasound.so.2", libasound)
GO("libdl.so.2", libdl)
GO("libm.so.6", libm)
GO("libSDL2-2.0.so.0", sdl2)
GO("libSDL2_mixer-2.0.so.0", sdl2mixer)
GO("libSDL2_image-2.0.so.0", sdl2image)
GO("libSDL-1.2.so.0", sdl1)
GO("libSDL_mixer-1.2.so.0", sdl1mixer)
GO("libSDL_image-1.2.so.0", sdl1image)
GO("libsmpeg2-2.0.so.0", smpeg2)
GO("libvorbisfile.so.3", vorbisfile)

GO("ld-linux.so.2", ldlinux)

#else
#error Nope
#endif