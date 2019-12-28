#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gdk_add_client_message_filter, 
//GO(gdk_add_option_entries_libgtk_only, 
//GO(gdk_app_launch_context_get_type, 
//GO(gdk_app_launch_context_new, 
//GO(gdk_app_launch_context_set_desktop, 
//GO(gdk_app_launch_context_set_display, 
//GO(gdk_app_launch_context_set_icon, 
//GO(gdk_app_launch_context_set_icon_name, 
//GO(gdk_app_launch_context_set_screen, 
//GO(gdk_app_launch_context_set_timestamp, 
//GO(gdk_atom_intern, 
//GO(gdk_atom_intern_static_string, 
//GO(gdk_atom_name, 
//GO(gdk_axis_use_get_type, 
//GO(gdk_beep, 
//GO(gdk_bitmap_create_from_data, 
//GO(gdk_byte_order_get_type, 
//GO(gdk_cairo_create, 
//GO(gdk_cairo_rectangle, 
//GO(gdk_cairo_region, 
//GO(gdk_cairo_reset_clip, 
//GO(gdk_cairo_set_source_color, 
//GO(gdk_cairo_set_source_pixbuf, 
//GO(gdk_cairo_set_source_pixmap, 
//GO(gdk_cairo_set_source_window, 
//GO(gdk_cap_style_get_type, 
//GO(gdk_char_height, 
//GO(gdk_char_measure, 
//GO(gdk_char_width, 
//GO(gdk_char_width_wc, 
GO(gdk_color_alloc, iFpp)
GO(gdk_color_black, iFpp)
GO(gdk_color_change, iFpp)
GO(gdk_color_copy, pFp)
GO(gdk_color_equal, iFpp)
GO(gdk_color_free, vFp)
GO(gdk_color_get_type, iFv)
GO(gdk_color_hash, uFp)
GO(gdk_colormap_alloc_color, iFppii)
GO(gdk_colormap_alloc_colors, iFppiiip)
GO(gdk_colormap_change, vFpi)
GO(gdk_colormap_free_colors, vFppi)
GO(gdk_colormap_get_screen, pFp)
GO(gdk_colormap_get_system, pFv)
GO(gdk_colormap_get_system_size, iFv)
GO(gdk_colormap_get_type, iFv)
GO(gdk_colormap_get_visual, pFp)
GO(gdk_colormap_new, pFpi)
GO(gdk_colormap_query_color, vFpLp)
GO(gdk_colormap_ref, pFp)
GO(gdk_colormap_unref, vFp)
GO(gdk_color_parse, iFpp)
GO(gdk_colors_alloc, iFpipipi)
GO(gdk_colors_free, vFppiL)
GO(gdk_colors_store, vFppi)
GO(gdk_color_to_string, pFp)
GO(gdk_color_white, iFpp)
//GO(gdk_crossing_mode_get_type, 
//GO(gdk_cursor_get_cursor_type, 
//GO(gdk_cursor_get_display, 
//GO(gdk_cursor_get_image, 
//GO(gdk_cursor_get_type, 
//GO(gdk_cursor_new, 
//GO(gdk_cursor_new_for_display, 
//GO(gdk_cursor_new_from_name, 
//GO(gdk_cursor_new_from_pixbuf, 
//GO(gdk_cursor_new_from_pixmap, 
//GO(gdk_cursor_ref, 
//GO(gdk_cursor_type_get_type, 
//GO(gdk_cursor_unref, 
//GO(gdk_device_free_history, 
//GO(gdk_device_get_axis, 
//GO(gdk_device_get_axis_use, 
//GO(gdk_device_get_core_pointer, 
//GO(gdk_device_get_has_cursor, 
//GO(gdk_device_get_history, 
//GO(gdk_device_get_key, 
//GO(gdk_device_get_mode, 
//GO(gdk_device_get_name, 
//GO(gdk_device_get_n_axes, 
//GO(gdk_device_get_n_keys, 
//GO(gdk_device_get_source, 
//GO(gdk_device_get_state, 
//GO(gdk_device_get_type, 
//GO(gdk_device_set_axis_use, 
//GO(gdk_device_set_key, 
//GO(gdk_device_set_mode, 
//GO(gdk_device_set_source, 
//GO(gdk_devices_list, 
//GO(gdk_display_add_client_message_filter, 
//GO(gdk_display_beep, 
//GO(gdk_display_close, 
//GO(gdk_display_flush, 
//GO(gdk_display_get_core_pointer, 
//GO(gdk_display_get_default, 
//GO(gdk_display_get_default_cursor_size, 
//GO(gdk_display_get_default_group, 
//GO(gdk_display_get_default_screen, 
//GO(gdk_display_get_event, 
//GO(gdk_display_get_maximal_cursor_size, 
//GO(gdk_display_get_name, 
//GO(gdk_display_get_n_screens, 
//GO(gdk_display_get_pointer, 
//GO(gdk_display_get_screen, 
//GO(gdk_display_get_type, 
//GO(gdk_display_get_window_at_pointer, 
//GO(gdk_display_is_closed, 
//GO(gdk_display_keyboard_ungrab, 
//GO(gdk_display_list_devices, 
//GO(gdk_display_manager_get, 
//GO(gdk_display_manager_get_default_display, 
//GO(gdk_display_manager_get_type, 
//GO(gdk_display_manager_list_displays, 
//GO(gdk_display_manager_set_default_display, 
//GO(gdk_display_open, 
//GO(gdk_display_open_default_libgtk_only, 
//GO(gdk_display_peek_event, 
//GO(gdk_display_pointer_is_grabbed, 
//GO(gdk_display_pointer_ungrab, 
//GO(gdk_display_put_event, 
//GO(gdk_display_request_selection_notification, 
//GO(gdk_display_set_double_click_distance, 
//GO(gdk_display_set_double_click_time, 
//GO(gdk_display_set_pointer_hooks, 
//GO(gdk_display_store_clipboard, 
//GO(gdk_display_supports_clipboard_persistence, 
//GO(gdk_display_supports_composite, 
//GO(gdk_display_supports_cursor_alpha, 
//GO(gdk_display_supports_cursor_color, 
//GO(gdk_display_supports_input_shapes, 
//GO(gdk_display_supports_selection_notification, 
//GO(gdk_display_supports_shapes, 
//GO(gdk_display_sync, 
//GO(gdk_display_warp_pointer, 
//GO(gdk_drag_abort, 
//GO(gdk_drag_action_get_type, 
//GO(gdk_drag_begin, 
//GO(gdk_drag_context_get_actions, 
//GO(gdk_drag_context_get_dest_window, 
//GO(gdk_drag_context_get_protocol, 
//GO(gdk_drag_context_get_selected_action, 
//GO(gdk_drag_context_get_source_window, 
//GO(gdk_drag_context_get_suggested_action, 
//GO(gdk_drag_context_get_type, 
//GO(gdk_drag_context_list_targets, 
//GO(gdk_drag_context_new, 
//GO(gdk_drag_context_ref, 
//GO(gdk_drag_context_unref, 
//GO(gdk_drag_drop, 
//GO(gdk_drag_drop_succeeded, 
//GO(gdk_drag_find_window, 
//GO(gdk_drag_find_window_for_screen, 
//GO(gdk_drag_get_protocol, 
//GO(gdk_drag_get_protocol_for_display, 
//GO(gdk_drag_get_selection, 
//GO(gdk_drag_motion, 
//GO(gdk_drag_protocol_get_type, 
//GO(gdk_drag_status, 
//GO(gdk_drawable_copy_to_image, 
//GO(gdk_drawable_get_clip_region, 
//GO(gdk_drawable_get_colormap, 
GO(gdk_drawable_get_data, pFpp)
//GO(gdk_drawable_get_depth, 
GO(gdk_drawable_get_display, pFp)
//GO(gdk_drawable_get_image, 
GO(gdk_drawable_get_screen, pFp)
//GO(gdk_drawable_get_size, 
GO(gdk_drawable_get_type, iFv)
//GO(gdk_drawable_get_visible_region, 
GO(gdk_drawable_get_visual, pFp)
//GO(gdk_drawable_ref, 
//GO(gdk_drawable_set_colormap, 
//GO(gdk_drawable_set_data, 
//GO(gdk_drawable_unref, 
//GO(gdk_draw_arc, 
//GO(gdk_draw_drawable, 
//GO(gdk_draw_glyphs, 
//GO(gdk_draw_glyphs_transformed, 
//GO(gdk_draw_gray_image, 
//GO(gdk_draw_image, 
//GO(gdk_draw_indexed_image, 
//GO(gdk_draw_layout, 
//GO(gdk_draw_layout_line, 
//GO(gdk_draw_layout_line_with_colors, 
//GO(gdk_draw_layout_with_colors, 
//GO(gdk_draw_line, 
//GO(gdk_draw_lines, 
//GO(gdk_draw_pixbuf, 
//GO(gdk_draw_point, 
//GO(gdk_draw_points, 
//GO(gdk_draw_polygon, 
//GO(gdk_draw_rectangle, 
//GO(gdk_draw_rgb_32_image, 
//GO(gdk_draw_rgb_32_image_dithalign, 
//GO(gdk_draw_rgb_image, 
//GO(gdk_draw_rgb_image_dithalign, 
//GO(gdk_draw_segments, 
//GO(gdk_draw_string, 
//GO(gdk_draw_text, 
//GO(gdk_draw_text_wc, 
//GO(gdk_draw_trapezoids, 
//GO(gdk_drop_finish, 
//GO(gdk_drop_reply, 
//GO(gdk_error_trap_pop, 
//GO(gdk_error_trap_push, 
//GO(gdk_event_copy, 
//GO(gdk_event_free, 
//GO(gdk_event_get, 
//GO(gdk_event_get_axis, 
//GO(gdk_event_get_coords, 
//GO(gdk_event_get_graphics_expose, 
//GO(gdk_event_get_root_coords, 
//GO(gdk_event_get_screen, 
//GO(gdk_event_get_state, 
//GO(gdk_event_get_time, 
//GO(gdk_event_get_type, 
//GO(gdk_event_handler_set, 
//GO(gdk_event_mask_get_type, 
//GO(gdk_event_new, 
//GO(gdk_event_peek, 
//GO(gdk_event_put, 
//GO(gdk_event_request_motions, 
//GO(gdk_event_send_client_message, 
//GO(gdk_event_send_client_message_for_display, 
//GO(gdk_event_send_clientmessage_toall, 
//GO(gdk_event_set_screen, 
//GO(gdk_events_pending, 
//GO(gdk_event_type_get_type, 
//GO(gdk_exit, 
//GO(gdk_extension_mode_get_type, 
//GO(gdk_fill_get_type, 
//GO(gdk_fill_rule_get_type, 
//GO(gdk_filter_return_get_type, 
GO(gdk_flush, vFv)
//GO(gdk_font_equal, 
//GO(gdk_font_from_description, 
//GO(gdk_font_from_description_for_display, 
//GO(gdk_font_get_display, 
GO(gdk_font_get_type, iFv)
//GO(gdk_font_id, 
//GO(gdk_font_load, 
//GO(gdk_font_load_for_display, 
//GO(gdk_font_ref, 
//GO(gdk_fontset_load, 
//GO(gdk_fontset_load_for_display, 
//GO(gdk_font_type_get_type, 
//GO(gdk_font_unref, 
//GO(gdk_free_compound_text, 
//GO(gdk_free_text_list, 
//GO(gdk_function_get_type, 
GO(gdk_gc_copy, vFpp)
GO(gdk_gc_get_colormap, pFp)
GO(gdk_gc_get_screen, pFp)
GO(gdk_gc_get_type, iFv)
GO(gdk_gc_get_values, vFpp)
GO(gdk_gc_new, pFp)
GO(gdk_gc_new_with_values, pFppu)
GO(gdk_gc_offset, vFpii)
GO(gdk_gc_ref, pFp)
GO(gdk_gc_set_background, vFpp)
GO(gdk_gc_set_clip_mask, vFpp)
GO(gdk_gc_set_clip_origin, vFpii)
GO(gdk_gc_set_clip_rectangle, vFpp)
GO(gdk_gc_set_clip_region, vFpp)
GO(gdk_gc_set_colormap, vFpp)
GO(gdk_gc_set_dashes, vFpipi)
GO(gdk_gc_set_exposures, vFpi)
GO(gdk_gc_set_fill, vFpi)
GO(gdk_gc_set_font, vFpp)
GO(gdk_gc_set_foreground, vFpp)
GO(gdk_gc_set_function,vFpi) 
GO(gdk_gc_set_line_attributes, vFpiiii)
GO(gdk_gc_set_rgb_bg_color, vFpp)
GO(gdk_gc_set_rgb_fg_color, vFpp)
GO(gdk_gc_set_stipple, vFpp)
GO(gdk_gc_set_subwindow, vFpi)
GO(gdk_gc_set_tile, vFpp)
GO(gdk_gc_set_ts_origin, vFpii)
GO(gdk_gc_set_values, vFppu)
GO(gdk_gc_unref, vFp)
GO(gdk_gc_values_mask_get_type, iFv)
GO(gdk_get_default_root_window, pFv)
//GO(gdk_get_display, 
//GO(gdk_get_display_arg_name, 
//GO(gdk_get_program_class, 
//GO(gdk_get_show_events, 
//GO(gdk_get_use_xshm, 
//GO(gdk_grab_status_get_type, 
//GO(gdk_gravity_get_type, 
//GO(gdk_image_get, 
//GO(gdk_image_get_bits_per_pixel, 
//GO(gdk_image_get_byte_order, 
//GO(gdk_image_get_bytes_per_line, 
//GO(gdk_image_get_bytes_per_pixel, 
//GO(gdk_image_get_colormap, 
//GO(gdk_image_get_depth, 
//GO(gdk_image_get_height, 
//GO(gdk_image_get_image_type, 
//GO(gdk_image_get_pixel, 
//GO(gdk_image_get_pixels, 
//GO(gdk_image_get_type, 
//GO(gdk_image_get_visual, 
//GO(gdk_image_get_width, 
//GO(gdk_image_new, 
//GO(gdk_image_new_bitmap, 
//GO(gdk_image_put_pixel, 
//GO(gdk_image_ref, 
//GO(gdk_image_set_colormap, 
//GO(gdk_image_type_get_type, 
//GO(gdk_image_unref, 
//GO(gdk_init, 
//GO(gdk_init_check, 
//GO(gdk_input_add, 
//GO(gdk_input_add_full, 
//GO(gdk_input_condition_get_type, 
//GO(gdk_input_mode_get_type, 
//GO(gdk_input_remove, 
//GO(gdk_input_set_extension_events, 
//GO(gdk_input_source_get_type, 
//GO(gdk_join_style_get_type, 
//GO(gdk_keyboard_grab, 
//GO(gdk_keyboard_grab_info_libgtk_only, 
//GO(gdk_keyboard_ungrab, 
//GO(gdk_keymap_add_virtual_modifiers, 
//GO(gdk_keymap_get_caps_lock_state, 
//GO(gdk_keymap_get_default, 
//GO(gdk_keymap_get_direction, 
//GO(gdk_keymap_get_entries_for_keycode, 
//GO(gdk_keymap_get_entries_for_keyval, 
//GO(gdk_keymap_get_for_display, 
//GO(gdk_keymap_get_type, 
//GO(gdk_keymap_have_bidi_layouts, 
//GO(gdk_keymap_lookup_key, 
//GO(gdk_keymap_map_virtual_modifiers, 
//GO(gdk_keymap_translate_keyboard_state, 
//GO(gdk_keyval_convert_case, 
//GO(gdk_keyval_from_name, 
//GO(gdk_keyval_is_lower, 
//GO(gdk_keyval_is_upper, 
//GO(gdk_keyval_name, 
//GO(gdk_keyval_to_lower, 
//GO(gdk_keyval_to_unicode, 
//GO(gdk_keyval_to_upper, 
//GO(gdk_line_style_get_type, 
//GO(gdk_list_visuals, 
//GO(gdk_mbstowcs, 
//GO(gdk_modifier_type_get_type, 
GO(gdk_net_wm_supports, iFp)
//GO(gdk_notify_startup_complete, 
//GO(gdk_notify_startup_complete_with_id, 
//GO(gdk_notify_type_get_type, 
GO(gdk_offscreen_window_get_embedder, pFp)
GO(gdk_offscreen_window_get_pixmap, pFp)
GO(gdk_offscreen_window_get_type, iFv)
GO(gdk_offscreen_window_set_embedder, vFpp)
GO(gdk_overlap_type_get_type, iFv)
GO(gdk_owner_change_get_type, iFv)
GO(gdk_pango_attr_emboss_color_new, pFp)
GO(gdk_pango_attr_embossed_new, pFi)
GO(gdk_pango_attr_stipple_new, pFp)
GO(gdk_pango_context_get, pFv)
GO(gdk_pango_context_get_for_screen, pFp)
GO(gdk_pango_context_set_colormap, vFpp)
GO(gdk_pango_layout_get_clip_region, pFpiipi)
GO(gdk_pango_layout_line_get_clip_region, pFpiipi)
GO(gdk_pango_renderer_get_default, pFp)
GO(gdk_pango_renderer_get_type, iFv)
GO(gdk_pango_renderer_new, pFp)
GO(gdk_pango_renderer_set_drawable, vFpp)
GO(gdk_pango_renderer_set_gc, vFpp)
GO(gdk_pango_renderer_set_override_color, vFpip)
GO(gdk_pango_renderer_set_stipple, vFpip)
//GO(gdk_parse_args, 
GO(gdk_pixbuf_get_from_drawable, pFpppiiiiii)
GO(gdk_pixbuf_get_from_image, pFpppiiiiii)
GO(gdk_pixbuf_render_pixmap_and_mask, vFpppi)
GO(gdk_pixbuf_render_pixmap_and_mask_for_colormap, vFppppi)
GO(gdk_pixbuf_render_threshold_alpha, vFppiiiiiii)
GO(gdk_pixbuf_render_to_drawable, vFpppiiiiiiiii)
GO(gdk_pixbuf_render_to_drawable_alpha, vFppiiiiiiiiiii)
//GO(gdk_pixmap_colormap_create_from_xpm, 
//GO(gdk_pixmap_colormap_create_from_xpm_d, 
//GO(gdk_pixmap_create_from_data, 
//GO(gdk_pixmap_create_from_xpm, 
//GO(gdk_pixmap_create_from_xpm_d, 
GO(gdk_pixmap_foreign_new, pFp)
GO(gdk_pixmap_foreign_new_for_display, pFpp)
GO(gdk_pixmap_foreign_new_for_screen, pFppiii)
//GO(gdk_pixmap_get_size, 
//GO(gdk_pixmap_get_type, 
//GO(gdk_pixmap_impl_x11_get_type, 
GO(gdk_pixmap_lookup, pFp)
GO(gdk_pixmap_lookup_for_display, pFpp)
//GO(gdk_pixmap_new, 
//GO(gdk_pointer_grab, 
//GO(gdk_pointer_grab_info_libgtk_only, 
//GO(gdk_pointer_is_grabbed, 
//GO(gdk_pointer_ungrab, 
//GO(gdk_pre_parse_libgtk_only, 
//GO(gdk_property_change, 
//GO(gdk_property_delete, 
//GO(gdk_property_get, 
//GO(gdk_property_state_get_type, 
//GO(gdk_prop_mode_get_type, 
//GO(gdk_query_depths, 
//GO(gdk_query_visual_types, 
GO(gdk_rectangle_get_type, iFv)
GO(gdk_rectangle_intersect, iFppp)
GO(gdk_rectangle_union, vFppp)
GO(gdk_region_copy, pFp)
GO(gdk_region_destroy, vFpp)
GO(gdk_region_empty, iFp)
GO(gdk_region_equal, iFpp)
GO(gdk_region_get_clipbox, vFpp)
GO(gdk_region_get_rectangles, vFppp)
GO(gdk_region_intersect, vFpp)
GO(gdk_region_new, pFv)
GO(gdk_region_offset, vFpii)
GO(gdk_region_point_in, iFpii)
GO(gdk_region_polygon, pFpii)
GO(gdk_region_rectangle, pFp)
GO(gdk_region_rect_equal, iFpp)
GO(gdk_region_rect_in, iFpp)
GO(gdk_region_shrink, vFpii)
//GOM(gdk_region_spans_intersect_foreach, vFppiiBp)
GO(gdk_region_subtract, vFpp)
GO(gdk_region_union, vFpp)
GO(gdk_region_union_with_rect, vFpp)
GO(gdk_region_xor, vFpp)
//GO(gdk_rgb_cmap_free, 
//GO(gdk_rgb_cmap_new, 
//GO(gdk_rgb_colormap_ditherable, 
//GO(gdk_rgb_ditherable, 
//GO(gdk_rgb_dither_get_type, 
//GO(gdk_rgb_find_color, 
//GO(gdk_rgb_gc_set_background, 
//GO(gdk_rgb_gc_set_foreground, 
//GO(gdk_rgb_get_colormap, 
//GO(gdk_rgb_get_visual, 
//GO(gdk_rgb_init, 
//GO(gdk_rgb_set_install, 
//GO(gdk_rgb_set_min_colors, 
//GO(gdk_rgb_set_verbose, 
//GO(gdk_rgb_xpixel_from_rgb, 
//GO(gdk_screen_broadcast_client_message, 
//GO(gdk_screen_get_active_window, 
//GO(gdk_screen_get_default, 
//GO(gdk_screen_get_default_colormap, 
//GO(gdk_screen_get_display, 
//GO(gdk_screen_get_font_options, 
//GO(gdk_screen_get_height, 
//GO(gdk_screen_get_height_mm, 
//GO(gdk_screen_get_monitor_at_point, 
//GO(gdk_screen_get_monitor_at_window, 
//GO(gdk_screen_get_monitor_geometry, 
//GO(gdk_screen_get_monitor_height_mm, 
//GO(gdk_screen_get_monitor_plug_name, 
//GO(gdk_screen_get_monitor_width_mm, 
//GO(gdk_screen_get_n_monitors, 
//GO(gdk_screen_get_number, 
//GO(gdk_screen_get_primary_monitor, 
//GO(gdk_screen_get_resolution, 
//GO(gdk_screen_get_rgba_colormap, 
//GO(gdk_screen_get_rgba_visual, 
//GO(gdk_screen_get_rgb_colormap, 
//GO(gdk_screen_get_rgb_visual, 
//GO(gdk_screen_get_root_window, 
//GO(gdk_screen_get_setting, 
//GO(gdk_screen_get_system_colormap, 
//GO(gdk_screen_get_system_visual, 
//GO(gdk_screen_get_toplevel_windows, 
//GO(gdk_screen_get_type, 
//GO(gdk_screen_get_width, 
//GO(gdk_screen_get_width_mm, 
//GO(gdk_screen_get_window_stack, 
GO(gdk_screen_height, iFv)
GO(gdk_screen_height_mm, iFv)
//GO(gdk_screen_is_composited, 
//GO(gdk_screen_list_visuals, 
//GO(gdk_screen_make_display_name, 
//GO(gdk_screen_set_default_colormap, 
//GO(gdk_screen_set_font_options, 
//GO(gdk_screen_set_resolution, 
GO(gdk_screen_width, iFv)
GO(gdk_screen_width_mm, iFv)
//GO(gdk_scroll_direction_get_type, 
//GO(gdk_selection_convert, 
//GO(gdk_selection_owner_get, 
//GO(gdk_selection_owner_get_for_display, 
//GO(gdk_selection_owner_set, 
//GO(gdk_selection_owner_set_for_display, 
//GO(gdk_selection_property_get, 
//GO(gdk_selection_send_notify, 
//GO(gdk_selection_send_notify_for_display, 
//GO(gdk_set_double_click_time, 
//GO(gdk_set_locale, 
//GOM(gdk_set_pointer_hooks, BFEB)
//GO(gdk_set_program_class, 
//GO(gdk_set_show_events, 
//GO(gdk_set_sm_client_id, 
//GO(gdk_setting_action_get_type, 
//GO(gdk_setting_get, 
//GO(gdk_set_use_xshm, 
//GO(gdk_spawn_command_line_on_screen, 
//GO(gdk_spawn_on_screen, 
//GO(gdk_spawn_on_screen_with_pipes, 
//GO(gdk_status_get_type, 
//GO(gdk_string_extents, 
//GO(gdk_string_height, 
//GO(gdk_string_measure, 
//GO(gdk_string_to_compound_text, 
//GO(gdk_string_to_compound_text_for_display, 
//GO(gdk_string_width, 
//GO(gdk_subwindow_mode_get_type, 
//GO(gdk_synthesize_window_state, 
//GO(gdk_test_render_sync, 
//GO(gdk_test_simulate_button, 
//GO(gdk_test_simulate_key, 
//GO(gdk_text_extents, 
//GO(gdk_text_extents_wc, 
//GO(gdk_text_height, 
//GO(gdk_text_measure, 
//GO(gdk_text_property_to_text_list, 
//GO(gdk_text_property_to_text_list_for_display, 
//GO(gdk_text_property_to_utf8_list, 
//GO(gdk_text_property_to_utf8_list_for_display, 
//GO(gdk_text_width, 
//GO(gdk_text_width_wc, 
//GO(gdk_threads_add_idle, 
//GO(gdk_threads_add_idle_full, 
//GO(gdk_threads_add_timeout, 
//GO(gdk_threads_add_timeout_full, 
//GO(gdk_threads_add_timeout_seconds, 
//GO(gdk_threads_add_timeout_seconds_full, 
//GO(gdk_threads_enter, 
//GO(gdk_threads_init, 
//GO(gdk_threads_leave, 
//GO(gdk_threads_set_lock_functions, 
//GO(gdk_unicode_to_keyval, 
//GO(gdk_utf8_to_compound_text, 
//GO(gdk_utf8_to_compound_text_for_display, 
//GO(gdk_utf8_to_string_target, 
//GO(gdk_visibility_state_get_type, 
//GO(gdk_visual_get_best, 
//GO(gdk_visual_get_best_depth, 
//GO(gdk_visual_get_best_type, 
//GO(gdk_visual_get_best_with_both, 
//GO(gdk_visual_get_best_with_depth, 
//GO(gdk_visual_get_best_with_type, 
//GO(gdk_visual_get_bits_per_rgb, 
//GO(gdk_visual_get_blue_pixel_details, 
//GO(gdk_visual_get_byte_order, 
//GO(gdk_visual_get_colormap_size, 
//GO(gdk_visual_get_depth, 
//GO(gdk_visual_get_green_pixel_details, 
//GO(gdk_visual_get_red_pixel_details, 
//GO(gdk_visual_get_screen, 
//GO(gdk_visual_get_system, 
//GO(gdk_visual_get_type, 
//GO(gdk_visual_get_visual_type, 
//GO(gdk_visual_type_get_type, 
//GO(gdk_wcstombs, 
//GOM(gdk_window_add_filter, vFEpBp)
GO(gdk_window_at_pointer, pFpp)
//GO(gdk_window_attributes_type_get_type, 
GO(gdk_window_beep, vFp)
GO(gdk_window_begin_move_drag, vFpiiiu)
GO(gdk_window_begin_paint_rect, vFpp)
GO(gdk_window_begin_paint_region, vFpp)
GO(gdk_window_begin_resize_drag, vFpuiiiu)
GO(gdk_window_class_get_type, iFv)
GO(gdk_window_clear, vFp)
GO(gdk_window_clear_area, vFpiiii)
GO(gdk_window_clear_area_e, vFpiiii)
GO(gdk_window_configure_finished, vFp)
GO(gdk_window_constrain_size, vFpuiipp)
GO(gdk_window_coords_from_parent, vFpddpp)
GO(gdk_window_coords_to_parent, vFpddpp)
//GO(gdk_window_create_similar_surface, 
GO(gdk_window_deiconify, vFp)
GO(gdk_window_destroy, vFp)
//GO(gdk_window_destroy_notify, 
//GO(gdk_window_edge_get_type, 
GO(gdk_window_enable_synchronized_configure, vFp)
GO(gdk_window_end_paint, vFp)
GO(gdk_window_ensure_native, iFp)
GO(gdk_window_flush, vFp)
GO(gdk_window_focus, vFpu)
GO(gdk_window_foreign_new, pFp)
GO(gdk_window_foreign_new_for_display, pFpp)
//GO(gdk_window_freeze_toplevel_updates_libgtk_only, 
GO(gdk_window_freeze_updates, vFp)
GO(gdk_window_fullscreen, vFp)
GO(gdk_window_geometry_changed, vFp)
GO(gdk_window_get_accept_focus, iFp)
GO(gdk_window_get_background_pattern, pFp)
GO(gdk_window_get_children, pFp)
GO(gdk_window_get_composited, iFp)
GO(gdk_window_get_cursor, pFp)
GO(gdk_window_get_decorations, iFpp)
GO(gdk_window_get_deskrelative_origin, iFppp)
GO(gdk_window_get_display, pFp)
GO(gdk_window_get_effective_parent, pFp)
GO(gdk_window_get_effective_toplevel, pFp)
GO(gdk_window_get_events, vFpi)
GO(gdk_window_get_focus_on_map, iFp)
GO(gdk_window_get_frame_extents, vFpp)
GO(gdk_window_get_geometry, vFpppppp)
GO(gdk_window_get_group, pFp)
GO(gdk_window_get_height, iFp)
GO(gdk_window_get_internal_paint_info, vFpppp)
GO(gdk_window_get_modal_hint, iFp)
GO(gdk_window_get_origin, iFpp)
GO(gdk_window_get_parent, pFp)
GO(gdk_window_get_pointer, pFpppp)
GO(gdk_window_get_position, vFppp)
GO(gdk_window_get_root_coords, vFpiipp)
GO(gdk_window_get_root_origin, vFpp)
GO(gdk_window_get_screen, pFp)
GO(gdk_window_get_state, iFp)
GO(gdk_window_get_toplevel, pFp)
GO(gdk_window_get_toplevels, pFp)
GO(gdk_window_get_type_hint, iFp)
GO(gdk_window_get_update_area, pFp)
GO(gdk_window_get_user_data, vFpp)
GO(gdk_window_get_visual, pFp)
GO(gdk_window_get_width, iFp)
GO(gdk_window_get_window_type, iFp)
GO(gdk_window_has_native, iFp)
GO(gdk_window_hide, vFp)
GO(gdk_window_hints_get_type, iFv)
GO(gdk_window_iconify, vFp)
GO(gdk_window_impl_get_type, iFv)
GO(gdk_window_impl_x11_get_type, iFv)
GO(gdk_window_input_shape_combine_mask, vFppii)
GO(gdk_window_input_shape_combine_region, vFppii)
//GOM(gdk_window_invalidate_maybe_recurse, vFEppBp)
GO(gdk_window_invalidate_rect, vFppi)
GO(gdk_window_invalidate_region, vFppi)
GO(gdk_window_is_destroyed, iFp)
GO(gdk_window_is_input_only, iFp)
GO(gdk_window_is_shaped, iFp)
GO(gdk_window_is_viewable, iFp)
GO(gdk_window_is_visible, iFp)
GO(gdk_window_lookup, pFp)
GO(gdk_window_lookup_for_display, pFpp)
GO(gdk_window_lower, vFp)
GO(gdk_window_maximize, vFp)
GO(gdk_window_merge_child_input_shapes, vFp)
GO(gdk_window_merge_child_shapes, vFp)
GO(gdk_window_move, vFpii)
GO(gdk_window_move_region, vFppii)
GO(gdk_window_move_resize, vFpiiii)
GO(gdk_window_new, pFppi)
GO(gdk_window_object_get_type, iFv)
GO(gdk_window_peek_children, pFp)
GO(gdk_window_process_all_updates, vFv)
GO(gdk_window_process_updates, vFpi)
GO(gdk_window_raise, vFp)
GO(gdk_window_redirect_to_drawable, vFppiiiiii)
GO(gdk_window_register_dnd, vFp)
//GOM(gdk_window_remove_filter, vFEpBp)
GO(gdk_window_remove_redirection, vFp)
GO(gdk_window_reparent, vFppii)
GO(gdk_window_resize, vFpii)
GO(gdk_window_restack, vFppi)
GO(gdk_window_scroll, vFpii)
GO(gdk_window_set_accept_focus, vFpi)
GO(gdk_window_set_background, vFpp)
GO(gdk_window_set_back_pixmap, vFppi)
GO(gdk_window_set_child_input_shapes, vFp)
GO(gdk_window_set_child_shapes, vFp)
GO(gdk_window_set_composited, vFpi)
GO(gdk_window_set_cursor, vFpp)
GO(gdk_window_set_debug_updates, vFi)
GO(gdk_window_set_decorations, vFpp)
GO(gdk_window_set_events, vFpp)
GO(gdk_window_set_focus_on_map, vFpi)
GO(gdk_window_set_functions, vFpu)
GO(gdk_window_set_geometry_hints, vFppi)
GO(gdk_window_set_group, vFpp)
GO(gdk_window_set_hints, vFpiiiiiii)
GO(gdk_window_set_icon, vFpppp)
GO(gdk_window_set_icon_list, vFpp)
GO(gdk_window_set_icon_name, vFpp)
GO(gdk_window_set_keep_above, vFpi)
GO(gdk_window_set_keep_below, vFpi)
GO(gdk_window_set_modal_hint, iFp)
GO(gdk_window_set_opacity, vFpi)
GO(gdk_window_set_override_redirect, vFpi)
GO(gdk_window_set_role, vFpp)
GO(gdk_window_set_skip_pager_hint, vFpi)
GO(gdk_window_set_skip_taskbar_hint, vFpi)
GO(gdk_window_set_startup_id, vFpp)
GO(gdk_window_set_static_gravities, iFpi)
GO(gdk_window_set_title, vFpp)
GO(gdk_window_set_transient_for, vFpp)
GO(gdk_window_set_type_hint, vFpi)
GO(gdk_window_set_urgency_hint, vFpi)
GO(gdk_window_set_user_data, vFpp)
GO(gdk_window_shape_combine_mask, vFppii)
GO(gdk_window_shape_combine_region, vFppii)
GO(gdk_window_show, vFp)
GO(gdk_window_show_unraised, vFp)
GO(gdk_window_state_get_type, iFv)
GO(gdk_window_stick, vFp)
//GO(gdk_window_thaw_toplevel_updates_libgtk_only, 
GO(gdk_window_thaw_updates, vFp)
GO(gdk_window_type_get_type, iFv)
GO(gdk_window_type_hint_get_type, iFv)
GO(gdk_window_unfullscreen, vFp)
GO(gdk_window_unmaximize, vFp)
GO(gdk_window_unstick, vFp)
GO(gdk_window_withdraw, vFp)
GO(gdk_wm_decoration_get_type, iFv)
GO(gdk_wm_function_get_type, iFv)
GO(gdk_x11_atom_to_xatom, pFp)
GO(gdk_x11_atom_to_xatom_for_display, pFpp)
GO(gdk_x11_colormap_foreign_new, pFpp)
GO(gdk_x11_colormap_get_xcolormap, pFp)
GO(gdk_x11_colormap_get_xdisplay, pFp)
GO(gdk_x11_cursor_get_xcursor, pFp)
GO(gdk_x11_cursor_get_xdisplay, pFp)
GO(gdk_x11_display_broadcast_startup_message, vFppppppppppppp)  //vaarg after 2 p
GO(gdk_x11_display_get_startup_notification_id, pFp)
GO(gdk_x11_display_get_user_time, uFp)
GO(gdk_x11_display_get_xdisplay, pFp)
GO(gdk_x11_display_grab, vFp)
GO(gdk_x11_display_set_cursor_theme, vFppi)
//GO(gdk_x11_display_string_to_compound_text, 
//GO(gdk_x11_display_text_property_to_text_list, 
GO(gdk_x11_display_ungrab, vFp)
//GO(gdk_x11_display_utf8_to_compound_text, 
GO(gdk_x11_drawable_get_xdisplay, pFp)
GO(gdk_x11_drawable_get_xid, pFp)
GO(gdk_x11_font_get_name, pFp)
GO(gdk_x11_font_get_xdisplay, pFp)
GO(gdk_x11_font_get_xfont, pFp)
//GO(gdk_x11_free_compound_text, 
//GO(gdk_x11_free_text_list, 
GO(gdk_x11_gc_get_xdisplay, pFp)
GO(gdk_x11_gc_get_xgc, pFp)
GO(gdk_x11_get_default_root_xwindow, pFv)
GO(gdk_x11_get_default_screen, iFv)
GO(gdk_x11_get_default_xdisplay, pFv)
GO(gdk_x11_get_server_time, uFp)
GO(gdk_x11_get_xatom_by_name, pFp)
GO(gdk_x11_get_xatom_by_name_for_display, pFpp)
GO(gdk_x11_get_xatom_name, pFp)
GO(gdk_x11_get_xatom_name_for_display, pFpp)
GO(gdk_x11_grab_server, vFv)
GO(gdk_x11_image_get_xdisplay, pFp)
GO(gdk_x11_image_get_ximage, pFp)
GO(gdk_x11_lookup_xdisplay, pFp)
//GO(gdk_x11_pixmap_get_drawable_impl, 
GO(gdk_x11_register_standard_event_type, vFpii)
GO(gdk_x11_screen_get_monitor_output, pFpi)
GO(gdk_x11_screen_get_screen_number, iFp)
GO(gdk_x11_screen_get_window_manager_name, pFp)
GO(gdk_x11_screen_get_xscreen, pFp)
GO(gdk_x11_screen_lookup_visual, pFpp)
GO(gdk_x11_screen_supports_net_wm_hint, iFpp)
GO(gdk_x11_set_sm_client_id, vFp)
GO(gdk_x11_ungrab_server, vFv)
GO(gdk_x11_visual_get_xvisual, pFp)
GO(gdk_x11_window_foreign_new_for_display, pFpp)
//GO(gdk_x11_window_get_drawable_impl, 
GO(gdk_x11_window_lookup_for_display, pFpp)
GO(gdk_x11_window_move_to_current_desktop, vFp)
GO(gdk_x11_window_set_user_time, vFpu)
GO(gdk_x11_xatom_to_atom, pFp)
GO(gdk_x11_xatom_to_atom_for_display, pFpp)
GO(gdkx_colormap_get, pFu)
GO(gdk_xid_table_lookup, pFp)
GO(gdk_xid_table_lookup_for_display, pFpp)
GO(gdkx_visual_get, pFp)
