#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(__b64_ntop, iFpupu)
GO(__b64_pton, iFppu)
GO(__dn_comp, iFppipp)
GO(__dn_count_labels, iFp)
GO(__dn_expand, iFppppi)
GOW(dn_expand, iFppppi)
GO(__dn_skipname, iFpp)
GO(__fp_nquery, vFpip)
GO(__fp_query, vFpp)
//GO(__fp_resstat
//GO(_gethtbyaddr
//GO(_gethtbyname
//GO(_gethtbyname2
//GO(_gethtent
//GO(_getlong
//GO(_getshort
GO(__hostalias, pFp)
GO(inet_neta, pFupu)
GO(inet_net_ntop, pFipipu)
GO(inet_net_pton, iFippu)
GO(__loc_aton, iFpp)
GO(__loc_ntoa, pFpp)
GO(ns_datetosecs, uFpp)
//GO(ns_format_ttl
GO(__ns_get16, uFp)
GOW(ns_get16, uFp)
GO(__ns_get32, uFp)
GOW(ns_get32, uFp)
GO(ns_initparse, iFpip)
//GO(ns_makecanon
//GO(ns_msg_getflag
GO(ns_name_compress, iFppupp)
GO(ns_name_ntol, iFppu)
GO(__ns_name_ntop, iFppu)
GOW(ns_name_ntop,iFppu)
GO(ns_name_pack, iFppipp)
GO(ns_name_pton, iFppu)
GO(ns_name_rollback, iFpp)
GO(ns_name_skip, iFpp)
GO(ns_name_uncompress, iFppppu)
GO(__ns_name_unpack, iFppppu)
GOW(ns_name_unpack, iFppppu)
GO(ns_parserr, iFpiip)
//GO(ns_parse_ttl
GO(ns_put16, vFup)
GO(ns_put32, vFup)
//GO(ns_samedomain
//GO(ns_samename
GO(ns_skiprr, iFppii)
//GO(ns_sprintrr
//GO(ns_sprintrrf
//GO(ns_subdomain
GO(__p_cdname, pFppp)
GO(__p_cdnname, pFppip)
GO(__p_class, pFi)
//DATA(__p_class_syms, 4)
GO(__p_fqname, pFppp)
GO(__p_fqnname, pFppipi)
GO(__p_option, pFu)
GO(__p_query, vFp)
GO(__p_rcode, pFi)
//GO(__p_secstodate
GO(__p_time, pFu)
GO(__p_type, pFi)
//DATA(__p_type_syms, 4)
GO(__putlong, vFup)
GO(__putshort, vFWp)
GO(__res_close, vFv)
//GO(__res_context_hostalias
//GO(__res_context_query
//GO(__res_context_search
GO(__res_dnok, iFp)
//GO(res_gethostbyaddr
//GO(res_gethostbyname
//GO(res_gethostbyname2
GO(__res_hnok, iFp)
//GO(__res_hostalias
GO(__res_isourserver, iFp)
GO(__res_mailok, iFp)
GO(__res_mkquery, iFipiipippi)
GOW(res_mkquery, iFipiipippi)
GO(__res_nameinquery, iFpiipp)
//GO(__res_nmkquery
//GO(__res_nquery
//GO(__res_nquerydomain
GO(__res_nsearch, iFppiipi)
//GO(__res_nsend
//DATA(_res_opcodes, 4)
GO(__res_ownok, iFp)
GO(__res_queriesmatch, iFpppp)
GO(__res_query, iFpiipi)
GOW(res_query, iFpiipi)
GO(__res_querydomain, iFppiipi)
GOW(res_querydomain, iFppiipi)
GO(__res_search, iFpiipi)
GOW(res_search, iFpiipi)
GO(__res_send, iFpipi)
//GO(res_send_setqhook
//GO(res_send_setrhook
//GO(_sethtent
GO(__sym_ntop, pFpip)
GO(__sym_ntos, pFpip)
GO(__sym_ston, iFppp)
