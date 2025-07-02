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

#pragma once

#include <stdint.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cuttlefish/common/libs/utils/result.h"

namespace cuttlefish {

struct HttpVoidResponse {};

struct HttpHeader {
  std::string name;
  std::string value;
};

template <typename T>
struct TypedHttpResponse;

struct HttpResponse {
  HttpResponse() = default;
  explicit HttpResponse(uint64_t http_code, std::vector<HttpHeader> headers);

  bool HttpInfo() const;
  bool HttpSuccess() const;
  bool HttpRedirect() const;
  bool HttpClientError() const;
  bool HttpServerError() const;

  std::string StatusDescription() const;

  std::optional<std::string_view> HeaderValue(
      std::string_view header_name) const;

  template <typename T>
  TypedHttpResponse<T> WithData(T data);

  long http_code;
  std::vector<HttpHeader> headers;
};

template <typename T>
struct TypedHttpResponse : public HttpResponse {
  TypedHttpResponse(HttpResponse response, T data)
      : HttpResponse(std::move(response)), data(std::move(data)) {}

  T data;
};

template <typename T>
TypedHttpResponse<T> HttpResponse::WithData(T data) {
  return TypedHttpResponse(*this, std::move(data));
}

enum class HttpMethod {
  kGet,
  kPost,
  kDelete,
  kHead,
};

struct HttpRequest {
  HttpMethod method;
  std::string url;
  std::vector<std::string> headers;
  std::string data_to_write;
};

class HttpClient {
 public:
  typedef std::function<bool(char*, size_t)> DataCallback;

  virtual ~HttpClient();

  // Returns response's status code.
  virtual Result<HttpResponse> DownloadToCallback(HttpRequest,
                                                  DataCallback callback) = 0;
};

}  // namespace cuttlefish
