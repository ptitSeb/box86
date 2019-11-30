#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(pa_channel_map_valid, iFp)
//GO(pa_context_connect, iFppup)
//GO(pa_context_errno, iFp)
//GO(pa_context_get_state, iFp)
//GO(pa_context_new, pFpp)
//GOM(pa_context_set_state_callback, vFEppp)
//GO(pa_context_unref, vFp)
//GOM(pa_log_level_meta, vFEipippVV)
//GO(pa_operation_cancel, vFp)
//GO(pa_operation_get_state, iFp)
//GO(pa_operation_unref, vFp)
//GO(pa_sample_spec_valid, iFp)
GO(pa_simple_drain, iFpp)
GO(pa_simple_flush, iFpp)
GO(pa_simple_free, vFp)
GO(pa_simple_get_latency, UFpp)
GO(pa_simple_new, pFppipppppp)
GO(pa_simple_read, iFppip)
GO(pa_simple_write, iFppip)
//GO(pa_stream_connect_playback, iFpppipp)
//GO(pa_stream_connect_record, iFpppi)
//GOM(pa_stream_drain, pFEppp)
//GO(pa_stream_drop, iFp)
//GOM(pa_stream_flush, pFEppp)
//GO(pa_stream_get_latency, iFppp)
//GO(pa_stream_get_state, iFp)
//GO(pa_stream_new, pFpppp)
//GO(pa_stream_peek, iFppp)
//GOM(pa_stream_set_latency_update_callback, vFEppp)
//GOM(pa_stream_set_read_callback, vFEppp)
//GOM(pa_stream_set_state_callback, vFEppp)
//GOM(pa_stream_set_write_callback, vFEppp)
//GO(pa_stream_unref, 
//GO(pa_stream_writable_size, 
//GO(pa_stream_write, 
//GO(pa_threaded_mainloop_free, 
//GO(pa_threaded_mainloop_get_api, 
//GO(pa_threaded_mainloop_lock, 
//GO(pa_threaded_mainloop_new, 
//GO(pa_threaded_mainloop_signal, 
//GO(pa_threaded_mainloop_start, 
//GO(pa_threaded_mainloop_stop, 
//GO(pa_threaded_mainloop_unlock, 
//GO(pa_threaded_mainloop_wait, 
//GO(pa_xfree, 
//GO(pa_xmalloc, 
// this one seems to comes from the "full" pulseaudio lib
GO(pa_strerror, pFi)
GO(pa_mainloop_free, vFp)
GO(pa_mainloop_get_api, pFp)
GO(pa_mainloop_new, pFv)
GO(pa_mainloop_prepare, iFpi)
GO(pa_mainloop_poll, iFp)
GO(pa_mainloop_dispatch, iFp)
GO(pa_mainloop_get_retval, iFp)
GO(pa_mainloop_iterate, iFpip)
GO(pa_mainloop_run, iFpp)
GO(pa_mainloop_quit, vFpi)
GO(pa_mainloop_wakeup, vFp)
//GO(pa_mainloop_set_poll_func, vFpBp)  //B is typedef int(*pa_poll_func) (struct pollfd *ufds, unsigned long nfds, int timeout, void *userdata)
