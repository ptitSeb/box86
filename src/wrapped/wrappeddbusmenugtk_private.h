#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(dbusmenu_gtkclient_get_accel_group, pFp)
GO(dbusmenu_gtkclient_get_type, iFv)
GO(dbusmenu_gtkclient_menuitem_get, pFpp)
GO(dbusmenu_gtkclient_menuitem_get_submenu, pFpp)
GO(dbusmenu_gtkclient_new, pFpp)
GO(dbusmenu_gtkclient_newitem_base, vFpppp)
GO(dbusmenu_gtkclient_set_accel_group, vFpp)
GO(dbusmenu_gtkmenu_get_client, pFp)
GO(dbusmenu_gtkmenu_get_type, iFv)
GO(dbusmenu_gtkmenu_new, pFpp)
GO(dbusmenu_gtk_parse_get_cached_item, pFp)
GO(dbusmenu_gtk_parse_menu_structure, pFp)
GO(dbusmenu_menuitem_property_get_image, pFpp)
GO(dbusmenu_menuitem_property_get_shortcut, vFppp)
GO(dbusmenu_menuitem_property_set_image, iFppp)
GO(dbusmenu_menuitem_property_set_shortcut, iFpui)
GO(dbusmenu_menuitem_property_set_shortcut_menuitem, iFpp)
GO(dbusmenu_menuitem_property_set_shortcut_string, iFpp)
// gtk3 vvv ?
//GO(genericmenuitem_check_type_get_nick, 
//GO(genericmenuitem_check_type_get_type, 
//GO(genericmenuitem_check_type_get_value_from_nick, 
//GO(genericmenuitem_disposition_get_nick, 
//GO(genericmenuitem_disposition_get_type, 
//GO(genericmenuitem_disposition_get_value_from_nick, 
//GO(genericmenuitem_get_disposition, 
//GO(genericmenuitem_get_image, 
//GO(genericmenuitem_get_type, 
//GO(genericmenuitem_set_check_type, 
//GO(genericmenuitem_set_disposition, 
//GO(genericmenuitem_set_image, 
//GO(genericmenuitem_set_state, 
//GO(genericmenuitem_state_get_nick, 
//GO(genericmenuitem_state_get_type, 
//GO(genericmenuitem_state_get_value_from_nick, 
