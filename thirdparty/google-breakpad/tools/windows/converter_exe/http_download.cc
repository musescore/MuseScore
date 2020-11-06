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

#include <assert.h>
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>

#include <vector>

#include "tools/windows/converter_exe/http_download.h"
#include "tools/windows/converter_exe/winhttp_client.h"
#include "tools/windows/converter_exe/wininet_client.h"

namespace crash {
static const std::vector<char>::size_type kVectorChunkSize = 4096;  // 4 KB

using std::vector;

// Class that atuo closes the contained HttpHandle when the object
// goes out of scope.
class AutoHttpHandle {
 public:
  AutoHttpHandle() : handle_(NULL) {}
  explicit AutoHttpHandle(HttpHandle handle) : handle_(handle) {}
  ~AutoHttpHandle() {
    if (handle_) {
      InternetCloseHandle(handle_);
    }
  }

  HttpHandle get() { return handle_; }
  HttpHandle* get_handle_addr () { return &handle_; }

 private:
  HttpHandle handle_;
};

// Template class for auto releasing the contained pointer when
// the object goes out of scope.
template<typename T>
class AutoPtr {
 public:
  explicit AutoPtr(T* ptr) : ptr_(ptr) {}
  ~AutoPtr() {
    if (ptr_) {
      delete ptr_;
    }
  }

  T* get() { return ptr_; }
  T* operator -> () { return ptr_; }

 private:
  T* ptr_;
};

// CheckParameters ensures that the parameters in |parameters| are safe for
// use in an HTTP URL.  Returns true if they are, false if unsafe characters
// are present.
static bool CheckParameters(const map<wstring, wstring>* parameters) {
  for (map<wstring, wstring>::const_iterator iterator = parameters->begin();
       iterator != parameters->end();
       ++iterator) {
    const wstring& key = iterator->first;
    if (key.empty()) {
      // Disallow empty parameter names.
      return false;
    }
    for (unsigned int i = 0; i < key.size(); ++i) {
      wchar_t c = key[i];
      if (c < 32 || c == '"' || c == '?' || c == '&' || c > 127) {
        return false;
      }
    }

    const wstring& value = iterator->second;
    for (unsigned int i = 0; i < value.size(); ++i) {
      wchar_t c = value[i];
      if (c < 32 || c == '"' || c == '?' || c == '&' || c > 127) {
        return false;
      }
    }
  }

  return true;
}

HttpClient* HTTPDownload::CreateHttpClient(const wchar_t* url) {
  const TCHAR* kHttpApiPolicyEnvironmentVariable = TEXT("USE_WINHTTP");
  TCHAR buffer[2] = {0};
  HttpClient* http_client = NULL;

  if (::GetEnvironmentVariable(kHttpApiPolicyEnvironmentVariable,
                               buffer,
                               sizeof(buffer)/sizeof(buffer[0])) > 0) {
    fprintf(stdout,
            "Environment variable [%ws] is set, use WinHttp\n",
            kHttpApiPolicyEnvironmentVariable);
    http_client = CreateWinHttpClient(url);
    if (http_client == NULL) {
      fprintf(stderr, "WinHttpClient not created, Is the protocol HTTPS? "
                      "Fall back to WinInet API.\n");
    }
  } else {
    fprintf(stderr,
            "Environment variable [%ws] is NOT set, use WinInet API\n",
            kHttpApiPolicyEnvironmentVariable);
  }

  if (http_client == NULL) {
    return CreateWinInetClient(url);
  }

  return http_client;
}

// static
bool HTTPDownload::Download(const wstring& url,
                            const map<wstring, wstring>* parameters,
                            string *content, int *status_code) {
  assert(content);
  AutoPtr<HttpClient> http_client(CreateHttpClient(url.c_str()));

  if (!http_client.get()) {
    fprintf(stderr, "Failed to create any http client.\n");
    return false;
  }

  if (status_code) {
    *status_code = 0;
  }

  wchar_t scheme[16] = {0};
  wchar_t host[256] = {0};
  wchar_t path[256] = {0};
  int port = 0;
  if (!http_client->CrackUrl(url.c_str(),
                             0,
                             scheme,
                             sizeof(scheme)/sizeof(scheme[0]),
                             host,
                             sizeof(host)/sizeof(host[0]),
                             path,
                             sizeof(path)/sizeof(path[0]),
                             &port)) {
    fprintf(stderr,
            "HTTPDownload::Download: InternetCrackUrl: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }

  bool secure = false;
  if (_wcsicmp(scheme, L"https") == 0) {
    secure = true;
  } else if (wcscmp(scheme, L"http") != 0) {
    fprintf(stderr,
            "HTTPDownload::Download: scheme must be http or https for %ws\n",
            url.c_str());
    return false;
  }

  AutoHttpHandle internet;
  if (!http_client->Open(NULL,  // user agent
                         HttpClient::ACCESS_TYPE_PRECONFIG,
                         NULL,  // proxy name
                         NULL,  // proxy bypass
                         internet.get_handle_addr())) {
    fprintf(stderr,
            "HTTPDownload::Download: Open: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }

  AutoHttpHandle connection;
  if (!http_client->Connect(internet.get(),
                            host,
                            port,
                            connection.get_handle_addr())) {
    fprintf(stderr,
            "HTTPDownload::Download: InternetConnect: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }

  wstring request_string = path;
  if (parameters) {
    // TODO(mmentovai): escape bad characters in parameters instead of
    // forbidding them.
    if (!CheckParameters(parameters)) {
      fprintf(stderr,
              "HTTPDownload::Download: invalid characters in parameters\n");
      return false;
    }

    bool added_parameter = false;
    for (map<wstring, wstring>::const_iterator iterator = parameters->begin();
         iterator != parameters->end();
         ++iterator) {
      request_string.append(added_parameter ? L"&" : L"?");
      request_string.append(iterator->first);
      request_string.append(L"=");
      request_string.append(iterator->second);
      added_parameter = true;
    }
  }

  AutoHttpHandle request;
  if (!http_client->OpenRequest(connection.get(),
                                L"GET",
                                request_string.c_str(),
                                NULL,    // version
                                NULL,    // referer
                                secure,
                                request.get_handle_addr())) {
    fprintf(stderr,
            "HttpClient::OpenRequest: error %lu for %ws, request: %ws\n",
            GetLastError(), url.c_str(), request_string.c_str());
    return false;
  }

  if (!http_client->SendRequest(request.get(), NULL, 0)) {
    fprintf(stderr,
            "HttpClient::SendRequest: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }

  if (!http_client->ReceiveResponse(request.get())) {
    fprintf(stderr,
            "HttpClient::ReceiveResponse: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }

  int http_status = 0;
  if (!http_client->GetHttpStatusCode(request.get(), &http_status)) {
    fprintf(stderr,
            "HttpClient::GetHttpStatusCode: error %lu for %ws\n",
            GetLastError(), url.c_str());
    return false;
  }
  if (http_status != 200) {
    fprintf(stderr,
            "HTTPDownload::Download: HTTP status code %d for %ws\n",
            http_status, url.c_str());
    return false;
  }

  DWORD content_length = 0;
  vector<char>::size_type buffer_size = 0;
  http_client->GetContentLength(request.get(), &content_length);
  if (content_length == HttpClient::kUnknownContentLength) {
    buffer_size = kVectorChunkSize;
  } else {
    buffer_size = content_length;
  }

  if (content_length != 0) {
    vector<char> response_buffer = vector<char>(buffer_size+1);
    DWORD size_read;
    DWORD total_read = 0;
    bool read_result;
    do {
      if (content_length == HttpClient::kUnknownContentLength
          && buffer_size == total_read) {
        // The content length wasn't specified in the response header, so we
        // have to keep growing the buffer until we're done reading.
        buffer_size += kVectorChunkSize;
        response_buffer.resize(buffer_size);
      }
      read_result = !!http_client->ReadData(
          request.get(),
          &response_buffer[total_read],
          static_cast<DWORD>(buffer_size) - total_read,
          &size_read);
      total_read += size_read;
    } while (read_result && (size_read != 0));

    if (!read_result) {
      fprintf(stderr,
              "HttpClient::ReadData: error %lu for %ws\n",
              GetLastError(),
              url.c_str());
      return false;
    } else if (size_read != 0) {
      fprintf(stderr,
              "HttpClient::ReadData: error %lu/%lu for %ws\n",
              total_read,
              content_length,
              url.c_str());
      return false;
    }
    content->assign(&response_buffer[0], total_read);
  } else {
    content->clear();
  }
  return true;
}

}  // namespace crash
