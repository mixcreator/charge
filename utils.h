#ifndef __UTILS_H__ 
#define __UTILS_H__  20180517

#include <pthread.h>
#include <syslog.h>

#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

    
#include <linux/can.h>
#include <linux/can/raw.h>


class MMutex{
    friend class Conditional;
public:
    MMutex();
    ~MMutex();
//private:
//    friend class Lock;
    void lock();
    
    void unlock();
private:
    pthread_mutex_t mtx;
};

class RWMutex{
public:
    RWMutex();
    ~RWMutex();
    void rdlock();
    void wrlock();
    void unlock();
private:
    pthread_rwlock_t mtx;
};

class Lock{
public:
    explicit Lock(MMutex& m);

    ~Lock();
private:    
    MMutex& mtx;
};

class RLock{
public:
    explicit RLock(RWMutex& m);

    ~RLock();
private:    
    RWMutex& mtx;
};


class RWLock{
public:
    explicit RWLock(RWMutex& m);

    ~RWLock();
private:    
    RWMutex& mtx;
};

class Conditional  :public MMutex{
public:    
    Conditional();
    ~Conditional();

    int wait();
    int signal();
    int broadcast();
private:
    pthread_cond_t cond;
};


class MThread{
public:    
    MThread();
    MThread(const char *ctx);
    virtual ~MThread(){}

public:
    void start();

    virtual int run(){ return 0;}

    int join();

    int join(void **retval);
    
    int detach();

    int cancel();

    int setcancelstate(int state);

    int setcanceltype(int type);

    inline int get_retcode()const{
        return retcode;
    }
protected:
    const char *context;
private:
    static void *th_routine(void *arg);
    int retcode;
    //const char *context;
    pthread_t th;
};

void openLog(const char *appname);

void Log(int priority, const char * fmtspec, ...);

void Log(int priority, const struct can_frame& frame, bool is_in);

void closeLog();


#endif


