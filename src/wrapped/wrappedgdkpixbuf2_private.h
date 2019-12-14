#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gdk_colorspace_get_type, 
//GO(gdk_interp_type_get_type, 
//GO(gdk_pixbuf_add_alpha, 
//GO(gdk_pixbuf_alpha_mode_get_type, 
//GO(gdk_pixbuf_animation_get_height, 
//GO(gdk_pixbuf_animation_get_iter, 
//GO(gdk_pixbuf_animation_get_static_image, 
//GO(gdk_pixbuf_animation_get_type, 
//GO(gdk_pixbuf_animation_get_width, 
//GO(gdk_pixbuf_animation_is_static_image, 
//GO(gdk_pixbuf_animation_iter_advance, 
//GO(gdk_pixbuf_animation_iter_get_delay_time, 
//GO(gdk_pixbuf_animation_iter_get_pixbuf, 
//GO(gdk_pixbuf_animation_iter_get_type, 
//GO(gdk_pixbuf_animation_iter_on_currently_loading_frame, 
//GO(gdk_pixbuf_animation_new_from_file, 
//GO(gdk_pixbuf_animation_new_from_resource, 
//GO(gdk_pixbuf_animation_new_from_stream, 
//GO(gdk_pixbuf_animation_new_from_stream_async, 
//GO(gdk_pixbuf_animation_new_from_stream_finish, 
//GO(gdk_pixbuf_animation_ref, 
//GO(gdk_pixbuf_animation_unref, 
//GO(gdk_pixbuf_apply_embedded_orientation, 
//GO(gdk_pixbuf_composite, 
//GO(gdk_pixbuf_composite_color, 
//GO(gdk_pixbuf_composite_color_simple, 
//GO(gdk_pixbuf_copy, 
//GO(gdk_pixbuf_copy_area, 
//GO(gdk_pixbuf_error_get_type, 
//GO(gdk_pixbuf_error_quark, 
//GO(gdk_pixbuf_fill, 
//GO(gdk_pixbuf_flip, 
//GO(gdk_pixbuf_format_copy, 
//GO(gdk_pixbuf_format_free, 
//GO(gdk_pixbuf_format_get_description, 
//GO(gdk_pixbuf_format_get_extensions, 
//GO(gdk_pixbuf_format_get_license, 
//GO(gdk_pixbuf_format_get_mime_types, 
//GO(gdk_pixbuf_format_get_name, 
//GO(gdk_pixbuf_format_get_type, 
//GO(gdk_pixbuf_format_is_disabled, 
//GO(gdk_pixbuf_format_is_scalable, 
//GO(gdk_pixbuf_format_is_writable, 
//GO(gdk_pixbuf_format_set_disabled, 
//GO(gdk_pixbuf_from_pixdata, 
GO(gdk_pixbuf_get_bits_per_sample, iFp)
GO(gdk_pixbuf_get_byte_length, uFp)
GO(gdk_pixbuf_get_colorspace, uFp)
GO(gdk_pixbuf_get_file_info, pFppp)
//GO(gdk_pixbuf_get_formats, 
GO(gdk_pixbuf_get_has_alpha, iFp)
GO(gdk_pixbuf_get_height, iFp)
GO(gdk_pixbuf_get_n_channels, iFp)
GO(gdk_pixbuf_get_option, pFpp)
GO(gdk_pixbuf_get_pixels, pFp)
GO(gdk_pixbuf_get_pixels_with_length, pFpp)
GO(gdk_pixbuf_get_rowstride, iFp)
//GO(gdk_pixbuf_gettext, 
//GO(gdk_pixbuf_get_type, 
GO(gdk_pixbuf_get_width, iFp)
//GO(gdk_pixbuf_loader_close, 
//GO(gdk_pixbuf_loader_get_animation, 
//GO(gdk_pixbuf_loader_get_format, 
//GO(gdk_pixbuf_loader_get_pixbuf, 
//GO(gdk_pixbuf_loader_get_type, 
//GO(gdk_pixbuf_loader_new, 
//GO(gdk_pixbuf_loader_new_with_mime_type, 
//GO(gdk_pixbuf_loader_new_with_type, 
//GO(gdk_pixbuf_loader_set_size, 
//GO(gdk_pixbuf_loader_write, 
//GO(gdk_pixbuf_loader_write_bytes, 
//GO(gdk_pixbuf_new, 
GOM(gdk_pixbuf_new_from_data, pFEpiiiiiipp)
GO(gdk_pixbuf_new_from_file, pFpp)
GO(gdk_pixbuf_new_from_file_at_scale, pFpiiip)
GO(gdk_pixbuf_new_from_file_at_size, pFpiip)
//GO(gdk_pixbuf_new_from_inline, 
GO(gdk_pixbuf_new_from_resource, pFpp)
GO(gdk_pixbuf_new_from_resource_at_scale, pFpiiip)
GO(gdk_pixbuf_new_from_stream, pFppp)
//GO(gdk_pixbuf_new_from_stream_async, 
//GO(gdk_pixbuf_new_from_stream_at_scale, 
//GO(gdk_pixbuf_new_from_stream_at_scale_async, 
//GO(gdk_pixbuf_new_from_stream_finish, 
//GO(gdk_pixbuf_new_from_xpm_data, 
//GO(gdk_pixbuf_new_subpixbuf, 
//GO(gdk_pixbuf_non_anim_get_type, 
//GO(gdk_pixbuf_non_anim_new, 
//GO(gdk_pixbuf_ref, 
//GO(gdk_pixbuf_rotate_simple, 
//GO(gdk_pixbuf_rotation_get_type, 
//GO(gdk_pixbuf_saturate_and_pixelate, 
//GO(gdk_pixbuf_save, 
//GO(gdk_pixbuf_save_to_buffer, 
//GO(gdk_pixbuf_save_to_bufferv, 
//GO(gdk_pixbuf_save_to_callback, 
//GO(gdk_pixbuf_save_to_callbackv, 
//GO(gdk_pixbuf_save_to_stream, 
//GO(gdk_pixbuf_save_to_stream_async, 
//GO(gdk_pixbuf_save_to_stream_finish, 
//GO(gdk_pixbuf_savev, 
//GO(gdk_pixbuf_scale, 
//GO(gdk_pixbuf_scaled_anim_get_type, 
//GO(gdk_pixbuf_scaled_anim_iter_get_type, 
//GO(gdk_pixbuf_scale_simple, 
GO(gdk_pixbuf_set_option, pFppp)
//GO(gdk_pixbuf_simple_anim_add_frame, 
//GO(gdk_pixbuf_simple_anim_get_loop, 
//GO(gdk_pixbuf_simple_anim_get_type, 
//GO(gdk_pixbuf_simple_anim_iter_get_type, 
//GO(gdk_pixbuf_simple_anim_new, 
//GO(gdk_pixbuf_simple_anim_set_loop, 
//GO(gdk_pixbuf_unref, 
//GO(gdk_pixdata_deserialize, 
//GO(gdk_pixdata_from_pixbuf, 
//GO(gdk_pixdata_serialize, 
//GO(gdk_pixdata_to_csource, 
