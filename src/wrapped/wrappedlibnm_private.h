#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(nm_802_11_ap_flags_get_type, iFv)
GO(nm_802_11_ap_security_flags_get_type, iFv)
GO(nm_802_11_mode_get_type, iFv)
GO(nm_access_point_connection_valid, iFp)
GO(nm_access_point_filter_connections, pFpp)
GO(nm_access_point_get_bssid, pFp)
GO(nm_access_point_get_flags, iFp)
GO(nm_access_point_get_frequency, uFp)
GO(nm_access_point_get_last_seen, iFp)
GO(nm_access_point_get_max_bitrate, uFp)
GO(nm_access_point_get_mode, iFp)
GO(nm_access_point_get_rsn_flags, iFp)
GO(nm_access_point_get_ssid, pFp)
GO(nm_access_point_get_strength, CFp)
GO(nm_access_point_get_type, iFv)
GO(nm_access_point_get_wpa_flags, iFp)
GO(nm_activation_state_flags_get_type, iFv)
//GO(nm_active_connection_get_connection, 
//GO(nm_active_connection_get_connection_type, 
GO(nm_active_connection_get_default, iFp)
GO(nm_active_connection_get_default6, iFp)
//GO(nm_active_connection_get_devices, 
//GO(nm_active_connection_get_dhcp4_config, 
//GO(nm_active_connection_get_dhcp6_config, 
//GO(nm_active_connection_get_id, 
//GO(nm_active_connection_get_ip4_config, 
//GO(nm_active_connection_get_ip6_config, 
//GO(nm_active_connection_get_master, 
//GO(nm_active_connection_get_specific_object_path, 
GO(nm_active_connection_get_state, iFp)
GO(nm_active_connection_get_state_flags, iFp)
GO(nm_active_connection_get_state_reason, iFp)
GO(nm_active_connection_get_type, pFv)
//GO(nm_active_connection_get_uuid, 
//GO(nm_active_connection_get_vpn, 
//GO(nm_active_connection_state_get_type, 
//GO(nm_active_connection_state_reason_get_type, 
//GO(nm_agent_manager_error_get_type, 
//GO(nm_agent_manager_error_quark, 
//GO(nm_bluetooth_capabilities_get_type, 
//GO(nm_capability_get_type, 
//GO(nm_checkpoint_create_flags_get_type, 
//GO(nm_checkpoint_get_created, 
//GO(nm_checkpoint_get_devices, 
//GO(nm_checkpoint_get_rollback_timeout, 
GO(nm_checkpoint_get_type, iFv)
GOM(nm_client_activate_connection_async, vFEppppppp)
GO(nm_client_activate_connection_finish, pFppp)
GOM(nm_client_add_and_activate_connection_async, vFEppppppp)
GO(nm_client_add_and_activate_connection_finish, pFppp)
GOM(nm_client_add_connection_async, vFEppippp)
GO(nm_client_add_connection_finish, iFppp)
GO(nm_client_check_connectivity, iFppp)
//GO(nm_client_check_connectivity_async, 
//GO(nm_client_check_connectivity_finish, 
//GOM(nm_client_checkpoint_adjust_rollback_timeout, vFEppupBp)
GO(nm_client_checkpoint_adjust_rollback_timeout_finish, iFppp)
//GOM(nm_client_checkpoint_create, vFEppuipBp)
GO(nm_client_checkpoint_create_finish, pFppp)
//GOM(nm_client_checkpoint_destroy, vFEpppBp)
GO(nm_client_checkpoint_destroy_finish, iFppp)
//GOM(nm_client_checkpoint_rollback, vFEpppBp)
GO(nm_client_checkpoint_rollback_finish, pFppp)
GO(nm_client_connectivity_check_get_available, iFp)
GO(nm_client_connectivity_check_get_enabled, iFp)
GO(nm_client_connectivity_check_set_enabled, vFpi)
GO(nm_client_deactivate_connection, iFpppp)
//GOM(nm_client_deactivate_connection_async, vFEpppBp)
GO(nm_client_deactivate_connection_finish, iFppp)
GO(nm_client_error_get_type, iFv)
GO(nm_client_error_quark, uFv)
GO(nm_client_get_activating_connection, pFp)
GO(nm_client_get_active_connections, pFp)
GO(nm_client_get_all_devices, pFp)
GO(nm_client_get_checkpoints, pFp)
GO(nm_client_get_connection_by_id, pFpp)
GO(nm_client_get_connection_by_path, pFpp)
GO(nm_client_get_connection_by_uuid, pFpp)
GO(nm_client_get_connections, pFp)
GO(nm_client_get_connectivity, iFp)
GO(nm_client_get_device_by_iface, pFpp)
GO(nm_client_get_device_by_path, pFpp)
GO(nm_client_get_devices, pFp)
GO(nm_client_get_dns_configuration, pFp)
GO(nm_client_get_dns_mode, pFp)
GO(nm_client_get_dns_rc_manager, pFp)
GO(nm_client_get_logging, iFpppp)
GO(nm_client_get_nm_running, iFp)
GO(nm_client_get_permission_result, iFpi)
GO(nm_client_get_primary_connection, pFp)
GO(nm_client_get_startup, iFp)
GO(nm_client_get_state, iFp)
GO(nm_client_get_type, iFv)
GO(nm_client_get_version, pFp)
GO(nm_client_load_connections, iFppppp)
//GOM(nm_client_load_connections_async, vFEpppBp)
GO(nm_client_load_connections_finish, iFpppp)
GO(nm_client_networking_get_enabled, iFp)
GO(nm_client_networking_set_enabled, iFpip)
GO(nm_client_new, pFpp)
//GOM(nm_client_new_async, vFpBp)
GO(nm_client_new_finish, pFpp)
GO(nm_client_permission_get_type, iFv)
GO(nm_client_permission_result_get_type, iFv)
GO(nm_client_reload_connections, iFppp)
//GOM(nm_client_reload_connections_async, vFEppBp)
GO(nm_client_reload_connections_finish, iFppp)
GO(nm_client_save_hostname, iFpppp)
//GOM(nm_client_save_hostname_async, vFpppBp)
GO(nm_client_save_hostname_finish, iFppp)
GO(nm_client_set_logging, iFpppp)
GO(nm_client_wimax_get_enabled, iFp)
GO(nm_client_wimax_hardware_get_enabled, iFp)
GO(nm_client_wimax_set_enabled, vFpi)
GO(nm_client_wireless_get_enabled, iFp)
GO(nm_client_wireless_hardware_get_enabled, iFp)
GO(nm_client_wireless_set_enabled, vFpi)
GO(nm_client_wwan_get_enabled, iFp)
GO(nm_client_wwan_hardware_get_enabled, iFp)
GO(nm_client_wwan_set_enabled, vFpi)
GO(nm_connection_add_setting, vFpp)
GO(nm_connection_clear_secrets, vFp)
//GOM(nm_connection_clear_secrets_with_flags, vFEpBp)
GO(nm_connection_clear_settings, vFp)
GO(nm_connection_compare, iFppi)
GO(nm_connection_diff, iFppip)
GO(nm_connection_dump, vFp)
//GO(nm_connection_error_get_type, 
//GO(nm_connection_error_quark, 
//GOM(nm_connection_for_each_setting_value, vFEpBp)
GO(nm_connection_get_connection_type, pFp)
GO(nm_connection_get_id, pFp)
GO(nm_connection_get_interface_name, pFp)
GO(nm_connection_get_path, pFp)
GO(nm_connection_get_setting, pFpi)
GO(nm_connection_get_setting_802_1x, pFp)
GO(nm_connection_get_setting_adsl, pFp)
GO(nm_connection_get_setting_bluetooth, pFp)
GO(nm_connection_get_setting_bond, pFp)
GO(nm_connection_get_setting_bridge, pFp)
GO(nm_connection_get_setting_bridge_port, pFp)
GO(nm_connection_get_setting_by_name, pFpp)
GO(nm_connection_get_setting_cdma, pFp)
GO(nm_connection_get_setting_connection, pFp)
GO(nm_connection_get_setting_dcb, pFp)
GO(nm_connection_get_setting_dummy, pFp)
GO(nm_connection_get_setting_generic, pFp)
GO(nm_connection_get_setting_gsm, pFp)
GO(nm_connection_get_setting_infiniband, pFp)
GO(nm_connection_get_setting_ip4_config, pFp)
GO(nm_connection_get_setting_ip6_config, pFp)
GO(nm_connection_get_setting_ip_tunnel, pFp)
GO(nm_connection_get_setting_macsec, pFp)
GO(nm_connection_get_setting_macvlan, pFp)
GO(nm_connection_get_setting_olpc_mesh, pFp)
GO(nm_connection_get_setting_ovs_bridge, pFp)
GO(nm_connection_get_setting_ovs_interface, pFp)
GO(nm_connection_get_setting_ovs_patch, pFp)
GO(nm_connection_get_setting_ovs_port, pFp)
GO(nm_connection_get_setting_ppp, pFp)
GO(nm_connection_get_setting_pppoe, pFp)
GO(nm_connection_get_setting_proxy, pFp)
GO(nm_connection_get_settings, pFpp)
GO(nm_connection_get_setting_serial, pFp)
GO(nm_connection_get_setting_tc_config, pFp)
GO(nm_connection_get_setting_team, pFp)
GO(nm_connection_get_setting_team_port, pFp)
GO(nm_connection_get_setting_tun, pFp)
GO(nm_connection_get_setting_vlan, pFp)
GO(nm_connection_get_setting_vpn, pFp)
GO(nm_connection_get_setting_vxlan, pFp)
GO(nm_connection_get_setting_wimax, pFp)
GO(nm_connection_get_setting_wired, pFp)
GO(nm_connection_get_setting_wireless, pFp)
GO(nm_connection_get_setting_wireless_security, pFp)
GO(nm_connection_get_type, iFv)
GO(nm_connection_get_uuid, pFp)
GO(nm_connection_get_virtual_device_description, pFp)
GO(nm_connection_is_type, iFpp)
GO(nm_connection_is_virtual, iFp)
GO(nm_connection_multi_connect_get_type, iFv)
GO(nm_connection_need_secrets, pFpp)
GO(nm_connection_normalize, iFpppp)
GO(nm_connection_remove_setting, vFpi)
GO(nm_connection_replace_settings, iFppp)
GO(nm_connection_replace_settings_from_connection, vFpp)
GO(nm_connection_serialization_flags_get_type, iFv)
GO(nm_connection_set_path, vFpp)
GO(nm_connection_to_dbus, pFpi)
GO(nm_connection_update_secrets, iFpppp)
GO(nm_connection_verify, iFpp)
GO(nm_connection_verify_secrets, iFpp)
GO(nm_connectivity_state_get_type, iFv)
GO(nm_crypto_error_get_type, iFv)
//GO(nm_crypto_error_quark, 
GO(nm_device_6lowpan_get_type, iFv)
//GO(nm_device_adsl_get_carrier, 
GO(nm_device_adsl_get_type, iFv)
//GO(nm_device_bond_get_carrier, 
//GO(nm_device_bond_get_hw_address, 
//GO(nm_device_bond_get_slaves, 
GO(nm_device_bond_get_type, iFv)
//GO(nm_device_bridge_get_carrier, 
//GO(nm_device_bridge_get_hw_address, 
//GO(nm_device_bridge_get_slaves, 
GO(nm_device_bridge_get_type, iFv)
//GO(nm_device_bt_get_capabilities, 
//GO(nm_device_bt_get_hw_address, 
//GO(nm_device_bt_get_name, 
GO(nm_device_bt_get_type, iFv)
GO(nm_device_capabilities_get_type, iFv)
GO(nm_device_connection_compatible, iFppp)
GO(nm_device_connection_valid, iFpp)
GO(nm_device_delete, iFppp)
//GOM(nm_device_delete_async, vFEppBp)
GO(nm_device_delete_finish, iFppp)
GO(nm_device_disambiguate_names, pFpi)
GO(nm_device_disconnect, iFppp)
GOM(nm_device_disconnect_async, vFEpppp)
GO(nm_device_disconnect_finish, iFppp)
//GO(nm_device_dummy_get_hw_address, 
//GO(nm_device_dummy_get_type, 
//GO(nm_device_error_get_type, 
//GO(nm_device_error_quark, 
GO(nm_device_ethernet_get_carrier, iFp)
//GO(nm_device_ethernet_get_hw_address, 
GO(nm_device_ethernet_get_permanent_hw_address, pFp)
//GO(nm_device_ethernet_get_s390_subchannels, 
GO(nm_device_ethernet_get_speed, uFp)
GO(nm_device_ethernet_get_type, pFv)
GO(nm_device_filter_connections, pFpp)
//GO(nm_device_generic_get_hw_address, 
GO(nm_device_generic_get_type, iFv)
GO(nm_device_get_active_connection, pFp)
GO(nm_device_get_applied_connection, pFpuppp)
//GOM(nm_device_get_applied_connection_async, vFEpupBp)
GO(nm_device_get_applied_connection_finish, pFpppp)
GO(nm_device_get_autoconnect, iFp)
GO(nm_device_get_available_connections, pFp)
GO(nm_device_get_capabilities, iFp)
GO(nm_device_get_description, pFp)
GO(nm_device_get_device_type, iFp)
GO(nm_device_get_dhcp4_config, pFp)
GO(nm_device_get_dhcp6_config, pFp)
GO(nm_device_get_driver, pFp)
GO(nm_device_get_driver_version, pFp)
GO(nm_device_get_firmware_missing, iFp)
GO(nm_device_get_firmware_version, pFp)
GO(nm_device_get_hw_address, pFp)
GO(nm_device_get_iface, pFp)
GO(nm_device_get_ip4_config, pFp)
GO(nm_device_get_ip6_config, pFp)
GO(nm_device_get_ip_iface, pFp)
GO(nm_device_get_lldp_neighbors, pFp)
GO(nm_device_get_managed, iFp)
GO(nm_device_get_metered, iFp)
GO(nm_device_get_mtu, uFp)
GO(nm_device_get_nm_plugin_missing, iFp)
GO(nm_device_get_physical_port_id, pFp)
GO(nm_device_get_product, pFp)
GO(nm_device_get_setting_type, iFp)
GO(nm_device_get_state, iFp)
GO(nm_device_get_state_reason, iFp)
GO(nm_device_get_type, iFv)
GO(nm_device_get_type_description, pFp)
GO(nm_device_get_udi, pFp)
GO(nm_device_get_vendor, pFp)
//GO(nm_device_infiniband_get_carrier, 
//GO(nm_device_infiniband_get_hw_address, 
//GO(nm_device_infiniband_get_type, 
//GO(nm_device_ip_tunnel_get_encapsulation_limit, 
//GO(nm_device_ip_tunnel_get_flags, 
//GO(nm_device_ip_tunnel_get_flow_label, 
//GO(nm_device_ip_tunnel_get_input_key, 
//GO(nm_device_ip_tunnel_get_local, 
//GO(nm_device_ip_tunnel_get_mode, 
//GO(nm_device_ip_tunnel_get_output_key, 
//GO(nm_device_ip_tunnel_get_parent, 
//GO(nm_device_ip_tunnel_get_path_mtu_discovery, 
//GO(nm_device_ip_tunnel_get_remote, 
//GO(nm_device_ip_tunnel_get_tos, 
//GO(nm_device_ip_tunnel_get_ttl, 
//GO(nm_device_ip_tunnel_get_type, 
GO(nm_device_is_real, iFp)
GO(nm_device_is_software, iFp)
//GO(nm_device_macsec_get_cipher_suite, 
//GO(nm_device_macsec_get_encoding_sa, 
//GO(nm_device_macsec_get_encrypt, 
//GO(nm_device_macsec_get_es, 
//GO(nm_device_macsec_get_hw_address, 
//GO(nm_device_macsec_get_icv_length, 
//GO(nm_device_macsec_get_include_sci, 
//GO(nm_device_macsec_get_protect, 
//GO(nm_device_macsec_get_replay_protect, 
//GO(nm_device_macsec_get_scb, 
//GO(nm_device_macsec_get_sci, 
//GO(nm_device_macsec_get_type, 
//GO(nm_device_macsec_get_validation, 
//GO(nm_device_macsec_get_window, 
//GO(nm_device_macvlan_get_hw_address, 
//GO(nm_device_macvlan_get_mode, 
//GO(nm_device_macvlan_get_no_promisc, 
//GO(nm_device_macvlan_get_parent, 
//GO(nm_device_macvlan_get_tap, 
//GO(nm_device_macvlan_get_type, 
//GO(nm_device_modem_capabilities_get_type, 
//GO(nm_device_modem_get_current_capabilities, 
//GO(nm_device_modem_get_modem_capabilities, 
//GO(nm_device_modem_get_type, 
//GO(nm_device_olpc_mesh_get_active_channel, 
//GO(nm_device_olpc_mesh_get_companion, 
//GO(nm_device_olpc_mesh_get_hw_address, 
//GO(nm_device_olpc_mesh_get_type, 
//GO(nm_device_ovs_bridge_get_slaves, 
//GO(nm_device_ovs_bridge_get_type, 
//GO(nm_device_ovs_interface_get_type, 
//GO(nm_device_ovs_port_get_slaves, 
//GO(nm_device_ovs_port_get_type, 
GO(nm_device_ppp_get_type, iFv)
GO(nm_device_reapply, iFppUupp)
//GOM(nm_device_reapply_async, vFEppUupBp)
GO(nm_device_reapply_finish, iFppp)
GO(nm_device_set_autoconnect, vFpi)
GO(nm_device_set_managed, vFpi)
GO(nm_device_state_get_type, iFv)
GO(nm_device_state_reason_get_type, iFv)
//GO(nm_device_team_get_carrier, 
//GO(nm_device_team_get_config, 
//GO(nm_device_team_get_hw_address, 
//GO(nm_device_team_get_slaves, 
//GO(nm_device_team_get_type, 
//GO(nm_device_tun_get_group, 
//GO(nm_device_tun_get_hw_address, 
//GO(nm_device_tun_get_mode, 
//GO(nm_device_tun_get_multi_queue, 
//GO(nm_device_tun_get_no_pi, 
//GO(nm_device_tun_get_owner, 
//GO(nm_device_tun_get_type, 
//GO(nm_device_tun_get_vnet_hdr, 
//GO(nm_device_type_get_type, 
//GO(nm_device_vlan_get_carrier, 
//GO(nm_device_vlan_get_hw_address, 
//GO(nm_device_vlan_get_parent, 
//GO(nm_device_vlan_get_type, 
//GO(nm_device_vlan_get_vlan_id, 
//GO(nm_device_vxlan_get_ageing, 
//GO(nm_device_vxlan_get_dst_port, 
//GO(nm_device_vxlan_get_group, 
//GO(nm_device_vxlan_get_hw_address, 
//GO(nm_device_vxlan_get_id, 
//GO(nm_device_vxlan_get_l2miss, 
//GO(nm_device_vxlan_get_l3miss, 
//GO(nm_device_vxlan_get_learning, 
//GO(nm_device_vxlan_get_limit, 
//GO(nm_device_vxlan_get_local, 
//GO(nm_device_vxlan_get_parent, 
//GO(nm_device_vxlan_get_proxy, 
//GO(nm_device_vxlan_get_src_port_max, 
//GO(nm_device_vxlan_get_src_port_min, 
//GO(nm_device_vxlan_get_tos, 
//GO(nm_device_vxlan_get_ttl, 
//GO(nm_device_vxlan_get_type, 
//GO(nm_device_wifi_capabilities_get_type, 
GO(nm_device_wifi_get_access_point_by_path, pFpp)
GO(nm_device_wifi_get_access_points, pFp)
GO(nm_device_wifi_get_active_access_point, pFp)
GO(nm_device_wifi_get_bitrate, uFp)
GO(nm_device_wifi_get_capabilities, iFp)
GO(nm_device_wifi_get_hw_address, pFp)
GO(nm_device_wifi_get_last_scan, IFp)
GO(nm_device_wifi_get_mode, iFp)
GO(nm_device_wifi_get_permanent_hw_address, pFp)
GO(nm_device_wifi_get_type, iFv)
GO(nm_device_wifi_request_scan, iFppp)
GOM(nm_device_wifi_request_scan_async, vFEpppp)
GO(nm_device_wifi_request_scan_finish, iFppp)
GO(nm_device_wifi_request_scan_options, iFpppp)
//GOM(nm_device_wifi_request_scan_options_async, vFEpppBp)
//GO(nm_device_wimax_get_active_nsp, 
//GO(nm_device_wimax_get_bsid, 
//GO(nm_device_wimax_get_center_frequency, 
//GO(nm_device_wimax_get_cinr, 
//GO(nm_device_wimax_get_hw_address, 
//GO(nm_device_wimax_get_nsp_by_path, 
//GO(nm_device_wimax_get_nsps, 
//GO(nm_device_wimax_get_rssi, 
//GO(nm_device_wimax_get_tx_power, 
//GO(nm_device_wimax_get_type, 
//GO(nm_device_wireguard_get_fwmark, 
//GO(nm_device_wireguard_get_listen_port, 
//GO(nm_device_wireguard_get_public_key, 
//GO(nm_device_wireguard_get_type, 
//GO(nm_device_wpan_get_type, 
//GO(nm_dhcp_config_get_family, 
//GO(nm_dhcp_config_get_one_option, 
//GO(nm_dhcp_config_get_options, 
GO(nm_dhcp_config_get_type, iFv)
GO(nm_dns_entry_get_domains, pFp)
GO(nm_dns_entry_get_interface, pFp)
GO(nm_dns_entry_get_nameservers, pFp)
GO(nm_dns_entry_get_priority, iFp)
GO(nm_dns_entry_get_type, iFv)
GO(nm_dns_entry_get_vpn, iFp)
GO(nm_dns_entry_unref, vFp)
//GO(nm_ip_address_equal, 
GO(nm_ip_address_get_address, pFp)
GO(nm_ip_address_get_address_binary, vFpp)
//GO(nm_ip_address_get_attribute, 
//GO(nm_ip_address_get_attribute_names, 
//GO(nm_ip_address_get_family, 
GO(nm_ip_address_get_prefix, uFp)
//GO(nm_ip_address_get_type, 
GO(nm_ip_address_new, pFipup)
//GO(nm_ip_address_new_binary, 
GO(nm_ip_address_ref, vFp)
//GO(nm_ip_address_set_address, 
//GO(nm_ip_address_set_address_binary, 
//GO(nm_ip_address_set_attribute, 
//GO(nm_ip_address_set_prefix, 
GO(nm_ip_address_unref, vFp)
GO(nm_ip_config_get_addresses, pFp)
//GO(nm_ip_config_get_domains, 
//GO(nm_ip_config_get_family, 
GO(nm_ip_config_get_gateway, pFp)
GO(nm_ip_config_get_nameservers, pFp)
//GO(nm_ip_config_get_routes, 
//GO(nm_ip_config_get_searches, 
//GO(nm_ip_config_get_type, 
//GO(nm_ip_config_get_wins_servers, 
//GO(nm_ip_route_attribute_validate, 
//GO(nm_ip_route_equal, 
//GO(nm_ip_route_equal_full, 
//GO(nm_ip_route_get_attribute, 
//GO(nm_ip_route_get_attribute_names, 
//GO(nm_ip_route_get_dest, 
//GO(nm_ip_route_get_dest_binary, 
//GO(nm_ip_route_get_family, 
//GO(nm_ip_route_get_metric, 
//GO(nm_ip_route_get_next_hop, 
//GO(nm_ip_route_get_next_hop_binary, 
//GO(nm_ip_route_get_prefix, 
//GO(nm_ip_route_get_type, 
//GO(nm_ip_route_get_variant_attribute_spec, 
//GO(nm_ip_route_new, 
//GO(nm_ip_route_new_binary, 
//GO(nm_ip_route_ref, 
//GO(nm_ip_route_set_attribute, 
//GO(nm_ip_route_set_dest, 
//GO(nm_ip_route_set_dest_binary, 
//GO(nm_ip_route_set_metric, 
//GO(nm_ip_route_set_next_hop, 
//GO(nm_ip_route_set_next_hop_binary, 
//GO(nm_ip_route_set_prefix, 
//GO(nm_ip_route_unref, 
//GO(nm_ip_tunnel_flags_get_type, 
//GO(nm_ip_tunnel_mode_get_type, 
GO(nm_lldp_neighbor_get_attr_names, pFp)
GO(nm_lldp_neighbor_get_attr_string_value, iFppp)
GO(nm_lldp_neighbor_get_attr_type, pFpp)
GO(nm_lldp_neighbor_get_attr_uint_value, iFppp)
GO(nm_lldp_neighbor_get_type, iFv)
GO(nm_lldp_neighbor_new, pFv)
GO(nm_lldp_neighbor_ref, pFp)
GO(nm_lldp_neighbor_unref, vFp)
GO(nm_manager_error_get_type, iFv)
GO(nm_manager_error_quark, uFv)
GO(nm_metered_get_type, iFv)
//GO(nm_object_get_path, 
GO(nm_object_get_type, iFv)
GO(nm_remote_connection_commit_changes, iFpipp)
GOM(nm_remote_connection_commit_changes_async, vFEpippp)
GO(nm_remote_connection_commit_changes_finish, iFppp)
//GO(nm_remote_connection_delete, 
GOM(nm_remote_connection_delete_async, vFEpppp)
GO(nm_remote_connection_delete_finish, iFppp)
//GO(nm_remote_connection_get_filename, 
//GO(nm_remote_connection_get_flags, 
//GO(nm_remote_connection_get_secrets, 
GOM(nm_remote_connection_get_secrets_async, vFEppppp)
GO(nm_remote_connection_get_secrets_finish, pFppp)
GO(nm_remote_connection_get_type, pFv)
//GO(nm_remote_connection_get_unsaved, 
//GO(nm_remote_connection_get_visible, 
//GO(nm_remote_connection_save, 
//GO(nm_remote_connection_save_async, 
//GO(nm_remote_connection_save_finish, 
//GO(nm_remote_connection_update2, 
//GO(nm_remote_connection_update2_finish, 
//GO(nm_secret_agent_capabilities_get_type, 
//GO(nm_secret_agent_error_get_type, 
//GO(nm_secret_agent_error_quark, 
//GO(nm_secret_agent_get_secrets_flags_get_type, 
//GO(nm_secret_agent_old_delete_secrets, 
//GO(nm_secret_agent_old_get_registered, 
//GO(nm_secret_agent_old_get_secrets, 
//GO(nm_secret_agent_old_get_type, 
//GO(nm_secret_agent_old_register, 
//GO(nm_secret_agent_old_register_async, 
//GO(nm_secret_agent_old_register_finish, 
//GO(nm_secret_agent_old_save_secrets, 
//GO(nm_secret_agent_old_unregister, 
//GO(nm_secret_agent_old_unregister_async, 
//GO(nm_secret_agent_old_unregister_finish, 
//GO(nm_setting_6lowpan_get_type, 
//GO(nm_setting_802_1x_add_altsubject_match, 
GO(nm_setting_802_1x_add_eap_method, iFpp)
//GO(nm_setting_802_1x_add_phase2_altsubject_match, 
//GO(nm_setting_802_1x_auth_flags_get_type, 
//GO(nm_setting_802_1x_check_cert_scheme, 
//GO(nm_setting_802_1x_ck_format_get_type, 
//GO(nm_setting_802_1x_ck_scheme_get_type, 
//GO(nm_setting_802_1x_clear_altsubject_matches, 
//GO(nm_setting_802_1x_clear_eap_methods, 
//GO(nm_setting_802_1x_clear_phase2_altsubject_matches, 
//GO(nm_setting_802_1x_get_altsubject_match, 
//GO(nm_setting_802_1x_get_anonymous_identity, 
//GO(nm_setting_802_1x_get_auth_timeout, 
//GO(nm_setting_802_1x_get_ca_cert_blob, 
//GO(nm_setting_802_1x_get_ca_cert_password, 
//GO(nm_setting_802_1x_get_ca_cert_password_flags, 
//GO(nm_setting_802_1x_get_ca_cert_path, 
//GO(nm_setting_802_1x_get_ca_cert_scheme, 
//GO(nm_setting_802_1x_get_ca_cert_uri, 
//GO(nm_setting_802_1x_get_ca_path, 
//GO(nm_setting_802_1x_get_client_cert_blob, 
//GO(nm_setting_802_1x_get_client_cert_password, 
//GO(nm_setting_802_1x_get_client_cert_password_flags, 
//GO(nm_setting_802_1x_get_client_cert_path, 
//GO(nm_setting_802_1x_get_client_cert_scheme, 
//GO(nm_setting_802_1x_get_client_cert_uri, 
//GO(nm_setting_802_1x_get_domain_suffix_match, 
//GO(nm_setting_802_1x_get_eap_method, 
//GO(nm_setting_802_1x_get_identity, 
//GO(nm_setting_802_1x_get_num_altsubject_matches, 
//GO(nm_setting_802_1x_get_num_eap_methods, 
//GO(nm_setting_802_1x_get_num_phase2_altsubject_matches, 
//GO(nm_setting_802_1x_get_pac_file, 
//GO(nm_setting_802_1x_get_password, 
//GO(nm_setting_802_1x_get_password_flags, 
//GO(nm_setting_802_1x_get_password_raw, 
//GO(nm_setting_802_1x_get_password_raw_flags, 
//GO(nm_setting_802_1x_get_phase1_auth_flags, 
//GO(nm_setting_802_1x_get_phase1_fast_provisioning, 
//GO(nm_setting_802_1x_get_phase1_peaplabel, 
//GO(nm_setting_802_1x_get_phase1_peapver, 
//GO(nm_setting_802_1x_get_phase2_altsubject_match, 
//GO(nm_setting_802_1x_get_phase2_auth, 
//GO(nm_setting_802_1x_get_phase2_autheap, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_blob, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_password, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_password_flags, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_path, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_scheme, 
//GO(nm_setting_802_1x_get_phase2_ca_cert_uri, 
//GO(nm_setting_802_1x_get_phase2_ca_path, 
//GO(nm_setting_802_1x_get_phase2_client_cert_blob, 
//GO(nm_setting_802_1x_get_phase2_client_cert_password, 
//GO(nm_setting_802_1x_get_phase2_client_cert_password_flags, 
//GO(nm_setting_802_1x_get_phase2_client_cert_path, 
//GO(nm_setting_802_1x_get_phase2_client_cert_scheme, 
//GO(nm_setting_802_1x_get_phase2_client_cert_uri, 
//GO(nm_setting_802_1x_get_phase2_domain_suffix_match, 
//GO(nm_setting_802_1x_get_phase2_private_key_blob, 
//GO(nm_setting_802_1x_get_phase2_private_key_format, 
//GO(nm_setting_802_1x_get_phase2_private_key_password, 
//GO(nm_setting_802_1x_get_phase2_private_key_password_flags, 
//GO(nm_setting_802_1x_get_phase2_private_key_path, 
//GO(nm_setting_802_1x_get_phase2_private_key_scheme, 
//GO(nm_setting_802_1x_get_phase2_private_key_uri, 
//GO(nm_setting_802_1x_get_phase2_subject_match, 
//GO(nm_setting_802_1x_get_pin, 
//GO(nm_setting_802_1x_get_pin_flags, 
//GO(nm_setting_802_1x_get_private_key_blob, 
//GO(nm_setting_802_1x_get_private_key_format, 
//GO(nm_setting_802_1x_get_private_key_password, 
//GO(nm_setting_802_1x_get_private_key_password_flags, 
//GO(nm_setting_802_1x_get_private_key_path, 
//GO(nm_setting_802_1x_get_private_key_scheme, 
//GO(nm_setting_802_1x_get_private_key_uri, 
//GO(nm_setting_802_1x_get_subject_match, 
//GO(nm_setting_802_1x_get_system_ca_certs, 
//GO(nm_setting_802_1x_get_type, 
GO(nm_setting_802_1x_new, pFv)
//GO(nm_setting_802_1x_remove_altsubject_match, 
//GO(nm_setting_802_1x_remove_altsubject_match_by_value, 
//GO(nm_setting_802_1x_remove_eap_method, 
//GO(nm_setting_802_1x_remove_eap_method_by_value, 
//GO(nm_setting_802_1x_remove_phase2_altsubject_match, 
//GO(nm_setting_802_1x_remove_phase2_altsubject_match_by_value, 
//GO(nm_setting_802_1x_set_ca_cert, 
//GO(nm_setting_802_1x_set_client_cert, 
//GO(nm_setting_802_1x_set_phase2_ca_cert, 
//GO(nm_setting_802_1x_set_phase2_client_cert, 
//GO(nm_setting_802_1x_set_phase2_private_key, 
//GO(nm_setting_802_1x_set_private_key, 
//GO(nm_setting_adsl_get_encapsulation, 
//GO(nm_setting_adsl_get_password, 
//GO(nm_setting_adsl_get_password_flags, 
//GO(nm_setting_adsl_get_protocol, 
//GO(nm_setting_adsl_get_type, 
//GO(nm_setting_adsl_get_username, 
//GO(nm_setting_adsl_get_vci, 
//GO(nm_setting_adsl_get_vpi, 
//GO(nm_setting_adsl_new, 
//GO(nm_setting_bluetooth_get_bdaddr, 
//GO(nm_setting_bluetooth_get_connection_type, 
//GO(nm_setting_bluetooth_get_type, 
//GO(nm_setting_bluetooth_new, 
//GO(nm_setting_bond_add_option, 
//GO(nm_setting_bond_get_num_options, 
//GO(nm_setting_bond_get_option, 
//GO(nm_setting_bond_get_option_by_name, 
//GO(nm_setting_bond_get_option_default, 
//GO(nm_setting_bond_get_type, 
//GO(nm_setting_bond_get_valid_options, 
//GO(nm_setting_bond_new, 
//GO(nm_setting_bond_remove_option, 
//GO(nm_setting_bond_validate_option, 
//GO(nm_setting_bridge_get_ageing_time, 
//GO(nm_setting_bridge_get_forward_delay, 
//GO(nm_setting_bridge_get_group_forward_mask, 
//GO(nm_setting_bridge_get_hello_time, 
//GO(nm_setting_bridge_get_mac_address, 
//GO(nm_setting_bridge_get_max_age, 
//GO(nm_setting_bridge_get_multicast_snooping, 
//GO(nm_setting_bridge_get_priority, 
//GO(nm_setting_bridge_get_stp, 
//GO(nm_setting_bridge_get_type, 
//GO(nm_setting_bridge_new, 
//GO(nm_setting_bridge_port_get_hairpin_mode, 
//GO(nm_setting_bridge_port_get_path_cost, 
//GO(nm_setting_bridge_port_get_priority, 
//GO(nm_setting_bridge_port_get_type, 
//GO(nm_setting_bridge_port_new, 
//GO(nm_setting_cdma_get_mtu, 
//GO(nm_setting_cdma_get_number, 
//GO(nm_setting_cdma_get_password, 
//GO(nm_setting_cdma_get_password_flags, 
//GO(nm_setting_cdma_get_type, 
//GO(nm_setting_cdma_get_username, 
//GO(nm_setting_cdma_new, 
//GO(nm_setting_compare, 
//GO(nm_setting_compare_flags_get_type, 
//GO(nm_setting_connection_add_permission, 
//GO(nm_setting_connection_add_secondary, 
//GO(nm_setting_connection_autoconnect_slaves_get_type, 
//GO(nm_setting_connection_get_auth_retries, 
GO(nm_setting_connection_get_autoconnect, iFp)
//GO(nm_setting_connection_get_autoconnect_priority, 
//GO(nm_setting_connection_get_autoconnect_retries, 
//GO(nm_setting_connection_get_autoconnect_slaves, 
//GO(nm_setting_connection_get_connection_type, 
//GO(nm_setting_connection_get_gateway_ping_timeout, 
GO(nm_setting_connection_get_id, pFp)
//GO(nm_setting_connection_get_interface_name, 
//GO(nm_setting_connection_get_lldp, 
//GO(nm_setting_connection_get_llmnr, 
//GO(nm_setting_connection_get_master, 
//GO(nm_setting_connection_get_mdns, 
//GO(nm_setting_connection_get_metered, 
//GO(nm_setting_connection_get_multi_connect, 
//GO(nm_setting_connection_get_num_permissions, 
//GO(nm_setting_connection_get_num_secondaries, 
//GO(nm_setting_connection_get_permission, 
//GO(nm_setting_connection_get_read_only, 
//GO(nm_setting_connection_get_secondary, 
//GO(nm_setting_connection_get_slave_type, 
//GO(nm_setting_connection_get_stable_id, 
//GO(nm_setting_connection_get_timestamp, 
//GO(nm_setting_connection_get_type, 
//GO(nm_setting_connection_get_uuid, 
//GO(nm_setting_connection_get_zone, 
//GO(nm_setting_connection_is_slave_type, 
//GO(nm_setting_connection_lldp_get_type, 
//GO(nm_setting_connection_llmnr_get_type, 
//GO(nm_setting_connection_mdns_get_type, 
GO(nm_setting_connection_new, pFv)
//GO(nm_setting_connection_permissions_user_allowed, 
//GO(nm_setting_connection_remove_permission, 
//GO(nm_setting_connection_remove_permission_by_value, 
//GO(nm_setting_connection_remove_secondary, 
//GO(nm_setting_connection_remove_secondary_by_value, 
//GO(nm_setting_dcb_flags_get_type, 
//GO(nm_setting_dcb_get_app_fcoe_flags, 
//GO(nm_setting_dcb_get_app_fcoe_mode, 
//GO(nm_setting_dcb_get_app_fcoe_priority, 
//GO(nm_setting_dcb_get_app_fip_flags, 
//GO(nm_setting_dcb_get_app_fip_priority, 
//GO(nm_setting_dcb_get_app_iscsi_flags, 
//GO(nm_setting_dcb_get_app_iscsi_priority, 
//GO(nm_setting_dcb_get_priority_bandwidth, 
//GO(nm_setting_dcb_get_priority_flow_control, 
//GO(nm_setting_dcb_get_priority_flow_control_flags, 
//GO(nm_setting_dcb_get_priority_group_bandwidth, 
//GO(nm_setting_dcb_get_priority_group_flags, 
//GO(nm_setting_dcb_get_priority_group_id, 
//GO(nm_setting_dcb_get_priority_strict_bandwidth, 
//GO(nm_setting_dcb_get_priority_traffic_class, 
//GO(nm_setting_dcb_get_type, 
//GO(nm_setting_dcb_new, 
//GO(nm_setting_dcb_set_priority_bandwidth, 
//GO(nm_setting_dcb_set_priority_flow_control, 
//GO(nm_setting_dcb_set_priority_group_bandwidth, 
//GO(nm_setting_dcb_set_priority_group_id, 
//GO(nm_setting_dcb_set_priority_strict_bandwidth, 
//GO(nm_setting_dcb_set_priority_traffic_class, 
//GO(nm_setting_diff, 
//GO(nm_setting_diff_result_get_type, 
//GO(nm_setting_dummy_get_type, 
//GO(nm_setting_dummy_new, 
//GO(nm_setting_duplicate, 
//GO(nm_setting_enumerate_values, 
//GO(nm_setting_ethtool_clear_features, 
//GO(nm_setting_ethtool_get_feature, 
//GO(nm_setting_ethtool_get_type, 
//GO(nm_setting_ethtool_new, 
//GO(nm_setting_ethtool_set_feature, 
//GO(nm_setting_generic_get_type, 
//GO(nm_setting_generic_new, 
//GO(nm_setting_get_dbus_property_type, 
//GO(nm_setting_get_name, 
//GO(nm_setting_get_secret_flags, 
GO(nm_setting_get_type, pFv)
//GO(nm_setting_gsm_get_apn, 
//GO(nm_setting_gsm_get_device_id, 
//GO(nm_setting_gsm_get_home_only, 
//GO(nm_setting_gsm_get_mtu, 
//GO(nm_setting_gsm_get_network_id, 
//GO(nm_setting_gsm_get_number, 
//GO(nm_setting_gsm_get_password, 
//GO(nm_setting_gsm_get_password_flags, 
//GO(nm_setting_gsm_get_pin, 
//GO(nm_setting_gsm_get_pin_flags, 
//GO(nm_setting_gsm_get_sim_id, 
//GO(nm_setting_gsm_get_sim_operator_id, 
//GO(nm_setting_gsm_get_type, 
//GO(nm_setting_gsm_get_username, 
//GO(nm_setting_gsm_new, 
//GO(nm_setting_infiniband_get_mac_address, 
//GO(nm_setting_infiniband_get_mtu, 
//GO(nm_setting_infiniband_get_parent, 
//GO(nm_setting_infiniband_get_p_key, 
//GO(nm_setting_infiniband_get_transport_mode, 
//GO(nm_setting_infiniband_get_type, 
//GO(nm_setting_infiniband_get_virtual_interface_name, 
//GO(nm_setting_infiniband_new, 
//GO(nm_setting_ip4_config_get_dhcp_client_id, 
//GO(nm_setting_ip4_config_get_dhcp_fqdn, 
//GO(nm_setting_ip4_config_get_type, 
GO(nm_setting_ip4_config_new, pFv)
//GO(nm_setting_ip6_config_addr_gen_mode_get_type, 
//GO(nm_setting_ip6_config_get_addr_gen_mode, 
//GO(nm_setting_ip6_config_get_dhcp_duid, 
//GO(nm_setting_ip6_config_get_ip6_privacy, 
//GO(nm_setting_ip6_config_get_token, 
//GO(nm_setting_ip6_config_get_type, 
GO(nm_setting_ip6_config_new, pFv)
//GO(nm_setting_ip6_config_privacy_get_type, 
GO(nm_setting_ip_config_add_address, iFpp)
GO(nm_setting_ip_config_add_dns, iFpp)
//GO(nm_setting_ip_config_add_dns_option, 
//GO(nm_setting_ip_config_add_dns_search, 
//GO(nm_setting_ip_config_add_route, 
//GO(nm_setting_ip_config_clear_addresses, 
//GO(nm_setting_ip_config_clear_dns, 
//GO(nm_setting_ip_config_clear_dns_options, 
//GO(nm_setting_ip_config_clear_dns_searches, 
//GO(nm_setting_ip_config_clear_routes, 
//GO(nm_setting_ip_config_get_address, 
//GO(nm_setting_ip_config_get_dad_timeout, 
//GO(nm_setting_ip_config_get_dhcp_hostname, 
//GO(nm_setting_ip_config_get_dhcp_send_hostname, 
//GO(nm_setting_ip_config_get_dhcp_timeout, 
//GO(nm_setting_ip_config_get_dns, 
//GO(nm_setting_ip_config_get_dns_option, 
//GO(nm_setting_ip_config_get_dns_priority, 
//GO(nm_setting_ip_config_get_dns_search, 
//GO(nm_setting_ip_config_get_gateway, 
//GO(nm_setting_ip_config_get_ignore_auto_dns, 
//GO(nm_setting_ip_config_get_ignore_auto_routes, 
//GO(nm_setting_ip_config_get_may_fail, 
//GO(nm_setting_ip_config_get_method, 
//GO(nm_setting_ip_config_get_never_default, 
//GO(nm_setting_ip_config_get_num_addresses, 
//GO(nm_setting_ip_config_get_num_dns, 
//GO(nm_setting_ip_config_get_num_dns_options, 
//GO(nm_setting_ip_config_get_num_dns_searches, 
//GO(nm_setting_ip_config_get_num_routes, 
//GO(nm_setting_ip_config_get_route, 
//GO(nm_setting_ip_config_get_route_metric, 
//GO(nm_setting_ip_config_get_route_table, 
GO(nm_setting_ip_config_get_type, pFv)
//GO(nm_setting_ip_config_has_dns_options, 
//GO(nm_setting_ip_config_remove_address, 
//GO(nm_setting_ip_config_remove_address_by_value, 
//GO(nm_setting_ip_config_remove_dns, 
//GO(nm_setting_ip_config_remove_dns_by_value, 
//GO(nm_setting_ip_config_remove_dns_option, 
//GO(nm_setting_ip_config_remove_dns_option_by_value, 
//GO(nm_setting_ip_config_remove_dns_search, 
//GO(nm_setting_ip_config_remove_dns_search_by_value, 
//GO(nm_setting_ip_config_remove_route, 
//GO(nm_setting_ip_config_remove_route_by_value, 
//GO(nm_setting_ip_tunnel_get_flags, 
//GO(nm_setting_ip_tunnel_get_input_key, 
//GO(nm_setting_ip_tunnel_get_local, 
//GO(nm_setting_ip_tunnel_get_mode, 
//GO(nm_setting_ip_tunnel_get_mtu, 
//GO(nm_setting_ip_tunnel_get_output_key, 
//GO(nm_setting_ip_tunnel_get_parent, 
//GO(nm_setting_ip_tunnel_get_path_mtu_discovery, 
//GO(nm_setting_ip_tunnel_get_remote, 
//GO(nm_setting_ip_tunnel_get_tos, 
//GO(nm_setting_ip_tunnel_get_ttl, 
//GO(nm_setting_ip_tunnel_get_type, 
//GO(nm_setting_ip_tunnel_new, 
//GO(nm_setting_lookup_type, 
//GO(nm_setting_mac_randomization_get_type, 
//GO(nm_setting_macsec_get_encrypt, 
//GO(nm_setting_macsec_get_mka_cak, 
//GO(nm_setting_macsec_get_mka_cak_flags, 
//GO(nm_setting_macsec_get_mka_ckn, 
//GO(nm_setting_macsec_get_mode, 
//GO(nm_setting_macsec_get_parent, 
//GO(nm_setting_macsec_get_port, 
//GO(nm_setting_macsec_get_send_sci, 
//GO(nm_setting_macsec_get_type, 
//GO(nm_setting_macsec_get_validation, 
//GO(nm_setting_macsec_mode_get_type, 
//GO(nm_setting_macsec_new, 
//GO(nm_setting_macsec_validation_get_type, 
//GO(nm_setting_macvlan_get_mode, 
//GO(nm_setting_macvlan_get_parent, 
//GO(nm_setting_macvlan_get_promiscuous, 
//GO(nm_setting_macvlan_get_tap, 
//GO(nm_setting_macvlan_get_type, 
//GO(nm_setting_macvlan_mode_get_type, 
//GO(nm_setting_macvlan_new, 
//GO(nm_setting_match_add_interface_name, 
//GO(nm_setting_match_clear_interface_names, 
//GO(nm_setting_match_get_interface_name, 
//GO(nm_setting_match_get_interface_names, 
//GO(nm_setting_match_get_num_interface_names, 
//GO(nm_setting_match_get_type, 
//GO(nm_setting_match_remove_interface_name, 
//GO(nm_setting_match_remove_interface_name_by_value, 
//GO(nm_setting_olpc_mesh_get_channel, 
//GO(nm_setting_olpc_mesh_get_dhcp_anycast_address, 
//GO(nm_setting_olpc_mesh_get_ssid, 
//GO(nm_setting_olpc_mesh_get_type, 
//GO(nm_setting_olpc_mesh_new, 
//GO(nm_setting_ovs_bridge_get_fail_mode, 
//GO(nm_setting_ovs_bridge_get_mcast_snooping_enable, 
//GO(nm_setting_ovs_bridge_get_rstp_enable, 
//GO(nm_setting_ovs_bridge_get_stp_enable, 
//GO(nm_setting_ovs_bridge_get_type, 
//GO(nm_setting_ovs_bridge_new, 
//GO(nm_setting_ovs_interface_get_interface_type, 
//GO(nm_setting_ovs_interface_get_type, 
//GO(nm_setting_ovs_interface_new, 
//GO(nm_setting_ovs_patch_get_peer, 
//GO(nm_setting_ovs_patch_get_type, 
//GO(nm_setting_ovs_patch_new, 
//GO(nm_setting_ovs_port_get_bond_downdelay, 
//GO(nm_setting_ovs_port_get_bond_mode, 
//GO(nm_setting_ovs_port_get_bond_updelay, 
//GO(nm_setting_ovs_port_get_lacp, 
//GO(nm_setting_ovs_port_get_tag, 
//GO(nm_setting_ovs_port_get_type, 
//GO(nm_setting_ovs_port_get_vlan_mode, 
//GO(nm_setting_ovs_port_new, 
//GO(nm_setting_ppp_get_baud, 
//GO(nm_setting_ppp_get_crtscts, 
//GO(nm_setting_ppp_get_lcp_echo_failure, 
//GO(nm_setting_ppp_get_lcp_echo_interval, 
//GO(nm_setting_ppp_get_mppe_stateful, 
//GO(nm_setting_ppp_get_mru, 
//GO(nm_setting_ppp_get_mtu, 
//GO(nm_setting_ppp_get_noauth, 
//GO(nm_setting_ppp_get_nobsdcomp, 
//GO(nm_setting_ppp_get_nodeflate, 
//GO(nm_setting_ppp_get_no_vj_comp, 
//GO(nm_setting_ppp_get_refuse_chap, 
//GO(nm_setting_ppp_get_refuse_eap, 
//GO(nm_setting_ppp_get_refuse_mschap, 
//GO(nm_setting_ppp_get_refuse_mschapv2, 
//GO(nm_setting_ppp_get_refuse_pap, 
//GO(nm_setting_ppp_get_require_mppe, 
//GO(nm_setting_ppp_get_require_mppe_128, 
//GO(nm_setting_ppp_get_type, 
//GO(nm_setting_ppp_new, 
//GO(nm_setting_pppoe_get_parent, 
//GO(nm_setting_pppoe_get_password, 
//GO(nm_setting_pppoe_get_password_flags, 
//GO(nm_setting_pppoe_get_service, 
//GO(nm_setting_pppoe_get_type, 
//GO(nm_setting_pppoe_get_username, 
//GO(nm_setting_pppoe_new, 
//GO(nm_setting_proxy_get_browser_only, 
//GO(nm_setting_proxy_get_method, 
//GO(nm_setting_proxy_get_pac_script, 
//GO(nm_setting_proxy_get_pac_url, 
//GO(nm_setting_proxy_get_type, 
//GO(nm_setting_proxy_method_get_type, 
//GO(nm_setting_proxy_new, 
//GO(nm_settings_connection_flags_get_type, 
//GO(nm_setting_secret_flags_get_type, 
//GO(nm_setting_serial_get_baud, 
//GO(nm_setting_serial_get_bits, 
//GO(nm_setting_serial_get_parity, 
//GO(nm_setting_serial_get_send_delay, 
//GO(nm_setting_serial_get_stopbits, 
//GO(nm_setting_serial_get_type, 
//GO(nm_setting_serial_new, 
//GO(nm_setting_serial_parity_get_type, 
//GO(nm_settings_error_get_type, 
//GO(nm_settings_error_quark, 
//GO(nm_setting_set_secret_flags, 
//GO(nm_setting_sriov_add_vf, 
//GO(nm_setting_sriov_clear_vfs, 
//GO(nm_setting_sriov_get_autoprobe_drivers, 
//GO(nm_setting_sriov_get_num_vfs, 
//GO(nm_setting_sriov_get_total_vfs, 
//GO(nm_setting_sriov_get_type, 
//GO(nm_setting_sriov_get_vf, 
//GO(nm_setting_sriov_new, 
//GO(nm_setting_sriov_remove_vf, 
//GO(nm_setting_sriov_remove_vf_by_index, 
//GO(nm_settings_update2_flags_get_type, 
//GO(nm_setting_tc_config_add_qdisc, 
//GO(nm_setting_tc_config_add_tfilter, 
//GO(nm_setting_tc_config_clear_qdiscs, 
//GO(nm_setting_tc_config_clear_tfilters, 
//GO(nm_setting_tc_config_get_num_qdiscs, 
//GO(nm_setting_tc_config_get_num_tfilters, 
//GO(nm_setting_tc_config_get_qdisc, 
//GO(nm_setting_tc_config_get_tfilter, 
//GO(nm_setting_tc_config_get_type, 
//GO(nm_setting_tc_config_new, 
//GO(nm_setting_tc_config_remove_qdisc, 
//GO(nm_setting_tc_config_remove_qdisc_by_value, 
//GO(nm_setting_tc_config_remove_tfilter, 
//GO(nm_setting_tc_config_remove_tfilter_by_value, 
//GO(nm_setting_team_add_link_watcher, 
//GO(nm_setting_team_add_runner_tx_hash, 
//GO(nm_setting_team_clear_link_watchers, 
//GO(nm_setting_team_get_config, 
//GO(nm_setting_team_get_link_watcher, 
//GO(nm_setting_team_get_mcast_rejoin_count, 
//GO(nm_setting_team_get_mcast_rejoin_interval, 
//GO(nm_setting_team_get_notify_peers_count, 
//GO(nm_setting_team_get_notify_peers_interval, 
//GO(nm_setting_team_get_num_link_watchers, 
//GO(nm_setting_team_get_num_runner_tx_hash, 
//GO(nm_setting_team_get_runner, 
//GO(nm_setting_team_get_runner_active, 
//GO(nm_setting_team_get_runner_agg_select_policy, 
//GO(nm_setting_team_get_runner_fast_rate, 
//GO(nm_setting_team_get_runner_hwaddr_policy, 
//GO(nm_setting_team_get_runner_min_ports, 
//GO(nm_setting_team_get_runner_sys_prio, 
//GO(nm_setting_team_get_runner_tx_balancer, 
//GO(nm_setting_team_get_runner_tx_balancer_interval, 
//GO(nm_setting_team_get_runner_tx_hash, 
//GO(nm_setting_team_get_type, 
//GO(nm_setting_team_new, 
//GO(nm_setting_team_port_add_link_watcher, 
//GO(nm_setting_team_port_clear_link_watchers, 
//GO(nm_setting_team_port_get_config, 
//GO(nm_setting_team_port_get_lacp_key, 
//GO(nm_setting_team_port_get_lacp_prio, 
//GO(nm_setting_team_port_get_link_watcher, 
//GO(nm_setting_team_port_get_num_link_watchers, 
//GO(nm_setting_team_port_get_prio, 
//GO(nm_setting_team_port_get_queue_id, 
//GO(nm_setting_team_port_get_sticky, 
//GO(nm_setting_team_port_get_type, 
//GO(nm_setting_team_port_new, 
//GO(nm_setting_team_port_remove_link_watcher, 
//GO(nm_setting_team_port_remove_link_watcher_by_value, 
//GO(nm_setting_team_remove_link_watcher, 
//GO(nm_setting_team_remove_link_watcher_by_value, 
//GO(nm_setting_team_remove_runner_tx_hash, 
//GO(nm_setting_team_remove_runner_tx_hash_by_value, 
//GO(nm_setting_to_string, 
//GO(nm_setting_tun_get_group, 
//GO(nm_setting_tun_get_mode, 
//GO(nm_setting_tun_get_multi_queue, 
//GO(nm_setting_tun_get_owner, 
//GO(nm_setting_tun_get_pi, 
//GO(nm_setting_tun_get_type, 
//GO(nm_setting_tun_get_vnet_hdr, 
//GO(nm_setting_tun_mode_get_type, 
//GO(nm_setting_tun_new, 
//GO(nm_setting_user_check_key, 
//GO(nm_setting_user_check_val, 
//GO(nm_setting_user_get_data, 
//GO(nm_setting_user_get_keys, 
//GO(nm_setting_user_get_type, 
//GO(nm_setting_user_new, 
//GO(nm_setting_user_set_data, 
//GO(nm_setting_verify, 
//GO(nm_setting_verify_secrets, 
//GO(nm_setting_vlan_add_priority, 
//GO(nm_setting_vlan_add_priority_str, 
//GO(nm_setting_vlan_clear_priorities, 
//GO(nm_setting_vlan_get_flags, 
//GO(nm_setting_vlan_get_id, 
//GO(nm_setting_vlan_get_num_priorities, 
//GO(nm_setting_vlan_get_parent, 
//GO(nm_setting_vlan_get_priority, 
//GO(nm_setting_vlan_get_type, 
//GO(nm_setting_vlan_new, 
//GO(nm_setting_vlan_remove_priority, 
//GO(nm_setting_vlan_remove_priority_by_value, 
//GO(nm_setting_vlan_remove_priority_str_by_value, 
//GO(nm_setting_vpn_add_data_item, 
//GO(nm_setting_vpn_add_secret, 
//GO(nm_setting_vpn_foreach_data_item, 
//GO(nm_setting_vpn_foreach_secret, 
//GO(nm_setting_vpn_get_data_item, 
//GO(nm_setting_vpn_get_data_keys, 
//GO(nm_setting_vpn_get_num_data_items, 
//GO(nm_setting_vpn_get_num_secrets, 
//GO(nm_setting_vpn_get_secret, 
//GO(nm_setting_vpn_get_secret_keys, 
//GO(nm_setting_vpn_get_service_type, 
//GO(nm_setting_vpn_get_timeout, 
//GO(nm_setting_vpn_get_type, 
//GO(nm_setting_vpn_get_user_name, 
//GO(nm_setting_vpn_new, 
//GO(nm_setting_vpn_remove_data_item, 
//GO(nm_setting_vpn_remove_secret, 
//GO(nm_setting_vxlan_get_ageing, 
//GO(nm_setting_vxlan_get_destination_port, 
//GO(nm_setting_vxlan_get_id, 
//GO(nm_setting_vxlan_get_l2_miss, 
//GO(nm_setting_vxlan_get_l3_miss, 
//GO(nm_setting_vxlan_get_learning, 
//GO(nm_setting_vxlan_get_limit, 
//GO(nm_setting_vxlan_get_local, 
//GO(nm_setting_vxlan_get_parent, 
//GO(nm_setting_vxlan_get_proxy, 
//GO(nm_setting_vxlan_get_remote, 
//GO(nm_setting_vxlan_get_rsc, 
//GO(nm_setting_vxlan_get_source_port_max, 
//GO(nm_setting_vxlan_get_source_port_min, 
//GO(nm_setting_vxlan_get_tos, 
//GO(nm_setting_vxlan_get_ttl, 
//GO(nm_setting_vxlan_get_type, 
//GO(nm_setting_vxlan_new, 
//GO(nm_setting_wimax_get_mac_address, 
//GO(nm_setting_wimax_get_network_name, 
//GO(nm_setting_wimax_get_type, 
//GO(nm_setting_wimax_new, 
//GO(nm_setting_wired_add_mac_blacklist_item, 
//GO(nm_setting_wired_add_s390_option, 
//GO(nm_setting_wired_clear_mac_blacklist_items, 
//GO(nm_setting_wired_get_auto_negotiate, 
//GO(nm_setting_wired_get_cloned_mac_address, 
//GO(nm_setting_wired_get_duplex, 
//GO(nm_setting_wired_get_generate_mac_address_mask, 
//GO(nm_setting_wired_get_mac_address, 
//GO(nm_setting_wired_get_mac_address_blacklist, 
//GO(nm_setting_wired_get_mac_blacklist_item, 
//GO(nm_setting_wired_get_mtu, 
//GO(nm_setting_wired_get_num_mac_blacklist_items, 
//GO(nm_setting_wired_get_num_s390_options, 
//GO(nm_setting_wired_get_port, 
//GO(nm_setting_wired_get_s390_nettype, 
//GO(nm_setting_wired_get_s390_option, 
//GO(nm_setting_wired_get_s390_option_by_key, 
//GO(nm_setting_wired_get_s390_subchannels, 
//GO(nm_setting_wired_get_speed, 
//GO(nm_setting_wired_get_type, 
//GO(nm_setting_wired_get_valid_s390_options, 
//GO(nm_setting_wired_get_wake_on_lan, 
//GO(nm_setting_wired_get_wake_on_lan_password, 
GO(nm_setting_wired_new, pFv)
//GO(nm_setting_wired_remove_mac_blacklist_item, 
//GO(nm_setting_wired_remove_mac_blacklist_item_by_value, 
//GO(nm_setting_wired_remove_s390_option, 
//GO(nm_setting_wired_wake_on_lan_get_type, 
//GO(nm_setting_wireless_add_mac_blacklist_item, 
//GO(nm_setting_wireless_add_seen_bssid, 
//GO(nm_setting_wireless_ap_security_compatible, 
//GO(nm_setting_wireless_clear_mac_blacklist_items, 
//GO(nm_setting_wireless_get_band, 
//GO(nm_setting_wireless_get_bssid, 
//GO(nm_setting_wireless_get_channel, 
//GO(nm_setting_wireless_get_cloned_mac_address, 
//GO(nm_setting_wireless_get_generate_mac_address_mask, 
//GO(nm_setting_wireless_get_hidden, 
//GO(nm_setting_wireless_get_mac_address, 
//GO(nm_setting_wireless_get_mac_address_blacklist, 
//GO(nm_setting_wireless_get_mac_address_randomization, 
//GO(nm_setting_wireless_get_mac_blacklist_item, 
//GO(nm_setting_wireless_get_mode, 
//GO(nm_setting_wireless_get_mtu, 
//GO(nm_setting_wireless_get_num_mac_blacklist_items, 
//GO(nm_setting_wireless_get_num_seen_bssids, 
//GO(nm_setting_wireless_get_powersave, 
//GO(nm_setting_wireless_get_rate, 
//GO(nm_setting_wireless_get_seen_bssid, 
//GO(nm_setting_wireless_get_ssid, 
//GO(nm_setting_wireless_get_tx_power, 
//GO(nm_setting_wireless_get_type, 
//GO(nm_setting_wireless_get_wake_on_wlan, 
GO(nm_setting_wireless_new, pFv)
//GO(nm_setting_wireless_powersave_get_type, 
//GO(nm_setting_wireless_remove_mac_blacklist_item, 
//GO(nm_setting_wireless_remove_mac_blacklist_item_by_value, 
GO(nm_setting_wireless_security_add_group, iFpp)
GO(nm_setting_wireless_security_add_pairwise, iFpp)
GO(nm_setting_wireless_security_add_proto, iFpp)
GO(nm_setting_wireless_security_clear_groups, vFp)
GO(nm_setting_wireless_security_clear_pairwise, vFp)
GO(nm_setting_wireless_security_clear_protos, vFp)
GO(nm_setting_wireless_security_fils_get_type, iFv)
GO(nm_setting_wireless_security_get_auth_alg, pFp)
GO(nm_setting_wireless_security_get_fils, iFp)
GO(nm_setting_wireless_security_get_group, pFpu)
GO(nm_setting_wireless_security_get_key_mgmt, pFp)
GO(nm_setting_wireless_security_get_leap_password, pFp)
GO(nm_setting_wireless_security_get_leap_password_flags, iFp)
GO(nm_setting_wireless_security_get_leap_username, pFp)
GO(nm_setting_wireless_security_get_num_groups, uFp)
GO(nm_setting_wireless_security_get_num_pairwise, uFp)
GO(nm_setting_wireless_security_get_num_protos, uFp)
GO(nm_setting_wireless_security_get_pairwise, pFpu)
GO(nm_setting_wireless_security_get_pmf, iFp)
GO(nm_setting_wireless_security_get_proto, pFpu)
GO(nm_setting_wireless_security_get_psk, pFp)
GO(nm_setting_wireless_security_get_psk_flags, iFp)
GO(nm_setting_wireless_security_get_type, iFv)
GO(nm_setting_wireless_security_get_wep_key, pFpu)
GO(nm_setting_wireless_security_get_wep_key_flags, iFp)
GO(nm_setting_wireless_security_get_wep_key_type, iFp)
GO(nm_setting_wireless_security_get_wep_tx_keyidx, uFp)
GO(nm_setting_wireless_security_get_wps_method, iFp)
GO(nm_setting_wireless_security_new, pFv)
GO(nm_setting_wireless_security_pmf_get_type, iFv)
GO(nm_setting_wireless_security_remove_group, vFpu)
GO(nm_setting_wireless_security_remove_group_by_value, iFpp)
GO(nm_setting_wireless_security_remove_pairwise, vFpu)
GO(nm_setting_wireless_security_remove_pairwise_by_value, iFpp)
GO(nm_setting_wireless_security_remove_proto, vFpu)
GO(nm_setting_wireless_security_remove_proto_by_value, iFpp)
GO(nm_setting_wireless_security_set_wep_key, vFpup)
//GO(nm_setting_wireless_security_wps_method_get_type, 
//GO(nm_setting_wireless_wake_on_wlan_get_type, 
//GO(nm_setting_wpan_get_type, 
//GO(nm_simple_connection_get_type, 
GO(nm_simple_connection_new, pFv)
//GO(nm_simple_connection_new_clone, 
//GO(nm_simple_connection_new_from_dbus, 
//GO(nm_sriov_vf_add_vlan, 
//GO(nm_sriov_vf_dup, 
//GO(nm_sriov_vf_equal, 
//GO(nm_sriov_vf_get_attribute, 
//GO(nm_sriov_vf_get_attribute_names, 
//GO(nm_sriov_vf_get_index, 
//GO(nm_sriov_vf_get_type, 
//GO(nm_sriov_vf_get_vlan_ids, 
//GO(nm_sriov_vf_get_vlan_protocol, 
//GO(nm_sriov_vf_get_vlan_qos, 
//GO(nm_sriov_vf_new, 
//GO(nm_sriov_vf_ref, 
//GO(nm_sriov_vf_remove_vlan, 
//GO(nm_sriov_vf_set_attribute, 
//GO(nm_sriov_vf_set_vlan_protocol, 
//GO(nm_sriov_vf_set_vlan_qos, 
//GO(nm_sriov_vf_unref, 
//GO(nm_sriov_vf_vlan_protocol_get_type, 
//GO(nm_state_get_type, 
//GO(nm_tc_action_dup, 
//GO(nm_tc_action_equal, 
//GO(nm_tc_action_get_attribute, 
//GO(nm_tc_action_get_attribute_names, 
//GO(nm_tc_action_get_kind, 
//GO(nm_tc_action_get_type, 
//GO(nm_tc_action_new, 
//GO(nm_tc_action_ref, 
//GO(nm_tc_action_set_attribute, 
//GO(nm_tc_action_unref, 
//GO(nm_tc_qdisc_dup, 
//GO(nm_tc_qdisc_equal, 
//GO(nm_tc_qdisc_get_handle, 
//GO(nm_tc_qdisc_get_kind, 
//GO(nm_tc_qdisc_get_parent, 
//GO(nm_tc_qdisc_get_type, 
//GO(nm_tc_qdisc_new, 
//GO(nm_tc_qdisc_ref, 
//GO(nm_tc_qdisc_set_handle, 
//GO(nm_tc_qdisc_unref, 
//GO(nm_tc_tfilter_dup, 
//GO(nm_tc_tfilter_equal, 
//GO(nm_tc_tfilter_get_handle, 
//GO(nm_tc_tfilter_get_kind, 
//GO(nm_tc_tfilter_get_parent, 
//GO(nm_tc_tfilter_get_type, 
//GO(nm_tc_tfilter_new, 
//GO(nm_tc_tfilter_ref, 
//GO(nm_tc_tfilter_set_handle, 
//GO(nm_tc_tfilter_unref, 
//GO(nm_team_link_watcher_arp_ping_flags_get_type, 
//GO(nm_team_link_watcher_dup, 
//GO(nm_team_link_watcher_equal, 
//GO(nm_team_link_watcher_get_delay_down, 
//GO(nm_team_link_watcher_get_delay_up, 
//GO(nm_team_link_watcher_get_flags, 
//GO(nm_team_link_watcher_get_init_wait, 
//GO(nm_team_link_watcher_get_interval, 
//GO(nm_team_link_watcher_get_missed_max, 
//GO(nm_team_link_watcher_get_name, 
//GO(nm_team_link_watcher_get_source_host, 
//GO(nm_team_link_watcher_get_target_host, 
//GO(nm_team_link_watcher_get_type, 
//GO(nm_team_link_watcher_new_arp_ping, 
//GO(nm_team_link_watcher_new_ethtool, 
//GO(nm_team_link_watcher_new_nsna_ping, 
//GO(nm_team_link_watcher_ref, 
//GO(nm_team_link_watcher_unref, 
//GO(nm_ternary_get_type, 
//GO(nm_utils_ap_mode_security_valid, 
//GO(nm_utils_bin2hexstr, 
//GO(nm_utils_bond_mode_int_to_string, 
//GO(nm_utils_bond_mode_string_to_int, 
//GO(nm_utils_check_virtual_device_compatibility, 
//GO(nm_utils_enum_from_str, 
//GO(nm_utils_enum_get_values, 
//GO(nm_utils_enum_to_str, 
//GO(nm_utils_escape_ssid, 
//GO(nm_utils_file_is_certificate, 
//GO(nm_utils_file_is_pkcs12, 
//GO(nm_utils_file_is_private_key, 
//GO(nm_utils_file_search_in_paths, 
//GO(nm_utils_format_variant_attributes, 
//GO(nm_utils_get_timestamp_msec, 
//GO(nm_utils_hexstr2bin, 
//GO(nm_utils_hwaddr_atoba, 
//GO(nm_utils_hwaddr_aton, 
//GO(nm_utils_hwaddr_canonical, 
//GO(nm_utils_hwaddr_len, 
//GO(nm_utils_hwaddr_matches, 
//GO(nm_utils_hwaddr_ntoa, 
//GO(nm_utils_hwaddr_valid, 
//GO(nm_utils_iface_valid_name, 
//GO(nm_utils_inet4_ntop, 
//GO(nm_utils_inet6_ntop, 
//GO(nm_utils_ip4_addresses_from_variant, 
//GO(nm_utils_ip4_addresses_to_variant, 
//GO(nm_utils_ip4_dns_from_variant, 
//GO(nm_utils_ip4_dns_to_variant, 
//GO(nm_utils_ip4_get_default_prefix, 
//GO(nm_utils_ip4_netmask_to_prefix, 
//GO(nm_utils_ip4_prefix_to_netmask, 
//GO(nm_utils_ip4_routes_from_variant, 
//GO(nm_utils_ip4_routes_to_variant, 
//GO(nm_utils_ip6_addresses_from_variant, 
//GO(nm_utils_ip6_addresses_to_variant, 
//GO(nm_utils_ip6_dns_from_variant, 
//GO(nm_utils_ip6_dns_to_variant, 
//GO(nm_utils_ip6_routes_from_variant, 
//GO(nm_utils_ip6_routes_to_variant, 
//GO(nm_utils_ipaddr_valid, 
//GO(nm_utils_is_empty_ssid, 
//GO(nm_utils_is_json_object, 
//GO(nm_utils_is_uuid, 
//GO(nm_utils_is_valid_iface_name, 
//GO(nm_utils_parse_variant_attributes, 
//GO(nm_utils_same_ssid, 
//GO(nm_utils_security_type_get_type, 
GO(nm_utils_security_valid, iFiiiiiii)
//GO(nm_utils_sriov_vf_from_str, 
//GO(nm_utils_sriov_vf_to_str, 
GO(nm_utils_ssid_to_utf8, pFpL)
//GO(nm_utils_tc_action_from_str, 
//GO(nm_utils_tc_action_to_str, 
//GO(nm_utils_tc_qdisc_from_str, 
//GO(nm_utils_tc_qdisc_to_str, 
//GO(nm_utils_tc_tfilter_from_str, 
//GO(nm_utils_tc_tfilter_to_str, 
GO(nm_utils_uuid_generate, pFv)
//GO(nm_utils_version, 
//GO(nm_utils_wep_key_valid, 
//GO(nm_utils_wifi_2ghz_freqs, 
//GO(nm_utils_wifi_5ghz_freqs, 
//GO(nm_utils_wifi_channel_to_freq, 
//GO(nm_utils_wifi_find_next_channel, 
//GO(nm_utils_wifi_freq_to_channel, 
//GO(nm_utils_wifi_is_channel_valid, 
//GO(nm_utils_wifi_strength_bars, 
//GO(nm_utils_wpa_psk_valid, 
//GO(nm_vlan_flags_get_type, 
//GO(nm_vlan_priority_map_get_type, 
//GO(nm_vpn_connection_get_banner, 
//GO(nm_vpn_connection_get_type, 
//GO(nm_vpn_connection_get_vpn_state, 
//GO(nm_vpn_connection_state_get_type, 
//GO(nm_vpn_connection_state_reason_get_type, 
//GO(nm_vpn_editor_get_type, 
//GO(nm_vpn_editor_get_widget, 
//GO(nm_vpn_editor_plugin_capability_get_type, 
//GO(nm_vpn_editor_plugin_export, 
//GO(nm_vpn_editor_plugin_get_capabilities, 
//GO(nm_vpn_editor_plugin_get_editor, 
//GO(nm_vpn_editor_plugin_get_plugin_info, 
//GO(nm_vpn_editor_plugin_get_suggested_filename, 
//GO(nm_vpn_editor_plugin_get_type, 
//GO(nm_vpn_editor_plugin_get_vt, 
//GO(nm_vpn_editor_plugin_import, 
//GO(nm_vpn_editor_plugin_load, 
//GO(nm_vpn_editor_plugin_load_from_file, 
//GO(nm_vpn_editor_plugin_set_plugin_info, 
//GO(nm_vpn_editor_update_connection, 
//GO(nm_vpn_plugin_error_get_type, 
//GO(nm_vpn_plugin_error_quark, 
//GO(nm_vpn_plugin_failure_get_type, 
//GO(nm_vpn_plugin_info_get_aliases, 
//GO(nm_vpn_plugin_info_get_auth_dialog, 
//GO(nm_vpn_plugin_info_get_editor_plugin, 
//GO(nm_vpn_plugin_info_get_filename, 
//GO(nm_vpn_plugin_info_get_name, 
//GO(nm_vpn_plugin_info_get_plugin, 
//GO(nm_vpn_plugin_info_get_program, 
//GO(nm_vpn_plugin_info_get_service, 
//GO(nm_vpn_plugin_info_get_type, 
//GO(nm_vpn_plugin_info_list_add, 
//GO(nm_vpn_plugin_info_list_find_by_filename, 
//GO(nm_vpn_plugin_info_list_find_by_name, 
//GO(nm_vpn_plugin_info_list_find_by_service, 
//GO(nm_vpn_plugin_info_list_find_service_type, 
//GO(nm_vpn_plugin_info_list_get_service_types, 
//GO(nm_vpn_plugin_info_list_load, 
//GO(nm_vpn_plugin_info_list_remove, 
//GO(nm_vpn_plugin_info_load_editor_plugin, 
//GO(nm_vpn_plugin_info_lookup_property, 
//GO(nm_vpn_plugin_info_new_from_file, 
//GO(nm_vpn_plugin_info_new_search_file, 
//GO(nm_vpn_plugin_info_new_with_data, 
//GO(nm_vpn_plugin_info_set_editor_plugin, 
//GO(nm_vpn_plugin_info_supports_hints, 
//GO(nm_vpn_plugin_info_validate_filename, 
//GO(nm_vpn_plugin_old_disconnect, 
//GO(nm_vpn_plugin_old_failure, 
//GO(nm_vpn_plugin_old_get_connection, 
//GO(nm_vpn_plugin_old_get_secret_flags, 
//GO(nm_vpn_plugin_old_get_state, 
//GO(nm_vpn_plugin_old_get_type, 
//GO(nm_vpn_plugin_old_read_vpn_details, 
//GO(nm_vpn_plugin_old_secrets_required, 
//GO(nm_vpn_plugin_old_set_ip4_config, 
//GO(nm_vpn_plugin_old_set_login_banner, 
//GO(nm_vpn_plugin_old_set_state, 
//GO(nm_vpn_service_plugin_disconnect, 
//GO(nm_vpn_service_plugin_failure, 
//GO(nm_vpn_service_plugin_get_connection, 
//GO(nm_vpn_service_plugin_get_secret_flags, 
//GO(nm_vpn_service_plugin_get_type, 
//GO(nm_vpn_service_plugin_read_vpn_details, 
//GO(nm_vpn_service_plugin_secrets_required, 
//GO(nm_vpn_service_plugin_set_config, 
//GO(nm_vpn_service_plugin_set_ip4_config, 
//GO(nm_vpn_service_plugin_set_ip6_config, 
//GO(nm_vpn_service_plugin_set_login_banner, 
//GO(nm_vpn_service_plugin_shutdown, 
//GO(nm_vpn_service_state_get_type, 
//GO(nm_wep_key_type_get_type, 
//GO(nm_wimax_nsp_connection_valid, 
//GO(nm_wimax_nsp_filter_connections, 
//GO(nm_wimax_nsp_get_name, 
//GO(nm_wimax_nsp_get_network_type, 
//GO(nm_wimax_nsp_get_signal_quality, 
//GO(nm_wimax_nsp_get_type, 
//GO(nm_wimax_nsp_network_type_get_type, 
