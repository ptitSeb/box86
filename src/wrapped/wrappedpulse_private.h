#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(pa_ascii_filter, 
//GO(pa_ascii_valid, 
//GO(pa_bytes_per_second, 
//GO(pa_bytes_snprint, 
//GO(pa_bytes_to_usec, 
//GO(pa_channel_map_can_balance, 
//GO(pa_channel_map_can_fade, 
//GO(pa_channel_map_compatible, 
//GO(pa_channel_map_equal, 
//GO(pa_channel_map_init, 
//GO(pa_channel_map_init_auto, 
GO(pa_channel_map_init_extend, pFpui)
//GO(pa_channel_map_init_mono, 
//GO(pa_channel_map_init_stereo, 
//GO(pa_channel_map_parse, 
GO(pa_channel_map_snprint, pFpup)
//GO(pa_channel_map_superset, 
//GO(pa_channel_map_to_name, 
//GO(pa_channel_map_to_pretty_name, 
//GO(pa_channel_map_valid, 
//GO(pa_channel_position_to_pretty_string, 
//GO(pa_channel_position_to_string, 
//GO(pa_context_add_autoload, 
GOM(pa_context_connect, iFEppip)
GO(pa_context_disconnect, vFp)
//GO(pa_context_drain, 
GO(pa_context_errno, iFp)
//GO(pa_context_exit_daemon, 
//GO(pa_context_get_autoload_info_by_index, 
//GO(pa_context_get_autoload_info_by_name, 
//GO(pa_context_get_autoload_info_list, 
//GO(pa_context_get_card_info_by_index, 
//GO(pa_context_get_card_info_by_name, 
//GO(pa_context_get_card_info_list, 
//GO(pa_context_get_client_info, 
//GO(pa_context_get_client_info_list, 
//GO(pa_context_get_index, 
//GO(pa_context_get_module_info, 
GOM(pa_context_get_module_info_list, pFEppp)
//GO(pa_context_get_protocol_version, 
//GO(pa_context_get_sample_info_by_index, 
//GO(pa_context_get_sample_info_by_name, 
//GO(pa_context_get_sample_info_list, 
//GO(pa_context_get_server, 
GOM(pa_context_get_server_info, pFEppp)
//GO(pa_context_get_server_protocol_version, 
//GO(pa_context_get_sink_info_by_index, 
//GO(pa_context_get_sink_info_by_name, 
GOM(pa_context_get_sink_info_list, pFEppp)
//GO(pa_context_get_sink_input_info, 
GOM(pa_context_get_sink_input_info_list, pFEppp)
//GO(pa_context_get_source_info_by_index, 
//GO(pa_context_get_source_info_by_name, 
//GO(pa_context_get_source_info_list, 
//GO(pa_context_get_source_output_info, 
//GO(pa_context_get_source_output_info_list, 
GO(pa_context_get_state, iFp)
//GO(pa_context_is_local, 
//GO(pa_context_is_pending, 
//GO(pa_context_kill_client, 
//GO(pa_context_kill_sink_input, 
//GO(pa_context_kill_source_output, 
GOM(pa_context_load_module, pFEppppp)
GOM(pa_context_move_sink_input_by_index, pFEpuupp)
//GO(pa_context_move_sink_input_by_name, 
//GO(pa_context_move_source_output_by_index, 
//GO(pa_context_move_source_output_by_name, 
GOM(pa_context_new, pFEpp)
//GO(pa_context_new_with_proplist, 
//GO(pa_context_play_sample, 
//GO(pa_context_play_sample_with_proplist, 
//GO(pa_context_proplist_remove, 
//GO(pa_context_proplist_update, 
//GO(pa_context_ref, 
//GO(pa_context_remove_autoload_by_index, 
//GO(pa_context_remove_autoload_by_name, 
//GO(pa_context_remove_sample, 
//GO(pa_context_set_card_profile_by_index, 
//GO(pa_context_set_card_profile_by_name, 
GOM(pa_context_set_default_sink, pFEpppp)
//GO(pa_context_set_default_source, 
//GO(pa_context_set_event_callback, 
//GO(pa_context_set_name, 
//GO(pa_context_set_sink_input_mute, 
//GO(pa_context_set_sink_input_volume, 
//GO(pa_context_set_sink_mute_by_index, 
//GO(pa_context_set_sink_mute_by_name, 
//GO(pa_context_set_sink_volume_by_index, 
//GO(pa_context_set_sink_volume_by_name, 
//GO(pa_context_set_source_mute_by_index, 
//GO(pa_context_set_source_mute_by_name, 
//GO(pa_context_set_source_volume_by_index, 
//GO(pa_context_set_source_volume_by_name, 
GOM(pa_context_set_state_callback, vFEppp)
//GO(pa_context_set_subscribe_callback, 
//GO(pa_context_stat, 
//GO(pa_context_subscribe, 
//GO(pa_context_suspend_sink_by_index, 
//GO(pa_context_suspend_sink_by_name, 
//GO(pa_context_suspend_source_by_index, 
//GO(pa_context_suspend_source_by_name, 
GOM(pa_context_unload_module, pFEpupp)
GO(pa_context_unref, vFp)
//GO(pa_cvolume_avg, 
//GO(pa_cvolume_channels_equal_to, 
//GO(pa_cvolume_compatible, 
//GO(pa_cvolume_compatible_with_channel_map, 
//GO(pa_cvolume_equal, 
//GO(pa_cvolume_get_balance, 
//GO(pa_cvolume_get_fade, 
//GO(pa_cvolume_init, 
//GO(pa_cvolume_max, 
//GO(pa_cvolume_remap, 
//GO(pa_cvolume_scale, 
//GO(pa_cvolume_set, 
//GO(pa_cvolume_set_balance, 
//GO(pa_cvolume_set_fade, 
//GO(pa_cvolume_snprint, 
//GO(pa_cvolume_valid, 
//GO(pa_ext_stream_restore_delete, 
//GO(pa_ext_stream_restore_read, 
//GO(pa_ext_stream_restore_set_subscribe_cb, 
//GO(pa_ext_stream_restore_subscribe, 
//GO(pa_ext_stream_restore_test, 
//GO(pa_ext_stream_restore_write, 
//GO(pa_frame_size, 
//GO(pa_get_binary_name, 
//GO(pa_get_fqdn, 
//GO(pa_get_home_dir, 
//GO(pa_get_host_name, 
//GO(pa_get_library_version, 
//GO(pa_gettimeofday, 
//GO(pa_get_user_name, 
//GO(pa_locale_to_utf8, 
//GO(pa_mainloop_api_once, 
GO(pa_mainloop_dispatch, iFp)
GO(pa_mainloop_free, vFp)
GO(pa_mainloop_get_api, pFp)
GO(pa_mainloop_get_retval, iFp)
GO(pa_mainloop_iterate, iFpip)
GO(pa_mainloop_new, pFv)
GO(pa_mainloop_poll, iFp)
GO(pa_mainloop_prepare, iFpi)
GO(pa_mainloop_quit, vFpi)
GO(pa_mainloop_run, iFpp)
//GO(pa_mainloop_set_poll_func, vFpBp)  //B is typedef int(*pa_poll_func) (struct pollfd *ufds, unsigned long nfds, int timeout, void *userdata)
GO(pa_mainloop_wakeup, vFp)
//GO(pa_msleep, 
GO(pa_operation_cancel, vFp)
GO(pa_operation_get_state, iFp)
GO(pa_operation_ref, pFp)
GO(pa_operation_unref, vFp)
//GO(pa_parse_sample_format, 
//GO(pa_path_get_filename, 
//GO(pa_proplist_clear, 
//GO(pa_proplist_contains, 
//GO(pa_proplist_copy, 
//GO(pa_proplist_free, 
//GO(pa_proplist_from_string, 
//GO(pa_proplist_get, 
GO(pa_proplist_gets, pFpp)
//GO(pa_proplist_isempty, 
//GO(pa_proplist_iterate, 
//GO(pa_proplist_new, 
//GO(pa_proplist_set, 
//GO(pa_proplist_setf, 
//GO(pa_proplist_sets, 
//GO(pa_proplist_size, 
//GO(pa_proplist_to_string, 
//GO(pa_proplist_to_string_sep, 
//GO(pa_proplist_unset, 
//GO(pa_proplist_unset_many, 
//GO(pa_proplist_update, 
//GO(pa_sample_format_to_string, 
//GO(pa_sample_size, 
//GO(pa_sample_size_of_format, 
//GO(pa_sample_spec_equal, 
//GO(pa_sample_spec_init, 
GO(pa_sample_spec_snprint, pFpup)
//GO(pa_sample_spec_valid, 
//GO(pa_signal_done, 
//GO(pa_signal_free, 
//GO(pa_signal_init, 
//GO(pa_signal_new, 
//GO(pa_signal_set_destroy, 
//GO(pa_stream_connect_playback, iFpppipp)
GO(pa_stream_connect_record, iFpppi)
//GO(pa_stream_connect_upload, 
//GO(pa_stream_cork, 
//GO(pa_stream_disconnect, 
GOM(pa_stream_drain, pFEppp)
GO(pa_stream_drop, iFp)
//GO(pa_stream_finish_upload, 
GOM(pa_stream_flush, pFEppp)
//GO(pa_stream_get_buffer_attr, 
GO(pa_stream_get_channel_map, pFp)
GO(pa_stream_get_context, pFp)
GO(pa_stream_get_device_index, uFp)
GO(pa_stream_get_device_name, pFp)
//GO(pa_stream_get_index, 
GO(pa_stream_get_latency, iFppp)
//GO(pa_stream_get_monitor_stream, 
GO(pa_stream_get_sample_spec, pFp)
GO(pa_stream_get_state, iFp)
//GO(pa_stream_get_time, 
//GO(pa_stream_get_timing_info, 
//GO(pa_stream_is_corked, 
GO(pa_stream_is_suspended, iFp)
GO(pa_stream_new, pFpppp)
//GO(pa_stream_new_with_proplist, 
GO(pa_stream_peek, iFppp)
//GO(pa_stream_prebuf, 
//GO(pa_stream_proplist_remove, 
//GO(pa_stream_proplist_update, 
GO(pa_stream_readable_size, uFp)
GO(pa_stream_ref, pFp)
//GO(pa_stream_set_buffer_attr, 
//GO(pa_stream_set_buffer_attr_callback, 
//GO(pa_stream_set_event_callback, 
GOM(pa_stream_set_latency_update_callback, vFEppp)
//GO(pa_stream_set_monitor_stream, 
//GO(pa_stream_set_moved_callback, 
//GO(pa_stream_set_name, 
//GO(pa_stream_set_overflow_callback, 
GOM(pa_stream_set_read_callback, vFEppp)
//GO(pa_stream_set_started_callback, 
GOM(pa_stream_set_state_callback, vFEppp)
//GO(pa_stream_set_suspended_callback, 
//GO(pa_stream_set_underflow_callback, 
//GOM(pa_stream_set_write_callback, vFEppp)
//GO(pa_stream_trigger, 
GO(pa_stream_unref, vFp)
//GO(pa_stream_update_sample_rate, 
//GO(pa_stream_update_timing_info, 
GO(pa_stream_writable_size, uFp)
//GO(pa_stream_write, 
GO(pa_strerror, pFi)
//GO(pa_sw_cvolume_divide, 
//GO(pa_sw_cvolume_multiply, 
//GO(pa_sw_cvolume_snprint_dB, 
//GO(pa_sw_volume_divide, 
//GO(pa_sw_volume_from_dB, 
//GO(pa_sw_volume_from_linear, 
//GO(pa_sw_volume_multiply, 
//GO(pa_sw_volume_snprint_dB, 
//GO(pa_sw_volume_to_dB, 
//GO(pa_sw_volume_to_linear, 
//GO(pa_threaded_mainloop_accept, 
//GO(pa_threaded_mainloop_free, 
//GO(pa_threaded_mainloop_get_api, 
//GO(pa_threaded_mainloop_get_retval, 
//GO(pa_threaded_mainloop_in_thread, 
//GO(pa_threaded_mainloop_lock, 
//GO(pa_threaded_mainloop_new, 
//GO(pa_threaded_mainloop_signal, 
//GO(pa_threaded_mainloop_start, 
//GO(pa_threaded_mainloop_stop, 
//GO(pa_threaded_mainloop_unlock, 
//GO(pa_threaded_mainloop_wait, 
//GO(pa_timeval_add, 
//GO(pa_timeval_age, 
//GO(pa_timeval_cmp, 
//GO(pa_timeval_diff, 
//GO(pa_timeval_load, 
//GO(pa_timeval_store, 
//GO(pa_timeval_sub, 
GO(pa_usec_to_bytes, uFUp)
//GO(pa_utf8_filter, 
//GO(pa_utf8_to_locale, 
//GO(pa_utf8_valid, 
//GO(pa_volume_snprint, 
//GO(pa_xfree, 
//GO(pa_xmalloc, 
//GO(pa_xmalloc0, 
//GO(pa_xmemdup, 
//GO(pa_xrealloc, 
//GO(pa_xstrdup, 
//GO(pa_xstrndup, 
