#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(__aeabi_f2lz, 
//GO(__aeabi_f2ulz, 
//GO(_fini, 
//GO(g_array_get_type, 
//GO(g_binding_flags_get_type, 
//GO(g_binding_get_flags, 
//GO(g_binding_get_source, 
//GO(g_binding_get_source_property, 
//GO(g_binding_get_target, 
//GO(g_binding_get_target_property, 
//GO(g_binding_get_type, 
//GO(g_binding_unbind, 
//GO(g_boxed_copy, 
//GO(g_boxed_free, 
GOM(g_boxed_type_register_static, iFEppp)
//GO(g_byte_array_get_type, 
//GO(g_bytes_get_type, 
//GO(g_cclosure_marshal_BOOLEAN__BOXED_BOXED, 
//GO(g_cclosure_marshal_BOOLEAN__BOXED_BOXEDv, 
//GO(g_cclosure_marshal_BOOLEAN__FLAGS, 
//GO(g_cclosure_marshal_BOOLEAN__FLAGSv, 
//GO(g_cclosure_marshal_generic, 
//GO(g_cclosure_marshal_generic_va, 
//GO(g_cclosure_marshal_STRING__OBJECT_POINTER, 
//GO(g_cclosure_marshal_STRING__OBJECT_POINTERv, 
//GO(g_cclosure_marshal_VOID__BOOLEAN, 
//GO(g_cclosure_marshal_VOID__BOOLEANv, 
//GO(g_cclosure_marshal_VOID__BOXED, 
//GO(g_cclosure_marshal_VOID__BOXEDv, 
//GO(g_cclosure_marshal_VOID__CHAR, 
//GO(g_cclosure_marshal_VOID__CHARv, 
//GO(g_cclosure_marshal_VOID__DOUBLE, 
//GO(g_cclosure_marshal_VOID__DOUBLEv, 
//GO(g_cclosure_marshal_VOID__ENUM, 
//GO(g_cclosure_marshal_VOID__ENUMv, 
//GO(g_cclosure_marshal_VOID__FLAGS, 
//GO(g_cclosure_marshal_VOID__FLAGSv, 
//GO(g_cclosure_marshal_VOID__FLOAT, 
//GO(g_cclosure_marshal_VOID__FLOATv, 
//GO(g_cclosure_marshal_VOID__INT, 
//GO(g_cclosure_marshal_VOID__INTv, 
//GO(g_cclosure_marshal_VOID__LONG, 
//GO(g_cclosure_marshal_VOID__LONGv, 
//GO(g_cclosure_marshal_VOID__OBJECT, 
//GO(g_cclosure_marshal_VOID__OBJECTv, 
//GO(g_cclosure_marshal_VOID__PARAM, 
//GO(g_cclosure_marshal_VOID__PARAMv, 
//GO(g_cclosure_marshal_VOID__POINTER, 
//GO(g_cclosure_marshal_VOID__POINTERv, 
GO(g_cclosure_marshal_VOID__STRING, vFppuppp)
//GO(g_cclosure_marshal_VOID__STRINGv, 
//GO(g_cclosure_marshal_VOID__UCHAR, 
//GO(g_cclosure_marshal_VOID__UCHARv, 
//GO(g_cclosure_marshal_VOID__UINT, 
//GO(g_cclosure_marshal_VOID__UINT_POINTER, 
//GO(g_cclosure_marshal_VOID__UINT_POINTERv, 
//GO(g_cclosure_marshal_VOID__UINTv, 
//GO(g_cclosure_marshal_VOID__ULONG, 
//GO(g_cclosure_marshal_VOID__ULONGv, 
//GO(g_cclosure_marshal_VOID__VARIANT, 
//GO(g_cclosure_marshal_VOID__VARIANTv, 
GO(g_cclosure_marshal_VOID__VOID, vFppuppp)
//GO(g_cclosure_marshal_VOID__VOIDv, 
//GO(g_cclosure_new, 
//GO(g_cclosure_new_object, 
//GO(g_cclosure_new_object_swap, 
//GO(g_cclosure_new_swap, 
//GO(g_checksum_get_type, 
GO(g_clear_object, vFp)
//GO(g_closure_add_finalize_notifier, 
//GO(g_closure_add_invalidate_notifier, 
//GO(g_closure_add_marshal_guards, 
//GO(g_closure_get_type, 
//GO(g_closure_invalidate, 
//GO(g_closure_invoke, 
//GO(g_closure_new_object, 
//GO(g_closure_new_simple, 
//GO(g_closure_ref, 
//GO(g_closure_remove_finalize_notifier, 
//GO(g_closure_remove_invalidate_notifier, 
//GO(g_closure_set_marshal, 
//GO(g_closure_set_meta_marshal, 
//GO(g_closure_sink, 
//GO(g_closure_unref, 
//GO(g_date_get_type, 
//GO(g_date_time_get_type, 
//GO(g_enum_complete_type_info, 
//GO(g_enum_get_value, 
//GO(g_enum_get_value_by_name, 
//GO(g_enum_get_value_by_nick, 
//GO(g_enum_register_static, 
//GO(g_error_get_type, 
//GO(g_flags_complete_type_info, 
//GO(g_flags_get_first_value, 
//GO(g_flags_get_value_by_name, 
//GO(g_flags_get_value_by_nick, 
//GO(g_flags_register_static, 
//GO(g_gstring_get_type, 
//GO(g_gtype_get_type, 
//GO(g_hash_table_get_type, 
//GO(g_initially_unowned_get_type, 
//GO(g_io_channel_get_type, 
//GO(g_io_condition_get_type, 
//GO(g_key_file_get_type, 
//GO(g_main_context_get_type, 
//GO(g_main_loop_get_type, 
//GO(g_mapped_file_get_type, 
//GO(g_markup_parse_context_get_type, 
//GO(g_match_info_get_type, 
//GOM(g_object_add_toggle_ref, vFEpBp)
GO(g_object_add_weak_pointer, vFpp)
//GO(g_object_bind_property, 
//GO(g_object_bind_property_full, 
//GO(g_object_bind_property_with_closures, 
GO(g_object_class_find_property, pFpp)
GO(g_object_class_install_properties, vFpup)
GO(g_object_class_install_property, vFpup)
GO(g_object_class_list_properties, pFpp)
GO(g_object_class_override_property, vFpup)
//GO(g_object_compat_control, 
GO(g_object_connect, pFpppppppppppppppp)    //vaarg
GO(g_object_disconnect, vFpppppppppppppppp)   // caarg
//GOM(g_object_dup_data, pFEppBp)
//GOM(g_object_dup_qdata, pFEppBp)
GO(g_object_force_floating, vFp)
GO(g_object_freeze_notify, vFp)
GO2(g_object_get, vFppV, g_object_get_valist)
GO(g_object_get_data, pFpp)
GO(g_object_get_property, vFppp)
GO(g_object_get_qdata, pFpp)
GO(g_object_get_type, iFv)
GO(g_object_get_valist, vFppp)
GO(g_object_interface_find_property, pFpp)
GO(g_object_interface_install_property, vFpp)
GO(g_object_interface_list_properties, pFpp)
GO(g_object_is_floating, iFp)
GOM(g_object_new, pFEipV)
GO(g_object_newv, pFiup)
GO(g_object_new_valist, pFipp)
GO(g_object_notify, vFpp)
GO(g_object_notify_by_pspec, vFpp)
GO(g_object_ref, pFp)
GO(g_object_ref_sink, pFp)
//GOM(g_object_remove_toggle_ref, vFEpBp)
GO(g_object_remove_weak_pointer, vFpp)
//GOM(g_object_replace_data, iFEppppBB)
//GOM(g_object_replace_qdata, iFEppppBB)
GO(g_object_run_dispose, vFp)
GO2(g_object_set, vFppV, g_object_set_valist)
GO(g_object_set_data, vFppp)
//GOM(g_object_set_data_full, vFEpppB)
GO(g_object_set_property, vFppp)
GO(g_object_set_qdata, vFppp)
//GOM(g_object_set_qdata_full, vFEpppB)
GO(g_object_set_valist, vFppp)
GO(g_object_steal_data, pFpp)
GO(g_object_steal_qdata, pFpp)
GO(g_object_thaw_notify, vFp)
GO(g_object_unref, vFp)
GO(g_object_watch_closure, vFpp)
//GOM(g_object_weak_ref, vFpBp)
GO(g_object_weak_unref, vFpp)
//GO(g_param_spec_boolean, 
//GO(g_param_spec_boxed, 
//GO(g_param_spec_char, 
//GO(g_param_spec_double, 
//GO(g_param_spec_enum, 
//GO(g_param_spec_flags, 
//GO(g_param_spec_float, 
//GO(g_param_spec_get_blurb, 
//GO(g_param_spec_get_default_value, 
//GO(g_param_spec_get_name, 
//GO(g_param_spec_get_nick, 
//GO(g_param_spec_get_qdata, 
//GO(g_param_spec_get_redirect_target, 
GO(g_param_spec_gtype, pFpppii)
//GO(g_param_spec_int, 
//GO(g_param_spec_int64, 
//GO(g_param_spec_internal, 
//GO(g_param_spec_long, 
//GO(g_param_spec_object, 
GO(g_param_spec_override, pFpp)
//GO(g_param_spec_param, 
//GO(g_param_spec_pointer, 
//GO(g_param_spec_pool_insert, 
//GO(g_param_spec_pool_list, 
//GO(g_param_spec_pool_list_owned, 
//GO(g_param_spec_pool_lookup, 
//GO(g_param_spec_pool_new, 
//GO(g_param_spec_pool_remove, 
//GO(g_param_spec_ref, 
//GO(g_param_spec_ref_sink, 
//GO(g_param_spec_set_qdata, 
//GO(g_param_spec_set_qdata_full, 
//GO(g_param_spec_sink, 
//GO(g_param_spec_steal_qdata, 
//GO(g_param_spec_string, 
//GO(g_param_spec_uchar, 
//GO(g_param_spec_uint, 
//GO(g_param_spec_uint64, 
//GO(g_param_spec_ulong, 
GO(g_param_spec_unichar, pFpppui)
//GO(g_param_spec_unref, 
GO(g_param_spec_value_array, pFppppi)
GO(g_param_spec_variant, pFpppppi)
//GO(g_param_type_register_static, 
//GO(g_param_value_convert, 
//GO(g_param_value_defaults, 
//GO(g_param_values_cmp, 
//GO(g_param_value_set_default, 
//GO(g_param_value_validate, 
//GO(g_pointer_type_register_static, 
//GO(g_pollfd_get_type, 
//GO(g_ptr_array_get_type, 
//GO(g_regex_get_type, 
GO(g_signal_accumulator_first_wins, iFpppp)
GO(g_signal_accumulator_true_handled, iFpppp)
//GOM(g_signal_add_emission_hook, LFEupBpB)
GO(g_signal_chain_from_overridden, vFpp)
GO(g_signal_chain_from_overridden_handler, vFpppppppppp)  //vaarg
GO(g_signal_connect_closure, LFpppi)
GO(g_signal_connect_closure_by_id, LFpuppi)
GOM(g_signal_connect_data, LFEpppppu)
//GOM(g_signal_connect_object, LFEppBpi)
GO2(g_signal_emit, vFpuuV, g_signal_emit_valist) // vaarg
GO(g_signal_emit_by_name, vFppppppppppp)    //vaarg
GO(g_signal_emitv, vFpuup)
GO(g_signal_emit_valist, vFppup)    // va_list here
GO(g_signal_get_invocation_hint, pFp)
GO(g_signal_handler_block, vFpL)
GO(g_signal_handler_disconnect, vFpL)
GOM(g_signal_handler_find, LFEpiupppp)
GO(g_signal_handler_is_connected, iFpL)
GOM(g_signal_handlers_block_matched, uFEpiupppp)
//GO(g_signal_handlers_destroy, 
GOM(g_signal_handlers_disconnect_matched, uFEpiupppp)
GOM(g_signal_handlers_unblock_matched, uFEpiupppp)
GO(g_signal_handler_unblock, vFpL)
GO(g_signal_has_handler_pending, iFpupi)
GO(g_signal_list_ids, pFip)
GO(g_signal_lookup, uFpi)
GO(g_signal_name, pFu)
GOM(g_signal_new, uFEpiiupppiuV)
//GOM(g_signal_new_class_handler, uFEpiupppppnV)
GOM(g_signal_newv, uFEpiippppiup)
GOM(g_signal_new_valist, uFEpiippppiup)
GO(g_signal_override_class_closure, vFuip)
//GOM(g_signal_override_class_handler, vFEppB)
GO(g_signal_parse_name, iFpippi)
GO(g_signal_query, vFup)
GO(g_signal_remove_emission_hook, vFpL)
//GOM(g_signal_set_va_marshaller, vFEuiB)
GO(g_signal_stop_emission, vFpup)
GO(g_signal_stop_emission_by_name, vFpp)
GO(g_signal_type_cclosure_new, pFiu)    //should wrap?
GO(g_source_get_type, iFv)
//GO(g_source_set_closure, 
//GO(g_source_set_dummy_callback, 
GO(g_strdup_value_contents, pFp)
//GO(g_strv_get_type, 
//GO(g_thread_get_type, 
//GO(g_time_zone_get_type, 
//GOM(g_type_add_class_cache_func, vFEpB)
GO(g_type_add_class_private, vFpiu)
//GO(g_type_add_instance_private, 
//GOM(g_type_add_interface_check, vFEpB)
//GOM(g_type_add_interface_dynamic, vFEiip)
//GOM(g_type_add_interface_static, vFEuup)
GO(g_type_check_class_cast, pFpi)
GO(g_type_check_class_is_a, iFpi)
GO(g_type_check_instance, iFp)
GO(g_type_check_instance_cast, pFpi)
GO(g_type_check_instance_is_a, iFpi)
GO(g_type_check_instance_is_fundamentally_a, iFpi)
GO(g_type_check_is_value_type, iFpi)
GO(g_type_check_value, iFp)
GO(g_type_check_value_holds, iFpi)
//GO(g_type_children, 
GO(g_type_class_add_private, vFpu)
GO(g_type_class_adjust_private_offset, vFpp)
GO(g_type_class_get_instance_private_offset, iFp)
GO(g_type_class_get_private, pFpi)
GO(g_type_class_peek, pFi)
GO(g_type_class_peek_parent, pFp)
GO(g_type_class_peek_static, pFi)
GO(g_type_class_ref, pFp)
GO(g_type_class_unref, vFp)
GO(g_type_class_unref_uncached, vFp)
//GO(g_type_create_instance, pFi)
//GO(g_type_default_interface_peek, 
//GO(g_type_default_interface_ref, 
//GO(g_type_default_interface_unref, 
//GO(g_type_depth, 
GO(g_type_ensure, vFi)
GO(g_type_free_instance, vFp)
//GO(g_type_from_name, 
GO(g_type_fundamental, iFi)
GO(g_type_fundamental_next, iFv)
//GOM(g_type_get_plugin, pFEi)      // GTypePugin is a stuct with callback
//GO(g_type_get_qdata, 
GO(g_type_get_type_registration_serial, uFv)
//GO(g_type_init, 
//GO(g_type_init_with_debug_flags, 
//GO(g_type_instance_get_private, 
GO(g_type_interface_add_prerequisite, vFii)
//GOM(g_type_interface_get_plugin, pFEii) // return GTypePlugin*
//GO(g_type_interface_peek, 
//GO(g_type_interface_peek_parent, 
GO(g_type_interface_prerequisites, pFip)
//GO(g_type_interfaces, 
//GO(g_type_is_a, 
//GO(g_type_module_add_interface, 
//GO(g_type_module_get_type, 
//GO(g_type_module_register_enum, 
//GO(g_type_module_register_flags, 
//GO(g_type_module_register_type, 
//GO(g_type_module_set_name, 
//GO(g_type_module_unuse, 
//GO(g_type_module_use, 
//GO(g_type_name, 
//GO(g_type_name_from_class, 
//GO(g_type_name_from_instance, 
//GO(g_type_next_base, 
//GO(g_type_parent, 
//GO(g_type_plugin_complete_interface_info, 
//GO(g_type_plugin_complete_type_info, 
//GO(g_type_plugin_get_type, 
//GO(g_type_plugin_unuse, 
//GO(g_type_plugin_use, 
//GO(g_type_qname, 
//GO(g_type_query, 
//GOM(type_register_dynamic, iFEpippi)
GOM(g_type_register_fundamental, iFEipppi)
GOM(g_type_register_static, iFEippi)
//GOM(g_type_register_static_simple, iFEipuBuBi)
//GOM(g_type_remove_class_cache_func, vFEpB)
//GOM(g_type_remove_interface_check, vFEpB)
//GO(g_type_set_qdata, 
//GO(g_type_test_flags, 
//GOM(g_type_value_table_peek, pFEi)    //need to bridge GTypeValueTable
//GO(g_value_array_append, 
//GO(g_value_array_copy, 
//GO(g_value_array_free, 
//GO(g_value_array_get_nth, 
//GO(g_value_array_get_type, 
//GO(g_value_array_insert, 
//GO(g_value_array_new, 
//GO(g_value_array_prepend, 
//GO(g_value_array_remove, 
//GO(g_value_array_sort, 
//GO(g_value_array_sort_with_data, 
GO(g_value_copy, vFpp)
//GO(g_value_dup_boxed, 
GO(g_value_dup_object, pFp)
//GO(g_value_dup_param, 
//GO(g_value_dup_string, 
//GO(g_value_dup_variant, 
GO(g_value_fits_pointer, iFp)
//GO(g_value_get_boolean, 
//GO(g_value_get_boxed, 
//GO(g_value_get_char, 
//GO(g_value_get_double, 
//GO(g_value_get_enum, 
//GO(g_value_get_flags, 
GO(g_value_get_float, fFp)
//GO(g_value_get_gtype, 
//GO(g_value_get_int, 
//GO(g_value_get_int64, 
//GO(g_value_get_long, 
GO(g_value_get_object, pFp)
//GO(g_value_get_param, 
//GO(g_value_get_pointer, 
//GO(g_value_get_schar, 
//GO(g_value_get_string, 
//GO(g_value_get_type, 
//GO(g_value_get_uchar, 
//GO(g_value_get_uint, 
//GO(g_value_get_uint64, 
//GO(g_value_get_ulong, 
//GO(g_value_get_variant, 
GO(g_value_init, pFpi)
GO(g_value_init_from_instance, vFpp)
GO(g_value_peek_pointer, pFp)
//GOM(g_value_register_transform_func, vFEppp)
GO(g_value_reset, pFp)
//GO(g_value_set_boolean, 
//GO(g_value_set_boxed, 
//GO(g_value_set_boxed_take_ownership, 
//GO(g_value_set_char, 
//GO(g_value_set_double, 
//GO(g_value_set_enum, 
//GO(g_value_set_flags, 
//GO(g_value_set_float, 
//GO(g_value_set_gtype, 
GO(g_value_set_instance, vFpp)
//GO(g_value_set_int, 
//GO(g_value_set_int64, 
//GO(g_value_set_long, 
GO(g_value_set_object, vFpp)
GO(g_value_set_object_take_ownership, vFpp)
//GO(g_value_set_param, 
//GO(g_value_set_param_take_ownership, 
//GO(g_value_set_pointer, 
//GO(g_value_set_schar, 
//GO(g_value_set_static_boxed, 
//GO(g_value_set_static_string, 
//GO(g_value_set_string, 
//GO(g_value_set_string_take_ownership, 
//GO(g_value_set_uchar, 
//GO(g_value_set_uint, 
//GO(g_value_set_uint64, 
//GO(g_value_set_ulong, 
//GO(g_value_set_variant, 
//GO(g_value_take_boxed, 
//GO(g_value_take_object, 
//GO(g_value_take_param, 
//GO(g_value_take_string, 
//GO(g_value_take_variant, 
GO(g_value_transform, iFpp)
GO(g_value_type_compatible, iFii)
GO(g_value_type_transformable, iFii)
GO(g_value_unset, vFp)
//GO(g_variant_builder_get_type, 
//GO(g_variant_dict_get_type, 
//GO(g_variant_get_gtype, 
//GO(g_variant_type_get_gtype, 
//GO(g_weak_ref_clear, 
//GO(g_weak_ref_get, 
//GO(g_weak_ref_init, 
//GO(g_weak_ref_set, 
//GO(_init, 
