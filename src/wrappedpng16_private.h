#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA)

GO(png_create_write_struct, pFpppp)
GO(png_destroy_write_struct, vFpp)
GO(png_write_row, vFpp)
GO(png_set_compression_level, vFpi)
GO(png_set_tIME, vFppp)
GO(png_set_IHDR, vFppuuiiiii)
GO(png_write_end, vFpp)
GO(png_write_chunk, vFpppu)
GO(png_convert_from_time_t, vFpu)
GO(png_create_info_struct, pFp)
GO(png_init_io, vFpp)
GO(png_set_PLTE, vFpppi)
GO(png_write_info, vFpp)

#endif