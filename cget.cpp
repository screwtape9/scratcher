#include <cstdio>
#include <cstring>
#include "cget.h"

typedef struct _data_pair {
  FILE *fp;
  CGET *cget;
} data_pair;

static size_t onWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  return fwrite(buffer, size, nmemb, (FILE *)stream);
}

static void dump(const char *txt,
                 FILE *fp,
                 unsigned char *ptr,
                 size_t sz,
                 curl_infotype type,
                 CGET::LogLevel logLvl)
{
  const unsigned int width = 16;

  if (logLvl >= CGET::LogLevel::Trace) {
    fprintf(fp, "%s, %10.10ld bytes\n", txt, (long)sz);
    for(size_t i = 0; i < sz; i += width) {
      fprintf(fp, "0x%8.8lx: ", (long)i);

      // show hex to the left
      for(size_t c = 0; c < width; c++) {
        if(i + c < sz)
          fprintf(fp, "%02x ", ptr[i + c]);
        else
          fputs("   ", fp);
      }

      // show data on the right
      for(size_t c = 0; (c < width) && ((i + c) < sz); c++) {
        char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
        fputc(x, fp);
      }
      fputc('\n', fp);
    }
  }
  else if (logLvl >= CGET::LogLevel::Debug) {
    if ((type == CURLINFO_HEADER_OUT) || (type == CURLINFO_HEADER_IN))
      fwrite(ptr, 1, sz, fp);
  }
}

static int onTrace(__attribute__((unused)) CURL *handle,
                   curl_infotype type,
                   char *data,
                   size_t sz,
                   void *ptr)
{
  data_pair *pair = (data_pair *)ptr;
  FILE *fp = pair->fp;
  CGET *cget = pair->cget;
  const char *txt = nullptr;
  bool logit = false;

  switch (type) {
  case CURLINFO_TEXT:
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Info);
    if (logit) {
      fwrite(data, 1, sz, fp);
      logit = false;
    }
    break;
  case CURLINFO_HEADER_IN:
    txt = "<= RCV HDR";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Debug);
    break;
  case CURLINFO_HEADER_OUT:
    txt = "=> SND HDR";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Debug);
    break;
  case CURLINFO_DATA_IN:
    txt = "<= RCV DATA";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Trace);
    break;
  case CURLINFO_DATA_OUT:
    txt = "=> SND DATA";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Trace);
    break;
  case CURLINFO_SSL_DATA_IN:
    txt = "<= RCV SSL DATA";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Trace);
    break;
  case CURLINFO_SSL_DATA_OUT:
    txt = "=> SND SSL DATA";
    logit = (cget->GetLogLevel() >= CGET::LogLevel::Trace);
    break;
  default: /* in case a new one is introduced to shock us */
    break;
  }

  if (logit)
    dump(txt, fp, (unsigned char *)data, sz, type, cget->GetLogLevel());

  cget = nullptr;
  fp   = nullptr;
  pair = nullptr;
  txt  = nullptr;

  return 0;
}

CGET::CGET() : m_log_level(LogLevel::None)
{
  memset(szLastErr, 0, sizeof(szLastErr));
}

bool CGET::GetFile(const char *url, const char *outFile, const char *logFile)
{
  FILE *fplog = fopen(logFile, "wb"); // TODO: append log file?
  if (!fplog)
    return false;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  bool ret = false;
  CURL *curl = curl_easy_init();
  if (curl) {
    ret = true;
    FILE *fp = fopen(outFile, "wb");
    if (fp) {
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L); // TODO: configurable?
      curl_easy_setopt(curl, CURLOPT_URL, url);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWrite);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, onWrite);
      //curl_easy_setopt(curl, CURLOPT_HEADERDATA, fplog);
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, onTrace);
      data_pair pair = { fplog, this };
      curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &pair);
      memset(szLastErr, 0, sizeof(szLastErr));
      curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, szLastErr);
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        fprintf(fplog, "Oh no! curl_easy_perform() returned: %d\n", res);
        size_t len = strlen(szLastErr);
        if (len)
          fprintf(fplog, "%s%s",
                  szLastErr, ((szLastErr[len - 1] != '\n') ? "\n" : ""));
        else
          fprintf(fplog, "%s\n", curl_easy_strerror(res));
        ret = false;
      }
      fclose(fp);
      fp = nullptr;
    }
    else
      ret = false;
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  
  fclose(fplog);
  fplog = nullptr;

  return ret;
}
