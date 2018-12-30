#ifdef GO

GO01(p,v)
GO01(p,p)
GO01(p,u)
GO01(p,E)
GO02(p,E,p)
GO02(p,u,u)
GO02(p,i,p)
GO02(p,p,i)
GO02(p,p,p)
GO03(p,E,p,i)
GO03(p,E,p,p)
GO03(p,i,p,p)
GO03(p,p,i,u)
GO03(p,p,p,i)
GO03(p,p,p,u)
GO03(p,p,p,p)
GO04(p,E,p,p,p)
GO04(p,p,p,u,u)
GO05(p,p,i,p,p,p)
GO06(p,p,i,i,i,i,u)
GO08(p,p,p,i,i,u,u,u,i)
GO11(p,p,p,i,i,u,u,u,i,p,i,i)

#else
#error Meh
#endif
