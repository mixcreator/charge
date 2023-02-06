#include <stdarg.h>
#include <sys/time.h>

#include "utils.h"

MMutex::MMutex(){
    pthread_mutex_init(&mtx, 0);
}

MMutex::~MMutex(){
    pthread_mutex_destroy(&mtx);
}
//private:
//    friend class Lock;
void MMutex::lock(){
        pthread_mutex_lock(&mtx);
}
    
void MMutex::unlock(){
        pthread_mutex_unlock(&mtx);
}



RWMutex::RWMutex(){
    pthread_rwlock_init(&mtx, NULL);
}

RWMutex::~RWMutex(){
    pthread_rwlock_destroy(&mtx);
}

void RWMutex::rdlock(){
    pthread_rwlock_rdlock(&mtx);
}

void RWMutex::wrlock(){
    pthread_rwlock_wrlock(&mtx);
}

void RWMutex::unlock(){
    pthread_rwlock_unlock(&mtx);
}




Lock::Lock(MMutex &m): mtx(m){
        mtx.lock();
}

Lock::~Lock(){
        mtx.unlock();
}


RLock::RLock(RWMutex &m): mtx(m){
        mtx.rdlock();
}

RLock::~RLock(){
        mtx.unlock();
}


RWLock::RWLock(RWMutex &m): mtx(m){
        mtx.wrlock();
}

RWLock::~RWLock(){
        mtx.unlock();
}


Conditional::Conditional(): MMutex(){
    pthread_cond_init(&cond, NULL);
}

Conditional::~Conditional(){
    pthread_cond_destroy(&cond);
}

int Conditional::wait(){
    return pthread_cond_wait(&cond, &mtx);
}

int Conditional::signal(){
    return pthread_cond_signal(&cond);
}

int Conditional::broadcast(){
    return pthread_cond_broadcast(&cond);
}


//MThread::MThread(int sock):sockfd(sock){}
MThread::MThread():context(NULL), retcode(0){}

MThread::MThread(const char *ctx):retcode(0){
    context = ctx;
}

//int MThread::retcode;

void MThread::start(){
        pthread_create(&th, NULL, &MThread::th_routine, this);
}

int MThread::join(){
    return pthread_join(th, NULL);
}

int MThread::join(void **retval){
    return pthread_join(th, retval);
}

int MThread::detach(){
        return pthread_detach(th);
}

int MThread::cancel(){
        return pthread_cancel(th);
}

/*
    PTHREAD_CANCEL_ENABLE
    PTHREAD_CANCEL_DISABLE
*/
int MThread::setcancelstate(int state){
    int old_state;
    return pthread_setcancelstate(state, &old_state);
}

/*
    PTHREAD_CANCEL_DEFERRED
    PTHREAD_CANCEL_ASYNCHRONOUS    
*/
int MThread::setcanceltype(int type){
    int old_type;
    return pthread_setcanceltype(type, &old_type);
}


void *MThread::th_routine(void *arg){

    ((MThread*)arg)->retcode = ((MThread*)arg)->run();
    return (void*)&(((MThread*)arg)->retcode);
    //return (void*)&(static_cast<MThread*>(arg)->retcode);
}



void openLog(const char *appname){
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(appname, LOG_NDELAY | LOG_PID, LOG_LOCAL6);
}

void Log(int priority, const char * fmtspec, ...)
{
    va_list arglist;
    va_start (arglist, fmtspec);
    vsyslog(priority,fmtspec, arglist);
    va_end(arglist);
}

void Log(int priority, const struct can_frame& frame, bool is_in){
    char buff[128];
    memset(buff, 0, sizeof(buff));
    strcpy(buff, (is_in)?"DATA_IN: ":"DATA_OUT: ");
    char tbuf[8];
    sprintf(tbuf, "%02x ", frame.can_id);
    strcat(buff, tbuf);
    sprintf(tbuf, "%02x  ", frame.can_dlc);
    strcat(buff, tbuf);
    for(int i=0;i<frame.can_dlc; i++){
        sprintf(tbuf, "%02X ", (char)frame.data[i]);
        strcat(buff, tbuf);
    }
    syslog(priority, "%s", buff);
}


void closeLog(){
    closelog();
}
#if 0
void print_current_time_with_ms(const char *th){
    struct timeval start;
    gettimeofday(&start, NULL);
    Log(LOG_INFO, "%s : TS: %ld: ", th, start.tv_sec * 1000 + start.tv_usec/1000);
}

#define USE_MTX 111

static pthread_mutex_t mtx;

static void init_mutex(){
#ifdef USE_MTX
    pthread_mutex_init(&mtx, 0);
#endif
}

static void mlock(){
#ifdef USE_MTX
    pthread_mutex_lock(&mtx);
#endif
}

static void munlock(){
#ifdef USE_MTX
    pthread_mutex_unlock(&mtx);
#endif
}


static void form_date(char *date){
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);

    tm = localtime(&tv.tv_sec);

    sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d.%03d:  ",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec,
            (int) (tv.tv_usec / 1000)
        );

}

void print_can_frame(FILE *fstream, const struct can_frame& frame, bool is_in){
    char date[64];
    mlock();
    form_date(date);
    fprintf(fstream, "%s", date);
    fprintf(fstream, "%s", is_in?"DATA_IN: ":"DATA_OUT: ");
    fprintf(fstream, " %02x", frame.can_id);
    fprintf(fstream, " %02x  ", frame.can_dlc);

    int len = frame.can_dlc;
    for(int i=0;i<len;i++){
        fprintf(fstream, " %02x", frame.data[i]);
    }
    fprintf(fstream, "\n");
    munlock();
    fflush(fstream);
}

void print_actual_voltage_current(FILE *fstream, float voltage, float current){
    char date[64];
    mlock();
    form_date(date);
    fprintf(fstream, "%s", date);

    fprintf(fstream, "Current: %.1f А, Voltage: %.1f V\n", current, voltage);
    munlock();
    fflush(fstream);

}

void print_current_request(FILE *fstream, float current){
    char date[64];
    mlock();
    form_date(date);
    fprintf(fstream, "%s", date);
    fprintf(fstream, "Current request: %.1f А\n", current);
    munlock();
    fflush(fstream);
}

void print_message(FILE *fstream, const char *msg){
    char date[64];
    mlock();
    form_date(date);
    fprintf(fstream, "%s", msg);
    munlock();
    fflush(fstream);
}

FILE *open_log_file(){
    char out_filename[32];
    //sprintf(out_filename, "%lu", time(NULL));
    form_date(out_filename);
    FILE *fstream = fopen(out_filename, "w+");
    if(!fstream){
        return NULL;
    }
    init_mutex();
    return fstream;
}
#endif

