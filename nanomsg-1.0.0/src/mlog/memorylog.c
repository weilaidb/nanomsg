#include "memorylog.h"
#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "cvector.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#ifdef __WIN32__
#include <windows.h>
#endif

#define MUTEX_P(mutex) \
do{\
    MemoryLogInit();\
    pthread_mutex_lock   (&mutex);\
}while(0)

#define MUTEX_V(mutex)\
do{\
    MemoryLogInit();\
    pthread_mutex_unlock   (&mutex);\
}while(0)


struct mlognode {
    struct rb_node node;
    char *string;
    Vector *cvec;
};

struct rb_root mlogtree = RB_ROOT;
pthread_mutex_t mutex;



/*============================================
* FuncName    : mlog_search
* Description :
* @root       :
* @string     :
* Author      :
* Time        : 2017-06-11
============================================*/
struct mlognode * mlog_search(struct rb_root *root, char *string)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct mlognode *data = container_of(node, struct mlognode, node);
        int result;

        result = strcmp(string, data->string);

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return data;
    }
    return NULL;
}

/*============================================
* FuncName    : mlog_insert
* Description :
* @root       :
* @data       :
* Author      :
* Time        : 2017-06-11
============================================*/
int mlog_insert(struct rb_root *root, struct mlognode *data)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        struct mlognode *this = container_of(*new, struct mlognode, node);
        int result = strcmp(data->string, this->string);

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, root);

    return 1;
}

/*============================================
* FuncName    : mlog_free
* Description :
* @node       :
* Author      :
* Time        : 2017-06-11
============================================*/
void mlog_free(struct mlognode *node)
{
    if (node != NULL) {
        if (node->string != NULL) {
            free(node->string);
            node->string = NULL;
        }
        free(node);
        node = NULL;
    }
}



/*============================================
* FuncName    : MemoryLogInit
* Description :
* @           :
* Author      :
* Time        : 2017-06-05
============================================*/
void MemoryLogInit()
{
    static int flag = 0;
    if(flag)
        return;
    //    mlog.clear();
    pthread_mutex_init (&mutex,NULL);
    flag = 1;
}

/*============================================
* FuncName    : PushLog
* Description : 推入数据流到内存
* @key        : 索引字
* @fmt        : 字符串格式
* @--         : 变长
* Author      :
* Time        : 2017-06-05
============================================*/
void PushLog(const char *key, T_MLOG *plog)
{
    MUTEX_P (mutex);

    struct mlognode *data = mlog_search(&mlogtree, key);
    if (data)
    {
        if(VectorSize(data->cvec) >= get_mlogmaxsize())
        {
            MUTEX_V(mutex);
            return;
        }

        if(data->cvec)
            VectorPushBack(data->cvec, *plog);
    }
    else
    {
        struct mlognode *firstdata = (char *)malloc(sizeof(struct mlognode));
        memset(firstdata, 0, sizeof(*firstdata));
        firstdata->string = (char *)malloc(strlen(key) + 1);
        memset(firstdata->string, 0 , strlen(key) + 1);
        memcpy(firstdata->string, key, strlen(key));
        firstdata->string[strlen(key) + 1] = '\0';
        firstdata->cvec = VectorNew();
        if(firstdata->cvec)
            VectorPushBack(firstdata->cvec, *plog);
        mlog_insert(&mlogtree, firstdata);
    }

    MUTEX_V(mutex);
}





/*============================================
* FuncName    : CheckPushLog
* Description :
* @key        :
* Author      :
* Time        : 2017-06-07
============================================*/
int CheckPushLog(const char *key)
{
    MUTEX_P (mutex);
    struct mlognode *data = mlog_search(&mlogtree, key);
    if(data)
    {
        if(get_mlogformat() && VectorSize(data->cvec) >= get_mlogmaxsize())
        {
            T_MLOG tlog = VectorGet(data->cvec, 0);
            free(tlog.msgaddr);
            VectorRm(data->cvec, 0); //erase the first data
            MUTEX_V(mutex);
            return 1;
        }

        if(VectorSize(data->cvec) >= get_mlogmaxsize())
        {
            MUTEX_V(mutex);
            return 0;
        }
        else
        {
            MUTEX_V(mutex);
            return 1;
        }
    }
    MUTEX_V (mutex);

    return 1;
}



/*============================================
* FuncName    : ShowLogByName
* Description : show key's memory log
* @key        :
* @index      : show tips or not
* Author      :
* Time        : 2017-06-05
============================================*/
void ShowLogByName(const char *key, int index)
{
    MUTEX_P (mutex);
    if(index)
        fprintf(stdout, "show mlog key[%-10s], ", key);
    struct mlognode *data = mlog_search(&mlogtree, key);
    if(data)
    {
        T_MLOG tlog = VectorGet(data->cvec, 0);
        unsigned int vectorsize = VectorSize(data->cvec);
        fprintf(stdout, "\033[7m[%s]\033[0m size:%u\n", key, vectorsize);
        unsigned loop  =  0;
        for( loop  =  0 ; loop < vectorsize; loop++ )
        {
            ParseMsgBody(VectorGet(data->cvec, loop ),stdout);
        }
        if(index)
            fprintf(stdout, "show mlog key[%s] size:%u done!!\n\n", key, vectorsize);
    }
    else
    {
        if(index)
            fprintf(stdout, "no data!!\n\n");
    }
    MUTEX_V (mutex);
}







/*============================================
* FuncName    : MemoryLog::Version
* Description :
* @           :
* Author      :
* Time        : 2017-06-10
============================================*/
const char* Version()
{
    return MLOG_VERSION;
}



/*============================================
* FuncName    : timeval_diff
* Description : caculate time diff
* @tvbegin    : begin time
* @tvend      : end time
* Author      :
* Time        : 2017-06-07
============================================*/
unsigned int timeval_diff(struct timeval *tvbegin, struct timeval *tvend)
{
    unsigned int diff = 0;
    if(tvbegin->tv_sec > tvend->tv_sec)
    {
        diff = (tvbegin->tv_sec - tvend->tv_sec) * 1000 + (tvbegin->tv_usec - tvend->tv_usec)/1000;
    }
    else if(tvbegin->tv_sec ==  tvend->tv_sec)
    {
        diff = (tvbegin->tv_usec > tvend->tv_usec) ? (tvbegin->tv_usec - tvend->tv_usec)/1000 :
                                                     (tvend->tv_usec - tvbegin->tv_usec)/1000;
    }
    else
    {
        diff = (tvend->tv_sec - tvbegin->tv_sec) * 1000 + (tvend->tv_usec - tvbegin->tv_usec)/1000;
    }

    return diff;
}





/* variable declare begin */
unsigned int mlogmaxsize = 100;
/* variable declare end */
/*
Set and Get for mlogmaxsize
*/
/*============================================
* FuncName    : set_mlogmaxsize
* Description :
* @_mlogmaxsize  :
* Author      :
* Time        : 2017-06-05
============================================*/
void set_mlogmaxsize(unsigned int _mlogmaxsize)
{
    mlogmaxsize = _mlogmaxsize;
}

/*============================================
* FuncName    : get_mlogmaxsize
* Description :
* @--         :
* Author      :
* Time        : 2017-06-05
============================================*/
unsigned int get_mlogmaxsize(void)
{
    return mlogmaxsize;
}


/* variable declare begin */
static int mlogformat = 1;
/* variable declare end */
/*
Set and Get for mlogformat
*/
/*============================================
* FuncName    : set_mlogformat
* Description :
* @_mlogformat  : the data format : erase the first data when full or keep the full
*                0 - keep the full
*                1 - erase the first data (default)
* Author      :
* Time        : 2017-06-10
============================================*/
void set_mlogformat(int _mlogformat)
{
    mlogformat = _mlogformat;
}

/*============================================
* FuncName    : get_mlogformat
* Description :
* @--         :
* Author      :
* Time        : 2017-06-10
============================================*/
int get_mlogformat(void)
{
    return mlogformat;
}



/*============================================
* FuncName    : mloghelp
* Description :
* @           :
* Author      :
* Time        : 2017-06-10
============================================*/
void mloghelp()
{
    const char *usage = "memory log help:\n"
            "push log by name     --   pushlogbyname(const char *key, char *fmt, ...)\n"
            "push msg by name     --   pushmsgbyname(const char *key, *msg, unsigned int msglen, char *fmt, ...)\n"
            "show log by name     --   showmlogbyname( const char *key)\n"
            "show log of all      --   showmlogall()\n"
            "clear log by name    --   clearmlogbyname( const char *key)\n"
            "clear log of all     --   clearmlogall()\n"
            "show mlog keys       --   showmlogkeys()\n"
            "save log 2file by name      --   savemlog2filebyname(const char *key)\n"
            "save log 2file of all       --   savemlog2fileall(const char *filewithpath)\n"
            "save log 2file of keys      --   savemlog2filekeys()\n"
            "set mlog store max size     --   set_mlogmaxsize(unsigned int _mlogmaxsize)\n"
            "get mlog store max size     --   get_mlogmaxsize(void)\n"
            ;
    printf(usage);
    printf("mlog max size     :%u\n", get_mlogmaxsize());
    printf("mlog record format:%u\n", get_mlogformat());
    printf("    0 - no overwrite, 1 - overwrite\n");
    printf("mlog version      :%s\n", Version());
}



/*============================================
  * FuncName    : MemoryLog::ParseMsgBody
  * Description : parse msg body , the format of msg show;print or save msg body
  * @tlog       :
  * @fp         :stdout or files pointer
  * Author      :
  * Time        : 2017-06-10
  ============================================*/
void ParseMsgBody(T_MLOG tlog, FILE *fp)
{
    if(0 == tlog.msglen && NULL == tlog.msgaddr)
        fprintf(fp, "%s\n", tlog.tipsinfo);
    else
    {
        char *pmsg = NULL;
        unsigned int msglen = tlog.msglen;
        fprintf(fp, "%s\n", tlog.tipsinfo);
        pmsg = (char *)malloc(msglen * 3 + (msglen < 16 ? 1 : msglen /16) *4);
        if(!pmsg)
        {
            return;
        }
        memset(pmsg, 0, msglen);
        unsigned int i = 0;
        unsigned int j = 0;
        unsigned int tnum = 0;
        unsigned int cpos = 0;
        char *rmsg = (char *)tlog.msgaddr;
        tnum = sprintf(pmsg + cpos, "\n");
        for(i = 0; i < msglen; i++)
        {
            j = i + 1;
            if(0 == i)
            {
                tnum = sprintf(pmsg + cpos, "\n");
                cpos += tnum;
            }
            if(0 == j % 8 && 0 != j % 16)
                tnum = sprintf(pmsg + cpos, "%02X  ", (unsigned char)rmsg[i]);
            else if( 0 == j % 16)
                tnum = sprintf(pmsg + cpos, "%02X", (unsigned char)rmsg[i]);
            else
                tnum = sprintf(pmsg + cpos, "%02X ", (unsigned char)rmsg[i]);
            cpos += tnum;
            if( 0 == (j) % 16)
            {
                tnum = sprintf(pmsg + cpos, "\n");
                cpos += tnum;
            }
        }
        fprintf(fp, "%s\n", pmsg);
        free(pmsg);
    }
}

/*============================================
* FuncName    : GetMLogSize
* Description : memory log size
* @           :
* Author      :
* Time        : 2017-06-13
============================================*/
unsigned int GetMLogSize()
{
    struct rb_node *node;
    unsigned int size = 0;
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
        size++;

    return size;
}


/*============================================
  * FuncName    : ShowLogAll
  * Description : show all key's memory log
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void ShowLogAll()
{
    unsigned int mlogsize = GetMLogSize();
    if( !mlogsize )
    {
        fprintf(stdout, "show mlog, no data!!\n");
        return;
    }
    fprintf(stdout, "show all mlog data, size:%u\n", mlogsize);
    /* *search */
    struct rb_node *node;
    printf("search all nodes: \n");
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
        ShowLogByName(rb_entry(node, struct mlognode, node)->string, 0);

    fprintf(stdout, "show all mlog data, size:%u, done!!\n", mlogsize);
}
/*============================================
  * FuncName    : ShowLogKeys
  * Description : show memory log's keys
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void ShowLogKeys(FILE *fp)
{
    unsigned int mlogsize = GetMLogSize();
    if(!mlogsize )
    {
        fprintf(fp, "show mlog keys, no data!!\n");
        return;
    }
    fprintf(fp, "show mlog keys, size:%u\n", mlogsize);
    if(mlogsize)
    {
        fprintf(fp,"[key            ] [count]   [msglen]\n");
    }
    struct rb_node *node;
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
    {

        unsigned int vecsize = VectorSize(rb_entry(node, struct mlognode, node)->cvec);
        unsigned int msglen = 0;
        unsigned int loop  =  0;
        for( loop  =  0 ; loop < vecsize; loop++ )
        {
            msglen += VectorGet(rb_entry(node, struct mlognode, node)->cvec, loop ).msglen;
        }

        fprintf(fp,"%-020s %-07u %-08u\n" ,
                rb_entry(node, struct mlognode, node)->string,
                vecsize,
                msglen);
    }
    fprintf(fp, "show mlog keys, size:%u, done!!\n", mlogsize);
}

/*============================================
* FuncName    : MemoryLog::FindLogKeys
* Description : find memory log keys by name
* @fp         :
* @name       : the key words to find
* Author      :
* Time        : 2017-06-15
============================================*/
void FindLogKeys(FILE *fp, const char *name)
{
    unsigned int mlogsize = GetMLogSize();
    if(!mlogsize )
    {
        fprintf(fp, "find mlog keys by \"%s\", no data!!\n", name);
        return;
    }
    fprintf(fp, "find mlog keys by \"%s\", mlog size:%u\n", name, mlogsize);
    if(mlogsize)
    {
        fprintf(fp,"[key            ] [count]   [msglen]\n");
    }

    unsigned int findsize = 0;
    struct rb_node *node;
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
    {
        const char * findstr = rb_entry(node, struct mlognode, node)->string;
        if(!strstr(findstr, name))
        {
            continue;
        }
        findsize++;

        unsigned int vecsize = VectorSize(rb_entry(node, struct mlognode, node)->cvec);
        unsigned int msglen = 0;
        unsigned int loop  =  0;
        for( loop  =  0 ; loop < vecsize; loop++ )
        {
            msglen += VectorGet(rb_entry(node, struct mlognode, node)->cvec, loop ).msglen;
        }

        fprintf(fp,"%-020s %-07u %-08u\n" ,
                findstr,
                vecsize,
                msglen);
    }

    fprintf(fp, "find mlog keys by \"%s\", mlog size:%u, find size:%u done!!\n",
            name ,
            mlogsize, findsize);
}


/*============================================
  * FuncName    : ClearLogByName
  * Description :
  * @key        :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void ClearLogByName(const char *key, int tips, int lock)
{
    if(lock)
        MUTEX_P (mutex);
    if(tips)
        fprintf(stdout, "clear mlog key[%s], ", key);
    struct mlognode *data = mlog_search(&mlogtree, key);
    if(data)
    {
        unsigned int vecsize = VectorSize(data->cvec);
        unsigned int loop  =  0;
        for( loop  =  0 ; loop < vecsize; loop++ )
        {
            T_MLOG tlog = VectorGet(data->cvec, loop);
            free(tlog.msgaddr);
        }
        VectorDelete(data->cvec);
        rb_erase(data, &mlogtree);
        mlog_free(data);
        if(tips)
            fprintf(stdout, "\nclear mlog key[%s] done!!\n", key);
    }
    else
    {
        if(tips)
            fprintf(stdout, "no data!!\n");
    }
    if(lock)
        MUTEX_V (mutex);
}
/*============================================
* FuncName    : ClearLogAll
* Description :
* @           :
* Author      :
* Time        : 2017-06-05
============================================*/
void ClearLogAll()
{
    MUTEX_P (mutex);
    unsigned int mlogsize = GetMLogSize();
    if(!mlogsize )
    {
        fprintf(stdout, "clear all mlog, no data!!\n");
        MUTEX_V (mutex);
        return;
    }
    fprintf(stdout, "clear all mlog, size:%u\n", mlogsize);
    char **keys = (char **)malloc(mlogsize * sizeof(char *));
    if(!keys)
    {
        MUTEX_V (mutex);
        return;
    }
    memset(keys, 0, (mlogsize * sizeof(char *)));
    unsigned int cnt = 0;


    struct rb_node *node;
    unsigned int size = 0;
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
    {
        keys[cnt] = (char *)malloc(128);
        if(!keys[cnt])
        {
            MUTEX_V (mutex);
            return;
        }
        memset(keys[cnt], 0 , 128);
        snprintf(keys[cnt],128, "%s", rb_entry(node, struct mlognode, node)->string);
        cnt++;
    }

    unsigned int loop  =  0;
    for( loop  =  0 ; loop < mlogsize; loop++ )
    {
        ClearLogByName(keys[loop], 1, 0);
    }

    for( loop  =  0 ; loop < cnt; loop++ )
    {
        free(keys[loop]);
    }
    free(keys);

    fprintf(stdout, "clear all mlog, size:%u, done!!\n", mlogsize);
    MUTEX_V (mutex);
}
/*============================================
  * FuncName    : SaveLog2FileByName
  * Description :
  * @key        :
  * @tips       :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void SaveLog2FileByName(const char *key, const char *filewithpath,  int tips, FILE *fother)
{
    const char * name = (filewithpath);
    FILE *fp = NULL;
    if(NULL == fother)
    {
        fp = fopen(name, "w");
        if(!fp)
        {
            fprintf(stderr, "open file err![%-10s] %s\n", name, strerror(errno));
            return;
        }
    }
    else
    {
        fp = fother;
    }
    struct timeval now;
    gettimeofday(&now, NULL);
    MUTEX_P (mutex);
    if(tips && name != ""){
        //        fprintf(fp, "time:s-us: %u-%u\n", now.tv_sec, now.tv_usec);
        fprintf(stdout, "save mlog to file[%-10s], ", name);
        fprintf(fp, "save mlog to file[%-10s], ", name);
    }
    else if(tips)
    {
        //        fprintf(fp, "time:s-us: %u-%u\n", now.tv_sec, now.tv_usec);
        fprintf(stdout, "save mlog key[%-10s], ", key);
        fprintf(fp, "save mlog key[%-10s], ", key);
    }
    struct mlognode *data = mlog_search(&mlogtree, key);
    if(data)
    {
        T_MLOG tlog = VectorGet(data->cvec, 0);
        unsigned int vectorsize = VectorSize(data->cvec);
        fprintf(fp, "[%s] size:%u\n", key, vectorsize);
        unsigned loop  =  0;
        for( loop  =  0 ; loop < vectorsize; loop++ )
        {
            ParseMsgBody(VectorGet(data->cvec, loop ),fp);
        }
        if(tips){
            fprintf(stdout, "save mlog key[%-10s], done!!\n\n", key);
            fprintf(fp, "save mlog key[%-10s], done!!\n\n", key);
        }
    }
    else
    {
        if(tips)
            fprintf(fp, "no data!!\n\n");
    }
    fflush(fp);
    if(NULL == fother)
    {
        fclose(fp);
    }
    MUTEX_V (mutex);
}
/*============================================
  * FuncName    : MemoryLog::SaveLog2FileAll
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void SaveLog2FileAll(const char *filewithpath)
{
    unsigned int mlogsize = GetMLogSize();
    if(!mlogsize )
    {
        fprintf(stdout, "save mlog to file, no data!!\n");
        return;
    }
    const char *name = "./savemlogs.mlog";
    if(NULL != filewithpath)
    {
        name = filewithpath;
    }
    FILE *fp = fopen(name, "w");
    if(!fp)
    {
        fprintf(stderr, "open file err![%-10s] %s\n", name, strerror(errno));
        return;
    }
    SaveLog2FileKeys(fp);
    fprintf(stdout, "save mlog to file[%-10s], map size:%u\n",name,  mlogsize);
    fprintf(fp, "save mlog to file, size:%u\n", mlogsize);
    /* *search */
    struct rb_node *node;
    printf("search all nodes: \n");
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
        SaveLog2FileByName(rb_entry(node, struct mlognode, node)->string, name,
                               0, fp);

    fprintf(fp, "save mlog to file[%-10s], size:%u, done!!\n",name, mlogsize);
    fprintf(stdout, "save mlog to file[%-10s], size:%u, done!!\n",name, mlogsize);
    fclose(fp);
}
void SaveLog2FileKeys(FILE *felse)
{
    unsigned int mlogsize = GetMLogSize();
    FILE *fp = NULL;
    if(NULL == felse)
    {
        const char *name = "./mlog_keys.txt";
        fp = fopen(name, "w");
        if(!fp)
        {
            fprintf(stderr, "open file err![%-10s] %s\n", name, strerror(errno));
            return;
        }
    }
    else
    {
        fp = felse;
    }
    if(!mlogsize )
    {
        fprintf(stdout, "save mlog keys, no data!!\n");
        fprintf(fp, "save mlog keys, no data!!\n");
        return;
    }
    fprintf(stdout, "save mlog keys, size:%u, done!!\n", mlogsize);
    fprintf(fp, "save mlog keys, size:%u, done!!\n", mlogsize);
    if(mlogsize)
    {
        fprintf(fp,"[key            ] [count]   [msglen]\n");
    }
    struct rb_node *node;
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
    {

        unsigned int vecsize = VectorSize(rb_entry(node, struct mlognode, node)->cvec);
        unsigned int msglen = 0;
        unsigned int loop  =  0;
        for( loop  =  0 ; loop < vecsize; loop++ )
        {
            msglen += VectorGet(rb_entry(node, struct mlognode, node)->cvec, loop ).msglen;
        }

        fprintf(fp,"%-020s %-07u %-08u\n" ,
                rb_entry(node, struct mlognode, node)->string,
                vecsize,
                msglen);
    }

    fprintf(fp, "show mlog keys, size:%u, done!!\n", mlogsize);
    if(NULL == felse)
    {
        fclose(fp);
    }
}
//  #ifdef __cplusplus
//  extern "C" {
//  #endif
//  /* func_declare_begin */
//  void showmlogbyname( const char *key);
//  void showmlogall();
//  void clearmlogbyname( const char *key);
//  void clearmlogall();
//  /* func_declare_end */
//  /* func_implement */
/*============================================
  * FuncName    : pushlogbyname
  * Description :
  * @key        :
  * @fmt        :
  * @--         :
  * Author      :
  * Time        : 2017-06-06
  ============================================*/
void pushlogbyname(const char *key, char *fmt, ...)
{
    if(!CheckPushLog(key))
        return;
    va_list ap;
    char buf[1024];
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    T_MLOG tLog = {0};
    snprintf(tLog.tipsinfo, sizeof(tLog.tipsinfo), "%s", buf);
    PushLog(key, &tLog);
}
/*============================================
  * FuncName    : pushmsgbyname
  * Description :
  * @key        :
  * @fmt        :
  * @--         :
  * Author      :
  * Time        : 2017-06-06
  ============================================*/
void pushmsgbyname(const char *key, void *msg, unsigned int msglen, char *fmt, ...)
{
    if(!CheckPushLog(key))
        return;
    T_MLOG tLog = {0};
    char buf[1024];
    va_list ap;
    char *pmsg = NULL;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    memset(&tLog, 0 ,sizeof(tLog));
    snprintf(tLog.tipsinfo, sizeof(tLog.tipsinfo), "\033[4m%s\033[0m", buf);
    PushLog(key, &tLog);
    if(NULL != msg || 0!= msglen) // record msg
    {
        if(!CheckPushLog(key))
            return;
        pmsg = (char *)malloc(msglen);
        if(!pmsg)
            return;
        memset(pmsg, 0, msglen);
        memcpy(pmsg, msg, msglen);
        struct timeval now;
        struct timeval end;
        gettimeofday(&now, NULL);
        sprintf(buf, "smsg:%p,len:%u",
                msg, msglen);
        memset(&tLog, 0 ,sizeof(tLog));
        snprintf(tLog.tipsinfo, sizeof(tLog.tipsinfo), "%s", buf);
        tLog.msgaddr = pmsg;
        tLog.msglen = msglen;
        PushLog(key, &tLog);
        if(!CheckPushLog(key))
            return;
        gettimeofday(&end, NULL);
        sprintf(buf, "smsg:%p,len:%u;elptime::%-3ums",
                msg, msglen,timeval_diff(&end, &now));
        memset(&tLog, 0 ,sizeof(tLog));
        snprintf(tLog.tipsinfo, sizeof(tLog.tipsinfo), "%s", buf);
        PushLog(key, &tLog);
    }
}
/*============================================
  * FuncName    : showmlogbyname
  * Description :
  * @key        :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void showmlogbyname( const char *key)
{
    ShowLogByName(key, 1);
}
/*============================================
  * FuncName    : showmlogall
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void showmlogall()
{
   ShowLogAll();
}
/*============================================
  * FuncName    : clearmlogbyname
  * Description :
  * @key        :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void clearmlogbyname( const char *key)
{
    ClearLogByName(key, 1, 1);
}
/*============================================
  * FuncName    : clearmlogall
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void clearmlogall()
{
    ClearLogAll();
}
/*============================================
  * FuncName    : showmlogkeys
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void showmlogkeys()
{
    ShowLogKeys(stdout);
}

/*============================================
* FuncName    : findmlogkeys
* Description :
* @           :
* Author      :
* Time        : 2017-06-05
============================================*/
void findmlogkeys(const char *name)
{
    FindLogKeys(stdout, name);
}

/*============================================
  * FuncName    : savemlog2filebyname
  * Description :
  * @key        :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void savemlog2filebyname(const char *key, const char *filewithpath)
{
    SaveLog2FileByName(key, filewithpath, 1, NULL);
}
/*============================================
  * FuncName    : savemlog2fileall
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void savemlog2fileall(const char *filewithpath)
{
    SaveLog2FileAll(filewithpath);
}
/*============================================
  * FuncName    : savemlog2filekeys
  * Description :
  * @           :
  * Author      :
  * Time        : 2017-06-05
  ============================================*/
void savemlog2filekeys()
{
    SaveLog2FileKeys(NULL);
}






#define NUM_NODES 32


int test_mlog()
{

    struct mlognode *mn[NUM_NODES];

    /* *insert */
    int i = 0;
    printf("insert node from 1 to NUM_NODES(32): \n");
    for (; i < NUM_NODES; i++) {
        mn[i] = (struct mlognode *)malloc(sizeof(struct mlognode));
        mn[i]->string = (char *)malloc(sizeof(char) * 4);
        sprintf(mn[i]->string, "%d", i);
        mlog_insert(&mlogtree, mn[i]);
    }

    /* *search */
    struct rb_node *node;
    printf("search all nodes: \n");
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
        printf("key = %s\n", rb_entry(node, struct mlognode, node)->string);

    /* *delete */
    printf("delete node 20: \n");
    struct mlognode *data = mlog_search(&mlogtree, "20");
    if (data) {
        rb_erase(&data->node, &mlogtree);
        mlog_free(data);
    }

    /* *delete again*/
    printf("delete node 10: \n");
    data = mlog_search(&mlogtree, "10");
    if (data) {
        rb_erase(&data->node, &mlogtree);
        mlog_free(data);
    }

    /* *delete once again*/
    printf("delete node 15: \n");
    data = mlog_search(&mlogtree, "15");
    if (data) {
        rb_erase(&data->node, &mlogtree);
        mlog_free(data);
    }

    /* *search again*/
    printf("search again:\n");
    for (node = rb_first(&mlogtree); node; node = rb_next(node))
        printf("key = %s\n", rb_entry(node, struct mlognode, node)->string);
    return 0;
}


void test_pushlog()
{
    mloghelp();
    unsigned int loop  =  0;
    for( loop  =  0 ; loop < 40; loop++ )
    {

        pushlogbyname("nihao", "goodthings%u", loop);
    }


    pushlogbyname("nihao2", "goodthings2");
    printf("mlog size:%u\n", GetMLogSize());
    showmlogbyname("nihao");
    clearmlogbyname("nihao2");
    showmlogbyname("nihao2");
    showmlogbyname("nihao");
    clearmlogall();

}


void test_pushpkg()
{
    char buf[104];
    memset(buf, 1, sizeof(buf));

    pushmsgbyname("onetest", buf, sizeof(buf), "one test");
    pushmsgbyname("onetest23", buf, sizeof(buf), "one tes23t");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    showmlogbyname("onetest");
    showmlogall();
    showmlogkeys();
    clearmlogall();

}

void test_savefile()
{
    char buf[104];
    memset(buf, 2, sizeof(buf));
    pushmsgbyname("onetest2343", buf, sizeof(buf), "one onetest2343");
    memset(buf, 3, sizeof(buf));
    pushmsgbyname("onetest1", buf, sizeof(buf), "one onetest1");
    memset(buf, 4, sizeof(buf));
    pushmsgbyname("onetest1", buf, sizeof(buf), "one onetest11");
    showmlogbyname("onetest");

    savemlog2filebyname("onetest2343", "onetest2343.txt");
    savemlog2fileall("./mlog.all.txt");
    findmlogkeys("one");
    clearmlogall();
}




