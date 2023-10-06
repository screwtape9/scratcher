#ifndef __CURL_GET_H_
#define __CURL_GET_H_

#include <curl/curl.h>

class CGET
{
  public:
    enum class LogLevel { None, Info, Debug, Trace, NumLogLevels };
    CGET();
    ~CGET() { }
    bool GetFile(const char *url, const char *outFile, const char *logFile);
    LogLevel GetLogLevel() { return m_log_level; }
    void SetLogLevel(LogLevel lvl) { m_log_level = lvl; }
    const char *GetLastError() { return &szLastErr[0]; }
  private:
    CGET(CGET const&) { }
    CGET& operator=(CGET const&) { return (*this); }
    LogLevel m_log_level;
    char szLastErr[CURL_ERROR_SIZE];
};

#endif // __CURL_GET_H_
