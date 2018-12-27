#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(END)

GO(pthread_self, uFv)
GOM(pthread_create, iFEpppp)
GO(pthread_equal, iFuu)
GO(pthread_join, iFup)
END()

#else
#error Mmmm...
#endif