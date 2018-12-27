#if defined(GO) && defined(GOM) && defined(GO2) && defined(DATA) && defined(END)

//clockid_t is uint32?
// pid_t is uint32 too? (for clock_getcpuclockid)
// time_t is uint32?
GO(asctime, pFp)
GO(asctime_r, pFpp)
GO(clock, uFv)
GO(clock_getcpuclockid, iFup)
GO(clock_getres, iFup)
GO(clock_gettime, iFup)
GO(clock_nanosleep, iFuipp)
GO(clock_settime, iFup)
GO(ctime, pFp)
GO(ctime_r, pFpp)
//GO(difftime, dFuu)    // return a double. The double is in ST(0)!
GO(getdate, pFp)
GO(gmtime, pFp)
GO(gmtime_r, pFpp)
GO(localtime, pFp)
GO(localtime_r, pFpp)
GO(mktime, uFp)
GO(nanosleep, iFpp)
GO(strftime, uFpupp)
GO(strptime, pFppp)
GO(time, uFp)
GO(timer_create, iFpp)
GO(timer_delete, iFu)
GO(timer_gettime, iFup)
GO(timer_getoverrun, iFu)
GO(timer_settime, iFuipp)
GO(tzset, vFv)
DATA(daylight, 4)
DATA(timezone, 4)
DATA(tzname, 4)
END()

#else
#error Mmmm...
#endif