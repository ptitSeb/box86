#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(bpf_dump, vFpi)
GO(bpf_filter, uFppuu)
GO(bpf_image, pFpi)
GO(bpf_validate, iFpi)
//DATA(eproto_db, 
GO(pcap_activate, iFp)
GO(pcap_breakloop, vFp)
//GO(pcap_bufsize, 
GO(pcap_can_set_rfmon, iFp)
GO(pcap_close, vFp)
GO(pcap_compile, iFpppiu)
//GO(pcap_compile_nopcap, 
GO(pcap_create, pFpp)
GO(pcap_datalink, iFp)
//GO(pcap_datalink_ext, 
GO(pcap_datalink_name_to_val, iFp)
GO(pcap_datalink_val_to_description, pFi)
//GO(pcap_datalink_val_to_description_or_dlt, 
GO(pcap_datalink_val_to_name, pFi)
//GO(pcap_dispatch, 
//GO(pcap_dump, 
//GO(pcap_dump_close, 
//GO(pcap_dump_file, 
//GO(pcap_dump_flush, 
//GO(pcap_dump_fopen, 
//GO(pcap_dump_ftell, 
//GO(pcap_dump_ftell64, 
//GO(pcap_dump_open, 
//GO(pcap_dump_open_append, 
//GO(pcap_ether_aton, 
//GO(pcap_ether_hostton, 
//GO(pcap_file, 
//GO(pcap_fileno, 
GO(pcap_findalldevs, iFpp)
GO(pcap_fopen_offline, pFpp)
GO(pcap_fopen_offline_with_tstamp_precision, pFpup)
GO(pcap_freealldevs, vFp)
GO(pcap_freecode, vFp)
GO(pcap_free_datalinks, vFp)
GO(pcap_free_tstamp_types, vFp)
GO(pcap_geterr, pFp)
GO(pcap_getnonblock, iFpp)
//GO(pcap_get_required_select_timeout, 
GO(pcap_get_selectable_fd, iFp)
GO(pcap_get_tstamp_precision, iFp)
//GO(pcap_init, 
//GO(pcap_inject, 
//GO(pcap_is_swapped, 
GO(pcap_lib_version, pFv)
GO(pcap_list_datalinks, iFpp)
GO(pcap_list_tstamp_types, iFpp)
//GO(pcap_lookupdev, 
GO(pcap_lookupnet, iFpppp)
//GO(pcap_loop, 
//GO(pcap_major_version, 
//GO(pcap_minor_version, 
//GO(pcap_nametoaddr, 
//GO(pcap_nametoaddrinfo, 
//GO(pcap_nametoeproto, 
//GO(pcap_nametollc, 
//GO(pcap_nametonetaddr, 
//GO(pcap_nametoport, 
//GO(pcap_nametoportrange, 
//GO(pcap_nametoproto, 
GO(pcap_next, pFpp)
//GO(pcap_next_etherent, 
GO(pcap_next_ex, iFppp)
GO(pcap_offline_filter, iFppp)
GO(pcap_open_dead, pFii)
//GO(pcap_open_dead_with_tstamp_precision, 
GO(pcap_open_live, pFpiiip)
GO(pcap_open_offline, pFpp)
GO(pcap_open_offline_with_tstamp_precision, pFpup)
//GO(pcap_perror, 
GO(pcap_sendpacket, iFppi)
GO(pcap_set_buffer_size, iFpi)
GO(pcap_set_datalink, iFpi)
GO(pcap_setdirection, iFpi)
GO(pcap_setfilter, iFpp)
GO(pcap_set_immediate_mode, iFpi)
GO(pcap_setnonblock, pFpip)
GO(pcap_set_promisc, iFpi)
//GO(pcap_set_protocol_linux, 
GO(pcap_set_rfmon, iFpi)
GO(pcap_set_snaplen, iFpi)
GO(pcap_set_timeout, iFpi)
GO(pcap_set_tstamp_precision, iFpi)
GO(pcap_set_tstamp_type, iFpi)
GO(pcap_snapshot, iFp)
GO(pcap_stats, iFpp)
GO(pcap_statustostr, pFi)
//GO(pcap_strerror, 
GO(pcap_tstamp_type_name_to_val, iFp)
GO(pcap_tstamp_type_val_to_description, pFi)
GO(pcap_tstamp_type_val_to_name, pFi)
//DATA(pcap_version, 
