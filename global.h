#ifndef GLOBAL
#define GLOBAL



#define DEBUG
#ifdef DEBUG
//#define printMsg(format,...)  fprintf(stderr," [%s, %s(), %d]: "format"  \n", __FILE__, __FUNCTION__, __LINE__,##__VA_ARGS__);
#define printMsg(format,...)  fprintf(stderr," [%s(), %d]: "format"  \n", __FUNCTION__, __LINE__,##__VA_ARGS__);

#else
#define printMsg(...)
#endif


#define THREAD_BUSY_PERCENT (0.1)
#define MAX_ACTIVE_THREADS  (1024)
#define MAX_QUEUE_NUM       (6553)

#include <string>
typedef struct key_value {
    std::string key;
    std::string value;
}dbInfo;

enum COMMTYPE
{
    DEV_LOGIN = 1,
    DEV_BEATS = 21
};
enum KEY
{
    ID  = 0,
    CPU ,
    MEMORY,
    PLAYID,
    LASTTIME,
    PWD,
    PLAYLIST,
    ADDLIST,
    PLAYUPDATE,
    ADDUPDATE,
    APKVERSION,
    APKTIME,
    NOTIFY,
    DEBUGLOG
};

static const char *KEYS[] ={"deviceId","cpu","memory","play_file_id","last_login_time","pwd",\
                    "playlist","addplaylist","playlist_update","addplaylist_update",\
                    "apkversion","apkupdatetime","notify","debuglog"};


#endif // GLOBAL

