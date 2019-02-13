#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(oggpack_adv, 
//GO(oggpack_adv1, 
//GO(oggpackB_adv, 
//GO(oggpackB_adv1, 
//GO(oggpackB_bits, 
//GO(oggpackB_bytes, 
//GO(oggpackB_get_buffer, 
//GO(oggpack_bits, 
//GO(oggpackB_look, 
//GO(oggpackB_look1, 
//GO(oggpackB_read, 
//GO(oggpackB_read1, 
//GO(oggpackB_readinit, 
//GO(oggpackB_reset, 
//GO(oggpackB_write, 
//GO(oggpackB_writealign, 
//GO(oggpackB_writecheck, 
//GO(oggpackB_writeclear, 
//GO(oggpackB_writecopy, 
//GO(oggpackB_writeinit, 
//GO(oggpackB_writetrunc, 
//GO(oggpack_bytes, 
//GO(ogg_packet_clear, 
//GO(oggpack_get_buffer, 
//GO(oggpack_look, 
//GO(oggpack_look1, 
//GO(oggpack_read, 
//GO(oggpack_read1, 
//GO(oggpack_readinit, 
//GO(oggpack_reset, 
//GO(oggpack_write, 
//GO(oggpack_writealign, 
//GO(oggpack_writecheck, 
//GO(oggpack_writeclear, 
//GO(oggpack_writecopy, 
//GO(oggpack_writeinit, 
//GO(oggpack_writetrunc, 
GO(ogg_page_bos, iFp)
//GO(ogg_page_checksum_set, 
//GO(ogg_page_continued, 
GO(ogg_page_eos, iFp)
GO(ogg_page_granulepos, IFp)
//GO(ogg_page_packets, 
//GO(ogg_page_pageno, 
GO(ogg_page_serialno, iFp)
//GO(ogg_page_version, 
//GO(ogg_stream_check, 
GO(ogg_stream_clear, iFp)
//GO(ogg_stream_destroy, 
//GO(ogg_stream_eos, 
//GO(ogg_stream_flush, 
//GO(ogg_stream_flush_fill, 
GO(ogg_stream_init, iFpi)
//GO(ogg_stream_iovecin, 
//GO(ogg_stream_packetin, 
GO(ogg_stream_packetout, iFpp)
//GO(ogg_stream_packetpeek, 
GO(ogg_stream_pagein, iFpp)
//GO(ogg_stream_pageout, 
//GO(ogg_stream_pageout_fill, 
//GO(ogg_stream_reset, 
//GO(ogg_stream_reset_serialno, 
GO(ogg_sync_buffer, pFpi)
//GO(ogg_sync_check, 
GO(ogg_sync_clear, iFp)
//GO(ogg_sync_destroy, 
GO(ogg_sync_init, iFp)
GO(ogg_sync_pageout, iFpp)
//GO(ogg_sync_pageseek, 
//GO(ogg_sync_reset, 
GO(ogg_sync_wrote, iFpi)
