// Copyright 2019 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "tools/windows/converter_exe/wininet_client.h"

#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <wininet.h>

namespace crash {

namespace internal {

// This class implements HttpClient based on WinInet APIs.
class WinInetClient : public HttpClient {
 public:
  virtual ~WinInetClient() {}
  virtual bool CrackUrl(const TCHAR* url,
                        DWORD flags,
                        TCHAR* scheme,
                        size_t scheme_buffer_length,
                        TCHAR* host,
                        size_t host_buffer_length,
                        TCHAR* uri,
                        size_t uri_buffer_length,
                        int* port) const;
  virtual bool Open(const TCHAR* user_agent,
                    DWORD access_type,
                    const TCHAR* proxy_name,
                    const TCHAR* proxy_bypass,
                    HttpHandle* session_handle) const;
  virtual bool Connect(HttpHandle session_handle,
                       const TCHAR* server,
                       int port,
                       HttpHandle* connection_handle) const;
  virtual bool OpenRequest(HttpHandle connection_handle,
                           const TCHAR* verb,
                           const TCHAR* uri,
                           const TCHAR* version,
                           const TCHAR* referrer,
                           bool is_secure,
                           HttpHandle* request_handle) const;
  virtual bool SendRequest(HttpHandle request_handle,
                           const TCHAR* headers,
                           DWORD headers_length) const;
  virtual bool ReceiveResponse(HttpHandle request_handle) const;
  virtual bool GetHttpStatusCode(HttpHandle request_handle,
                                 int* status_code) const;
  virtual bool GetContentLength(HttpHandle request_handle,
                                DWORD* content_length) const;
  virtual bool ReadData(HttpHandle request_handle,
                        void* buffer,
                        DWORD buffer_length,
                        DWORD* bytes_read) const;
  virtual bool Close(HttpHandle handle) const;

 private:
  static DWORD MapAccessType(DWORD access_type);
  static HINTERNET ToHINTERNET(HttpHandle handle);
  static HttpHandle FromHINTERNET(HINTERNET handle);
};

bool WinInetClient::CrackUrl(const TCHAR* url,
                             DWORD flags,
                             TCHAR* scheme,
                             size_t scheme_buffer_length,
                             TCHAR* host,
                             size_t host_buffer_length,
                             TCHAR* uri,
                             size_t uri_buffer_length,
                             int* port) const {
  assert(url);
  assert(scheme);
  assert(host);
  assert(uri);
  assert(port);

  URL_COMPONENTS url_comp = {0};
  url_comp.dwStructSize = sizeof(url_comp);
  url_comp.lpszScheme = scheme;
  url_comp.dwSchemeLength = static_cast<DWORD>(scheme_buffer_length);
  url_comp.lpszHostName = host;
  url_comp.dwHostNameLength = static_cast<DWORD>(host_buffer_length);
  url_comp.lpszUrlPath = uri;
  url_comp.dwUrlPathLength = static_cast<DWORD>(uri_buffer_length);

  bool result = !!::InternetCrackUrl(url, 0, flags, &url_comp);
  if (result) {
    *port = static_cast<int>(url_comp.nPort);
  }
  return result;
}

bool WinInetClient::Open(const TCHAR* user_agent,
                         DWORD access_type,
                         const TCHAR* proxy_name,
                         const TCHAR* proxy_bypass,
                         HttpHandle* session_handle)  const {
  *session_handle = FromHINTERNET(::InternetOpen(user_agent,
                                                 MapAccessType(access_type),
                                                 proxy_name,
                                                 proxy_bypass,
                                                 0));
  return !!(*session_handle);
}

bool WinInetClient::Connect(HttpHandle session_handle,
                            const TCHAR* server,
                            int port,
                            HttpHandle* connection_handle) const {
  assert(server);

  // Uses NULL user name and password to connect. Always uses http service.
  *connection_handle = FromHINTERNET(::InternetConnect(
                                         ToHINTERNET(session_handle),
                                         server,
                                         static_cast<INTERNET_PORT>(port),
                                         NULL,
                                         NULL,
                                         INTERNET_SERVICE_HTTP,
                                         0,
                                         0));
  return !!(*connection_handle);
}

bool WinInetClient::OpenRequest(HttpHandle connection_handle,
                                const TCHAR* verb,
                                const TCHAR* uri,
                                const TCHAR* version,
                                const TCHAR* referrer,
                                bool is_secure,
                                HttpHandle* request_handle) const {
  assert(connection_handle);
  assert(verb);
  assert(uri);

  *request_handle = FromHINTERNET(::HttpOpenRequest(
                                      ToHINTERNET(connection_handle),
                                      verb,
                                      uri,
                                      version,
                                      referrer,
                                      NULL,
                                      is_secure ? INTERNET_FLAG_SECURE : 0,
                                      NULL));
  return !!(*request_handle);
}

bool WinInetClient::SendRequest(HttpHandle request_handle,
                                const TCHAR* headers,
                                DWORD headers_length) const {
  assert(request_handle);

  return !!::HttpSendRequest(ToHINTERNET(request_handle),
                             headers,
                             headers_length,
                             NULL,
                             0);
}

bool WinInetClient::ReceiveResponse(HttpHandle) const {
  return true;
}

bool WinInetClient::GetHttpStatusCode(HttpHandle request_handle,
                                      int* status_code) const {
  assert(request_handle);

  TCHAR http_status_string[4] = {0};
  DWORD http_status_string_size = sizeof(http_status_string);
  if (!::HttpQueryInfo(ToHINTERNET(request_handle),
                       HTTP_QUERY_STATUS_CODE,
                       static_cast<void*>(&http_status_string),
                       &http_status_string_size,
                       0)) {
    return false;
  }

  *status_code = _tcstol(http_status_string, NULL, 10);
  return true;
}

bool WinInetClient::GetContentLength(HttpHandle request_handle,
                                     DWORD* content_length) const {
  assert(request_handle);
  assert(content_length);

  TCHAR content_length_string[11];
  DWORD content_length_string_size = sizeof(content_length_string);
  if (!::HttpQueryInfo(ToHINTERNET(request_handle),
                       HTTP_QUERY_CONTENT_LENGTH,
                       static_cast<void*>(&content_length_string),
                       &content_length_string_size,
                       0)) {
    *content_length = kUnknownContentLength;
  } else {
    *content_length = wcstol(content_length_string, NULL, 10);
  }
  return true;
}

bool WinInetClient::ReadData(HttpHandle request_handle,
                             void* buffer,
                             DWORD buffer_length,
                             DWORD* bytes_read) const {
  assert(request_handle);
  assert(buffer);
  assert(bytes_read);

  DWORD bytes_read_local = 0;
  if (!::InternetReadFile(ToHINTERNET(request_handle),
                          buffer,
                          buffer_length,
                          &bytes_read_local)) {
    return false;
  }
  *bytes_read = bytes_read_local;
  return true;
}

bool WinInetClient::Close(HttpHandle handle) const {
  assert(handle);
  return !!::InternetCloseHandle(ToHINTERNET(handle));
}

DWORD WinInetClient::MapAccessType(DWORD access_type) {
  switch (static_cast<AccessType>(access_type)) {
    case ACCESS_TYPE_PRECONFIG:
    default:
      return INTERNET_OPEN_TYPE_PRECONFIG;
    case ACCESS_TYPE_DIRECT:
      return INTERNET_OPEN_TYPE_DIRECT;
    case ACCESS_TYPE_PROXY:
      return INTERNET_OPEN_TYPE_PROXY;
  }
}

HINTERNET WinInetClient::ToHINTERNET(HttpHandle handle) {
  return static_cast<HINTERNET>(handle);
}

HttpHandle WinInetClient::FromHINTERNET(HINTERNET handle) {
  return static_cast<HttpHandle>(handle);
}

}  // namespace internal

HttpClient* CreateWinInetClient(const TCHAR*) {
  return new internal::WinInetClient();
}

}  // namespace crash
