#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(gst_adapter_available, 
//GO(gst_adapter_available_fast, 
//GO(gst_adapter_clear, 
//GO(gst_adapter_copy, 
//GO(gst_adapter_copy_bytes, 
//GO(gst_adapter_distance_from_discont, 
//GO(gst_adapter_dts_at_discont, 
//GO(gst_adapter_flush, 
//GO(gst_adapter_get_buffer, 
//GO(gst_adapter_get_buffer_fast, 
//GO(gst_adapter_get_buffer_list, 
//GO(gst_adapter_get_list, 
//GO(gst_adapter_get_type, 
//GO(gst_adapter_map, 
//GO(gst_adapter_masked_scan_uint32, 
//GO(gst_adapter_masked_scan_uint32_peek, 
//GO(gst_adapter_new, 
//GO(gst_adapter_offset_at_discont, 
//GO(gst_adapter_prev_dts, 
//GO(gst_adapter_prev_dts_at_offset, 
//GO(gst_adapter_prev_offset, 
//GO(gst_adapter_prev_pts, 
//GO(gst_adapter_prev_pts_at_offset, 
//GO(gst_adapter_pts_at_discont, 
//GO(gst_adapter_push, 
//GO(gst_adapter_take, 
//GO(gst_adapter_take_buffer, 
//GO(gst_adapter_take_buffer_fast, 
//GO(gst_adapter_take_buffer_list, 
//GO(gst_adapter_take_list, 
//GO(gst_adapter_unmap, 
//GO(gst_aggregator_finish_buffer, 
//GO(gst_aggregator_finish_buffer_list, 
//GO(gst_aggregator_get_allocator, 
//GO(gst_aggregator_get_buffer_pool, 
//GO(gst_aggregator_get_latency, 
//GO(gst_aggregator_get_type, 
//GO(gst_aggregator_negotiate, 
//GO(gst_aggregator_pad_drop_buffer, 
//GO(gst_aggregator_pad_get_type, 
//GO(gst_aggregator_pad_has_buffer, 
//GO(gst_aggregator_pad_is_eos, 
//GO(gst_aggregator_pad_peek_buffer, 
//GO(gst_aggregator_pad_pop_buffer, 
//GO(gst_aggregator_peek_next_sample, 
//GO(gst_aggregator_selected_samples, 
//GO(gst_aggregator_set_latency, 
//GO(gst_aggregator_set_src_caps, 
//GO(gst_aggregator_simple_get_next_time, 
//GO(gst_aggregator_start_time_selection_get_type, 
//GO(gst_aggregator_update_segment, 
//GO(gst_base_parse_add_index_entry, 
//GO(gst_base_parse_convert_default, 
//GO(gst_base_parse_drain, 
//GO(gst_base_parse_finish_frame, 
//GO(gst_base_parse_frame_copy, 
//GO(gst_base_parse_frame_free, 
//GO(gst_base_parse_frame_get_type, 
//GO(gst_base_parse_frame_init, 
//GO(gst_base_parse_frame_new, 
//GO(gst_base_parse_get_type, 
//GO(gst_base_parse_merge_tags, 
//GO(gst_base_parse_push_frame, 
//GO(gst_base_parse_set_average_bitrate, 
//GO(gst_base_parse_set_duration, 
//GO(gst_base_parse_set_frame_rate, 
//GO(gst_base_parse_set_has_timing_info, 
//GO(gst_base_parse_set_infer_ts, 
//GO(gst_base_parse_set_latency, 
//GO(gst_base_parse_set_min_frame_size, 
//GO(gst_base_parse_set_passthrough, 
//GO(gst_base_parse_set_pts_interpolation, 
//GO(gst_base_parse_set_syncable, 
//GO(gst_base_parse_set_ts_at_offset, 
//GO(gst_base_sink_do_preroll, 
//GO(gst_base_sink_get_blocksize, 
//GO(gst_base_sink_get_drop_out_of_segment, 
//GO(gst_base_sink_get_last_sample, 
//GO(gst_base_sink_get_latency, 
//GO(gst_base_sink_get_max_bitrate, 
//GO(gst_base_sink_get_max_lateness, 
//GO(gst_base_sink_get_processing_deadline, 
//GO(gst_base_sink_get_render_delay, 
//GO(gst_base_sink_get_stats, 
//GO(gst_base_sink_get_sync, 
//GO(gst_base_sink_get_throttle_time, 
//GO(gst_base_sink_get_ts_offset, 
//GO(gst_base_sink_get_type, 
//GO(gst_base_sink_is_async_enabled, 
//GO(gst_base_sink_is_last_sample_enabled, 
//GO(gst_base_sink_is_qos_enabled, 
//GO(gst_base_sink_query_latency, 
//GO(gst_base_sink_set_async_enabled, 
//GO(gst_base_sink_set_blocksize, 
//GO(gst_base_sink_set_drop_out_of_segment, 
//GO(gst_base_sink_set_last_sample_enabled, 
//GO(gst_base_sink_set_max_bitrate, 
//GO(gst_base_sink_set_max_lateness, 
//GO(gst_base_sink_set_processing_deadline, 
//GO(gst_base_sink_set_qos_enabled, 
//GO(gst_base_sink_set_render_delay, 
//GO(gst_base_sink_set_sync, 
//GO(gst_base_sink_set_throttle_time, 
//GO(gst_base_sink_set_ts_offset, 
//GO(gst_base_sink_wait, 
//GO(gst_base_sink_wait_clock, 
//GO(gst_base_sink_wait_preroll, 
//GO(gst_base_src_get_allocator, 
//GO(gst_base_src_get_blocksize, 
//GO(gst_base_src_get_buffer_pool, 
//GO(gst_base_src_get_do_timestamp, 
//GO(gst_base_src_get_type, 
//GO(gst_base_src_is_async, 
//GO(gst_base_src_is_live, 
//GO(gst_base_src_negotiate, 
//GO(gst_base_src_new_seamless_segment, 
//GO(gst_base_src_new_segment, 
//GO(gst_base_src_query_latency, 
//GO(gst_base_src_set_async, 
//GO(gst_base_src_set_automatic_eos, 
//GO(gst_base_src_set_blocksize, 
//GO(gst_base_src_set_caps, 
//GO(gst_base_src_set_do_timestamp, 
//GO(gst_base_src_set_dynamic_size, 
//GO(gst_base_src_set_format, 
//GO(gst_base_src_set_live, 
//GO(gst_base_src_start_complete, 
//GO(gst_base_src_start_wait, 
//GO(gst_base_src_submit_buffer_list, 
//GO(gst_base_src_wait_playing, 
//GO(gst_base_transform_get_allocator, 
//GO(gst_base_transform_get_buffer_pool, 
//GO(gst_base_transform_get_type, 
//GO(gst_base_transform_is_in_place, 
//GO(gst_base_transform_is_passthrough, 
//GO(gst_base_transform_is_qos_enabled, 
//GO(gst_base_transform_reconfigure, 
//GO(gst_base_transform_reconfigure_sink, 
//GO(gst_base_transform_reconfigure_src, 
//GO(gst_base_transform_set_gap_aware, 
//GO(gst_base_transform_set_in_place, 
//GO(gst_base_transform_set_passthrough, 
//GO(gst_base_transform_set_prefer_passthrough, 
//GO(gst_base_transform_set_qos_enabled, 
//GO(gst_base_transform_update_qos, 
//GO(gst_base_transform_update_src_caps, 
//GO(gst_bit_reader_free, 
//GO(gst_bit_reader_get_bits_uint16, 
//GO(gst_bit_reader_get_bits_uint32, 
//GO(gst_bit_reader_get_bits_uint64, 
//GO(gst_bit_reader_get_bits_uint8, 
//GO(gst_bit_reader_get_pos, 
//GO(gst_bit_reader_get_remaining, 
//GO(gst_bit_reader_get_size, 
//GO(gst_bit_reader_init, 
//GO(gst_bit_reader_new, 
//GO(gst_bit_reader_peek_bits_uint16, 
//GO(gst_bit_reader_peek_bits_uint32, 
//GO(gst_bit_reader_peek_bits_uint64, 
//GO(gst_bit_reader_peek_bits_uint8, 
//GO(gst_bit_reader_set_pos, 
//GO(gst_bit_reader_skip, 
//GO(gst_bit_reader_skip_to_byte, 
//GO(gst_bit_writer_align_bytes, 
//GO(gst_bit_writer_free, 
//GO(gst_bit_writer_free_and_get_buffer, 
//GO(gst_bit_writer_free_and_get_data, 
//GO(gst_bit_writer_get_data, 
//GO(gst_bit_writer_get_size, 
//GO(gst_bit_writer_init, 
//GO(gst_bit_writer_init_with_data, 
//GO(gst_bit_writer_init_with_size, 
//GO(gst_bit_writer_new, 
//GO(gst_bit_writer_new_with_data, 
//GO(gst_bit_writer_new_with_size, 
//GO(gst_bit_writer_put_bits_uint16, 
//GO(gst_bit_writer_put_bits_uint32, 
//GO(gst_bit_writer_put_bits_uint64, 
//GO(gst_bit_writer_put_bits_uint8, 
//GO(gst_bit_writer_put_bytes, 
//GO(gst_bit_writer_reset, 
//GO(gst_bit_writer_reset_and_get_buffer, 
//GO(gst_bit_writer_reset_and_get_data, 
//GO(gst_bit_writer_set_pos, 
//GO(gst_byte_reader_dup_data, 
//GO(gst_byte_reader_dup_string_utf16, 
//GO(gst_byte_reader_dup_string_utf32, 
//GO(gst_byte_reader_dup_string_utf8, 
//GO(gst_byte_reader_free, 
//GO(gst_byte_reader_get_data, 
//GO(gst_byte_reader_get_float32_be, 
//GO(gst_byte_reader_get_float32_le, 
//GO(gst_byte_reader_get_float64_be, 
//GO(gst_byte_reader_get_float64_le, 
//GO(gst_byte_reader_get_int16_be, 
//GO(gst_byte_reader_get_int16_le, 
//GO(gst_byte_reader_get_int24_be, 
//GO(gst_byte_reader_get_int24_le, 
//GO(gst_byte_reader_get_int32_be, 
//GO(gst_byte_reader_get_int32_le, 
//GO(gst_byte_reader_get_int64_be, 
//GO(gst_byte_reader_get_int64_le, 
//GO(gst_byte_reader_get_int8, 
//GO(gst_byte_reader_get_pos, 
//GO(gst_byte_reader_get_remaining, 
//GO(gst_byte_reader_get_size, 
//GO(gst_byte_reader_get_string_utf8, 
//GO(gst_byte_reader_get_sub_reader, 
//GO(gst_byte_reader_get_uint16_be, 
//GO(gst_byte_reader_get_uint16_le, 
//GO(gst_byte_reader_get_uint24_be, 
//GO(gst_byte_reader_get_uint24_le, 
//GO(gst_byte_reader_get_uint32_be, 
//GO(gst_byte_reader_get_uint32_le, 
//GO(gst_byte_reader_get_uint64_be, 
//GO(gst_byte_reader_get_uint64_le, 
//GO(gst_byte_reader_get_uint8, 
//GO(gst_byte_reader_init, 
//GO(gst_byte_reader_masked_scan_uint32, 
//GO(gst_byte_reader_masked_scan_uint32_peek, 
//GO(gst_byte_reader_new, 
//GO(gst_byte_reader_peek_data, 
//GO(gst_byte_reader_peek_float32_be, 
//GO(gst_byte_reader_peek_float32_le, 
//GO(gst_byte_reader_peek_float64_be, 
//GO(gst_byte_reader_peek_float64_le, 
//GO(gst_byte_reader_peek_int16_be, 
//GO(gst_byte_reader_peek_int16_le, 
//GO(gst_byte_reader_peek_int24_be, 
//GO(gst_byte_reader_peek_int24_le, 
//GO(gst_byte_reader_peek_int32_be, 
//GO(gst_byte_reader_peek_int32_le, 
//GO(gst_byte_reader_peek_int64_be, 
//GO(gst_byte_reader_peek_int64_le, 
//GO(gst_byte_reader_peek_int8, 
//GO(gst_byte_reader_peek_string_utf8, 
//GO(gst_byte_reader_peek_sub_reader, 
//GO(gst_byte_reader_peek_uint16_be, 
//GO(gst_byte_reader_peek_uint16_le, 
//GO(gst_byte_reader_peek_uint24_be, 
//GO(gst_byte_reader_peek_uint24_le, 
//GO(gst_byte_reader_peek_uint32_be, 
//GO(gst_byte_reader_peek_uint32_le, 
//GO(gst_byte_reader_peek_uint64_be, 
//GO(gst_byte_reader_peek_uint64_le, 
//GO(gst_byte_reader_peek_uint8, 
//GO(gst_byte_reader_set_pos, 
//GO(gst_byte_reader_skip, 
//GO(gst_byte_reader_skip_string_utf16, 
//GO(gst_byte_reader_skip_string_utf32, 
//GO(gst_byte_reader_skip_string_utf8, 
//GO(gst_byte_writer_ensure_free_space, 
//GO(gst_byte_writer_fill, 
//GO(gst_byte_writer_free, 
//GO(gst_byte_writer_free_and_get_buffer, 
//GO(gst_byte_writer_free_and_get_data, 
//GO(gst_byte_writer_get_remaining, 
//GO(gst_byte_writer_init, 
//GO(gst_byte_writer_init_with_data, 
//GO(gst_byte_writer_init_with_size, 
//GO(gst_byte_writer_new, 
//GO(gst_byte_writer_new_with_data, 
//GO(gst_byte_writer_new_with_size, 
//GO(gst_byte_writer_put_data, 
//GO(gst_byte_writer_put_float32_be, 
//GO(gst_byte_writer_put_float32_le, 
//GO(gst_byte_writer_put_float64_be, 
//GO(gst_byte_writer_put_float64_le, 
//GO(gst_byte_writer_put_int16_be, 
//GO(gst_byte_writer_put_int16_le, 
//GO(gst_byte_writer_put_int24_be, 
//GO(gst_byte_writer_put_int24_le, 
//GO(gst_byte_writer_put_int32_be, 
//GO(gst_byte_writer_put_int32_le, 
//GO(gst_byte_writer_put_int64_be, 
//GO(gst_byte_writer_put_int64_le, 
//GO(gst_byte_writer_put_int8, 
//GO(gst_byte_writer_put_string_utf16, 
//GO(gst_byte_writer_put_string_utf32, 
//GO(gst_byte_writer_put_string_utf8, 
//GO(gst_byte_writer_put_uint16_be, 
//GO(gst_byte_writer_put_uint16_le, 
//GO(gst_byte_writer_put_uint24_be, 
//GO(gst_byte_writer_put_uint24_le, 
//GO(gst_byte_writer_put_uint32_be, 
//GO(gst_byte_writer_put_uint32_le, 
//GO(gst_byte_writer_put_uint64_be, 
//GO(gst_byte_writer_put_uint64_le, 
//GO(gst_byte_writer_put_uint8, 
//GO(gst_byte_writer_reset, 
//GO(gst_byte_writer_reset_and_get_buffer, 
//GO(gst_byte_writer_reset_and_get_data, 
//GO(gst_collect_pads_add_pad, 
//GO(gst_collect_pads_available, 
//GO(gst_collect_pads_clip_running_time, 
//GO(gst_collect_pads_event_default, 
//GO(gst_collect_pads_flush, 
//GO(gst_collect_pads_get_type, 
//GO(gst_collect_pads_new, 
//GO(gst_collect_pads_peek, 
//GO(gst_collect_pads_pop, 
//GO(gst_collect_pads_query_default, 
//GO(gst_collect_pads_read_buffer, 
//GO(gst_collect_pads_remove_pad, 
//GO(gst_collect_pads_set_buffer_function, 
//GO(gst_collect_pads_set_clip_function, 
//GO(gst_collect_pads_set_compare_function, 
//GO(gst_collect_pads_set_event_function, 
//GO(gst_collect_pads_set_flush_function, 
//GO(gst_collect_pads_set_flushing, 
//GO(gst_collect_pads_set_function, 
//GO(gst_collect_pads_set_query_function, 
//GO(gst_collect_pads_set_waiting, 
//GO(gst_collect_pads_src_event_default, 
//GO(gst_collect_pads_start, 
//GO(gst_collect_pads_stop, 
//GO(gst_collect_pads_take_buffer, 
//GO(gst_data_queue_drop_head, 
//GO(gst_data_queue_flush, 
//GO(gst_data_queue_get_level, 
//GO(gst_data_queue_get_type, 
//GO(gst_data_queue_is_empty, 
//GO(gst_data_queue_is_full, 
//GO(gst_data_queue_limits_changed, 
//GO(gst_data_queue_new, 
//GO(gst_data_queue_peek, 
//GO(gst_data_queue_pop, 
//GO(gst_data_queue_push, 
//GO(gst_data_queue_push_force, 
//GO(gst_data_queue_set_flushing, 
//GO(gst_flow_combiner_add_pad, 
//GO(gst_flow_combiner_clear, 
//GO(gst_flow_combiner_free, 
//GO(gst_flow_combiner_get_type, 
//GO(gst_flow_combiner_new, 
//GO(gst_flow_combiner_ref, 
//GO(gst_flow_combiner_remove_pad, 
//GO(gst_flow_combiner_reset, 
//GO(gst_flow_combiner_unref, 
//GO(gst_flow_combiner_update_flow, 
//GO(gst_flow_combiner_update_pad_flow, 
//GO(gst_push_src_get_type, 
//GO(gst_queue_array_clear, 
//GO(gst_queue_array_drop_element, 
//GO(gst_queue_array_drop_struct, 
//GO(gst_queue_array_find, 
//GO(gst_queue_array_free, 
//GO(gst_queue_array_get_length, 
//GO(gst_queue_array_is_empty, 
//GO(gst_queue_array_new, 
//GO(gst_queue_array_new_for_struct, 
//GO(gst_queue_array_peek_head, 
//GO(gst_queue_array_peek_head_struct, 
//GO(gst_queue_array_peek_nth, 
//GO(gst_queue_array_peek_nth_struct, 
//GO(gst_queue_array_peek_tail, 
//GO(gst_queue_array_peek_tail_struct, 
//GO(gst_queue_array_pop_head, 
//GO(gst_queue_array_pop_head_struct, 
//GO(gst_queue_array_pop_tail, 
//GO(gst_queue_array_pop_tail_struct, 
//GO(gst_queue_array_push_tail, 
//GO(gst_queue_array_push_tail_struct, 
//GO(gst_queue_array_set_clear_func, 
//GO(gst_type_find_helper, 
//GO(gst_type_find_helper_for_buffer, 
//GO(gst_type_find_helper_for_buffer_with_extension, 
//GO(gst_type_find_helper_for_data, 
GO(gst_type_find_helper_for_data_with_extension, pFppLpp)
//GO(gst_type_find_helper_for_extension, 
//GO(gst_type_find_helper_get_range, 
//GO(gst_type_find_helper_get_range_full, 
