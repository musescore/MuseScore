#include "common/windows/symbol_collector_client.h"

#include <stdio.h>

#include <regex>

#include "common/windows/http_upload.h"

namespace google_breakpad {

  // static
  bool SymbolCollectorClient::CreateUploadUrl(
      wstring& api_url,
      wstring& api_key,
      UploadUrlResponse *uploadUrlResponse) {
    wstring url = api_url +
        L"/v1/uploads:create"
        L"?key=" + api_key;
    wstring response;
    int response_code;

    if (!HTTPUpload::SendSimplePostRequest(
        url,
        L"",
        L"",
        NULL,
        &response,
        &response_code)) {
      wprintf(L"Failed to create upload url.\n");
      wprintf(L"Response code: %ld\n", response_code);
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return false;
    }

    // Note camel-case rather than underscores.
    std::wregex upload_url_regex(L"\"uploadUrl\": \"([^\"]+)\"");
    std::wregex upload_key_regex(L"\"uploadKey\": \"([^\"]+)\"");

    std::wsmatch upload_url_match;
    if (!std::regex_search(response, upload_url_match, upload_url_regex) ||
        upload_url_match.size() != 2) {
      wprintf(L"Failed to parse create url response.");
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return false;
    }
    wstring upload_url = upload_url_match[1].str();

    std::wsmatch upload_key_match;
    if (!std::regex_search(response, upload_key_match, upload_key_regex) ||
        upload_key_match.size() != 2) {
      wprintf(L"Failed to parse create url response.");
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return false;
    }
    wstring upload_key = upload_key_match[1].str();

    uploadUrlResponse->upload_url = upload_url;
    uploadUrlResponse->upload_key = upload_key;
    return true;
  }

  // static
  CompleteUploadResult SymbolCollectorClient::CompleteUpload(
      wstring& api_url,
      wstring& api_key,
      const wstring& upload_key,
      const wstring& debug_file,
      const wstring& debug_id) {
    wstring url = api_url +
        L"/v1/uploads/" + upload_key + L":complete"
        L"?key=" + api_key;
    wstring body =
        L"{ symbol_id: {"
        L"debug_file: \"" + debug_file + L"\", "
        L"debug_id: \"" + debug_id + L"\" "
        L"} }";
    wstring response;
    int response_code;

    if (!HTTPUpload::SendSimplePostRequest(
        url,
        body,
        L"application/json",
        NULL,
        &response,
        &response_code)) {
      wprintf(L"Failed to complete upload.\n");
      wprintf(L"Response code: %ld\n", response_code);
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return CompleteUploadResult::Error;
    }

    std::wregex result_regex(L"\"result\": \"([^\"]+)\"");
    std::wsmatch result_match;
    if (!std::regex_search(response, result_match, result_regex) ||
        result_match.size() != 2) {
      wprintf(L"Failed to parse complete upload response.");
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return CompleteUploadResult::Error;
    }
    wstring result = result_match[1].str();

    if (result.compare(L"DUPLICATE_DATA") == 0) {
      return CompleteUploadResult::DuplicateData;
    }

    return CompleteUploadResult::Ok;
  }

  // static
  SymbolStatus SymbolCollectorClient::CheckSymbolStatus(
      wstring& api_url,
      wstring& api_key,
      const wstring& debug_file,
      const wstring& debug_id) {
    wstring response;
    int response_code;
    wstring url = api_url +
        L"/v1/symbols/" + debug_file + L"/" + debug_id + L":checkStatus"
        L"?key=" + api_key;

    if (!HTTPUpload::SendGetRequest(
        url,
        NULL,
        &response,
        &response_code)) {
      wprintf(L"Failed to check symbol status.\n");
      wprintf(L"Response code: %ld\n", response_code);
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return SymbolStatus::Unknown;
    }

    std::wregex status_regex(L"\"status\": \"([^\"]+)\"");
    std::wsmatch status_match;
    if (!std::regex_search(response, status_match, status_regex) ||
        status_match.size() != 2) {
      wprintf(L"Failed to parse check symbol status response.");
      wprintf(L"Response:\n");
      wprintf(L"%s\n", response.c_str());
      return SymbolStatus::Unknown;
    }
    wstring status = status_match[1].str();

    return (status.compare(L"FOUND") == 0) ?
      SymbolStatus::Found :
      SymbolStatus::Missing;
  }

}  // namespace google_breakpad