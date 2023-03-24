#include <curl/mprintf.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define API_URL "https://api.telegram.org"

void check_curl(CURLcode code) {
  if (code != 0) {
    fprintf(stderr, "[CURL] %s\n", curl_easy_strerror(code));
    exit(1);
  }
}

void check_curlu(CURLUcode code) {
  if (code != 0) {
    fprintf(stderr, "[CURLU] %s\n", curl_url_strerror(code));
    exit(1);
  }
}

char *get_url(char *token, char *method) {
  return curl_maprintf("https://api.telegram.org/bot%s/%s", token, method);
}

// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html#EXAMPLE
typedef struct Response {
  char *content;
  size_t size;
} Response;

static size_t write_function(void *data, size_t size, size_t nmemb,
                             void *clientp) {
  size_t realsize = size * nmemb;
  Response *response = (Response *)clientp;

  char *ptr = (char *)realloc(response->content, response->size + realsize + 1);
  if (ptr == NULL) return 0; /* out of memory! */

  response->content = ptr;
  memcpy(&(response->content[response->size]), data, realsize);
  response->size += realsize;
  response->content[response->size] = 0;

  return realsize;
}
