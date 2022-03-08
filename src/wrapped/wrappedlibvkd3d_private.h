#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif
GO(vkd3d_instance_get_vk_instance, pFp)
GOM(vkd3d_create_instance, iFEpp)
GO(vkd3d_create_device, iFppp)
GO(vkd3d_instance_decref, uFp)
GO(vkd3d_create_root_signature_deserializer, iFpupp)
GO(vkd3d_serialize_root_signature, iFpipp)
