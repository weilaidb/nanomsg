#ifndef MLOG_INIT_H
#define MLOG_INIT_H
#include "memorylog.h"

//public
#define mlog_pushcommon(key, fmt,args...)\
do{\
    pushlogbyname(key,fmt,##args);\
}while(0)

//public package
#define mlog_pushcommonpackage(key, msg, msglen, fmt,args...)\
do{\
    pushmsgbyname(key,msg, msglen,fmt,##args);\
}while(0)




//按function保存数据
#define mlog_byfunc(fmt,args...)\
do{\
    char key[32];\
    snprintf(key, sizeof(key), "%s", __FUNCTION__);\
    pushlogbyname(key,fmt,##args);\
}while(0)

//按function保存报文
#define mlog_msgbyfunc(msg, msglen, fmt,args...)\
do{\
    char key[32];\
    snprintf(key, sizeof(key), "%s", __FUNCTION__);\
    pushmsgbyname(key,msg, msglen,fmt,##args);\
}while(0)

	
	
	

//按function显示数据
#define showmlog_byfunc \
do{\
    char key[32];\
    snprintf(key, sizeof(key), "%s", __FUNCTION__);\
    showmlogbyname(key);\
}while(0)

//按function保存数据package
#define mlogmsg_byfunc(msg, msglen, fmt,args...)\
do{\
    char key[32];\
    snprintf(key, sizeof(key), "%s", __FUNCTION__);\
    mlog_pushcommonpackage(key,msg, msglen, fmt,##args);\
}while(0)

#if 0
//按function显示数据package
#define showmlog_byfunc \
do{\
    char key[32];\
    snprintf(key, sizeof(key), "%s", __FUNCTION__);\
    showmlogbyname(key);\
}while(0)
#endif



#endif // MLOG_INIT_H
