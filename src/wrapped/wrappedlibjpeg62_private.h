#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif


GO(jcopy_block_row, vFppu)
GO(jcopy_sample_rows, vFpipiiu)
GO(jdiv_round_up, iFii)
GO(jinit_1pass_quantizer, vFp)
GO(jinit_2pass_quantizer, vFp)
GO(jinit_arith_decoder, vFp)
GO(jinit_arith_encoder, vFp)
GO(jinit_c_coef_controller, vFpi)
GO(jinit_c_main_controller, vFpi)
GO(jinit_c_master_control, vFpi)
GO(jinit_color_converter, vFp)
GO(jinit_color_deconverter, vFp)
GO(jinit_compress_master, vFp)
GO(jinit_c_prep_controller, vFpi)
GO(jinit_d_coef_controller, vFpi)
GO(jinit_d_main_controller, vFpi)
GO(jinit_downsampler, vFp)
GO(jinit_d_post_controller, vFpi)
GO(jinit_forward_dct, vFp)
GO(jinit_huff_decoder, vFp)
GO(jinit_huff_encoder, vFp)
GO(jinit_input_controller, vFp)
GO(jinit_inverse_dct, vFp)
GO(jinit_marker_reader, vFp)
GO(jinit_marker_writer, vFp)
GO(jinit_master_decompress, vFp)
GO(jinit_memory_mgr, vFp)
GO(jinit_merged_upsampler, vFp)
GO(jinit_phuff_decoder, vFp)
GO(jinit_phuff_encoder, vFp)
GO(jinit_upsampler, vFp)
GO(jpeg_abort, vFp)
GO(jpeg_abort_compress, vFp)
GO(jpeg_abort_decompress, vFp)
GO(jpeg_add_quant_table, vFpipii)
GO(jpeg_alloc_huff_table, pFp)
GO(jpeg_alloc_quant_table, pFp)
DATA(jpeg_aritab,4) //R
GO(jpeg_calc_jpeg_dimensions, vFp)
GO(jpeg_calc_output_dimensions, vFp)
GO(jpeg_consume_input, iFp)
GO(jpeg_copy_critical_parameters, vFp)
GO(jpeg_core_output_dimensions, vFp)
GOM(jpeg_CreateCompress, vFEpiL)
GOM(jpeg_CreateDecompress, vFEpiL)
GO(jpeg_crop_scanline, vFppp)
GO(jpeg_default_colorspace, vFp)
GO(jpeg_default_qtables, vFpi)
GO(jpeg_destroy, vFp)
GOM(jpeg_destroy_compress, vFEp)
GOM(jpeg_destroy_decompress, vFEp)
//GO(jpeg_fdct_float
//GO(jpeg_fdct_ifast
//GO(jpeg_fdct_islow
//GO(jpeg_fill_bit_buffer
GOM(jpeg_finish_compress, vFEp)
GOM(jpeg_finish_decompress, iFEp)
GO(jpeg_finish_output, iFp)
//GO(jpeg_free_large
//GO(jpeg_free_small
//GO(jpeg_gen_optimal_table
//GO(jpeg_get_large
//GO(jpeg_get_small
GO(jpeg_has_multiple_scans, iFp)
//GO(jpeg_huff_decode
//GO(jpeg_idct_10x10
//GO(jpeg_idct_11x11
//GO(jpeg_idct_12x12
//GO(jpeg_idct_13x13
//GO(jpeg_idct_14x14
//GO(jpeg_idct_15x15
//GO(jpeg_idct_16x16
//GO(jpeg_idct_1x1
//GO(jpeg_idct_2x2
//GO(jpeg_idct_3x3
//GO(jpeg_idct_4x4
//GO(jpeg_idct_5x5
//GO(jpeg_idct_6x6
//GO(jpeg_idct_7x7
//GO(jpeg_idct_9x9
//GO(jpeg_idct_float
//GO(jpeg_idct_ifast
//GO(jpeg_idct_islow
GO(jpeg_input_complete, iFp)
//GO(jpeg_make_c_derived_tbl
//GO(jpeg_make_d_derived_tbl
//GO(jpeg_mem_available
GOM(jpeg_mem_dest, vFEppp)
//GO(jpeg_mem_init
GO(jpeg_mem_src, vFppu)
//GO(jpeg_mem_term
DATA(jpeg_natural_order,4)  //R
GO(jpeg_new_colormap, vFp)
//GO(jpeg_open_backing_store
GO(jpeg_quality_scaling, iFi)
GO(jpeg_read_coefficients, pFp)
GOM(jpeg_read_header, iFEpi)
GO(jpeg_read_raw_data, uFppu)
GOM(jpeg_read_scanlines, uFEppu)
GOM(jpeg_resync_to_restart, iFEpi)
GO(jpeg_save_markers, vFpiu)
GO(jpeg_set_colorspace, vFpi)
GOM(jpeg_set_defaults, vFEp)
GO(jpeg_set_linear_quality, vFpii)
GOM(jpeg_set_marker_processor, vFEpip)
GOM(jpeg_set_quality, vFEpii)
GOM(jpeg_simd_cpu_support, iFv) //%noE
GO(jpeg_simple_progression, vFp)
GO(jpeg_skip_scanlines, uFpu)
GOM(jpeg_start_compress, vFEpi)
GOM(jpeg_start_decompress, iFEp)
GO(jpeg_start_output, iFpi)
GOM(jpeg_std_error, pFEp)
GOM(jpeg_stdio_dest, vFEpp)
GOM(jpeg_stdio_src, vFEpp)
DATA(jpeg_std_message_table, 4)
GO(jpeg_suppress_tables, vFpi)
GO(jpeg_write_coefficients, vFpp)
GOM(jpeg_write_marker, vFEpipu)
GO(jpeg_write_m_byte, vFpi)
GO(jpeg_write_m_header, vFpiu)
GO(jpeg_write_raw_data, uFppu)
GOM(jpeg_write_scanlines, uFEppu)
GO(jpeg_write_tables, vFp)
GO(jround_up, iFii)
GO(jzero_far, vFpu)

//dummy for the bridge wrappers
GO(jppeg_dummy_virt_array, pFpiiuuu)
GO(jppeg_dummy_virt_access, pFppuui)
