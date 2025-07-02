//
// Copyright (C) 2019 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cuttlefish/host/libs/web/http_client/http_client.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/core.h>

namespace cuttlefish {
namespace {

bool EqualsCaseInsensitive(const std::string_view a, const std::string_view b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); i++) {
    if (tolower(a[i]) != tolower(b[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace

HttpResponse::HttpResponse(uint64_t http_code, std::vector<HttpHeader> headers)
    : http_code(http_code), headers(std::move(headers)) {}

bool HttpResponse::HttpInfo() const {
  return http_code >= 100 && http_code <= 199;
}
bool HttpResponse::HttpSuccess() const {
  return http_code >= 200 && http_code <= 299;
}
bool HttpResponse::HttpRedirect() const {
  return http_code >= 300 && http_code <= 399;
}
bool HttpResponse::HttpClientError() const {
  return http_code >= 400 && http_code <= 499;
}
bool HttpResponse::HttpServerError() const {
  return http_code >= 500 && http_code <= 599;
}

std::string HttpResponse::StatusDescription() const {
  switch (http_code) {
    case 200:
      return "OK";
    case 201:
      return "Created";
    case 204:
      return "No Content";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "File Not Found";
    case 500:
      return "Internal Server Error";
    case 502:
      return "Bad Gateway";
    case 503:
      return "Service Unavailable";
    default:
      return fmt::format("Status Code: {}", http_code);
  }
}

std::optional<std::string_view> HttpResponse::HeaderValue(
    std::string_view header_name) const {
  for (const HttpHeader& header : headers) {
    if (EqualsCaseInsensitive(header.name, header_name)) {
      return header.value;
    }
  }
  return std::nullopt;
}

HttpClient::~HttpClient() = default;

}  // namespace cuttlefish
