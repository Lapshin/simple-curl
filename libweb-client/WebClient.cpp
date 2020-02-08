/*
 * WebClient.cpp
 *
 *  Created on: Feb 3, 2020
 *      Author: alapshin
 */

#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include "WebClient.h"

using namespace std;
namespace simple_curl
{
WebClient::WebClient(string url)
    : socket_(-1),
      url_(url),
      method_("GET"),
      response_(""),
      content_lenght_(-1ULL),
      file_path_(""),
      server_ipv4_( { AF_INET, htons(url_.getPort()), { 0 }, { 0 } }),
      connection_timeout_(3) {
}

WebClient::~WebClient() {
  if (socket_ != -1) {
    int val = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    ::close(socket_);
    socket_ = -1;
  }
}

bool WebClient::contentLenghtInited() {
  return (content_lenght_ != -1ULL);
}

void WebClient::setFilePath(const char *path) {
  if (path == NULL) {
    return;
  }
  file_path_ = path;
  return;
}

void WebClient::setFilePath(const string path) {
  file_path_ = path;
  return;
}

void WebClient::fetch() {
  createSocket();
  setInAddr();
  connect();
  sendRequest();
  receiveResponse();
}

const string WebClient::buildHttpPacket() {
  string packet = method_ + " " + url_.getPath() + " HTTP/1.1\r\n";

  packet += "Host: " + url_.getHost() + "\r\n";
  packet += "User-Agent: simple-curl/1.0\r\n";
  packet += "Accept: */*\r\n";
  packet += "\r\n";
  return packet;
}

size_t WebClient::getHeaderLenght(const string &body) {
  string line;
  istringstream f(body);
  size_t headerLenght = 0;
  while (std::getline(f, line)) {
    headerLenght += line.size() + 1;
    if (line.compare(0, 1, "\r") == 0) {
      return headerLenght;
    }
  }

  return -1;
}

void WebClient::setContentLenght(const string &body) {
#define CONTENT_LENGHT "content-length: "
  string line;
  istringstream f(body);
  while (std::getline(f, line)) {
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    if (line.compare(0, sizeof(CONTENT_LENGHT) - 1, CONTENT_LENGHT) == 0) {
      long long length = stoll(&line[sizeof(CONTENT_LENGHT) - 1]);
      if (length < 0
          && ((unsigned long long) length >= numeric_limits<size_t>::max() - 1)) {
        throw std::runtime_error("Content-length is out of range");
      }
      content_lenght_ = length;
      return;
    }
  }

  throw std::runtime_error("Can not find Content-length in headers");
}

void WebClient::receiveResponse() {
#define BUFFER_SIZE 512
  char buffer[BUFFER_SIZE];
  size_t headerLenght = -1ULL;
  int ret = -1;

  do {
    memset(buffer, 0, sizeof(buffer));
    errno = 0;
    ret = recv(socket_, buffer, sizeof(buffer) - 1, 0);
    if (ret < 0) {
      throw std::runtime_error("Response receiving failed");
    } else if (ret == 0) {
      throw std::runtime_error("recv() returned 0. Connection closed");
    }
    if ((response_.size() + ret) < response_.size()) {
      throw std::runtime_error("Too big response");
    }

    response_ += buffer;
    if (headerLenght == -1ULL) {
      headerLenght = getHeaderLenght(response_);
    } else if (!contentLenghtInited()) {
      setContentLenght(response_);
    }
  } while (!contentLenghtInited()
      || response_.size() < (size_t) (headerLenght + content_lenght_));
}

void WebClient::sendRequest() {
  const string data = buildHttpPacket();
  const int length = strlen(data.c_str());
  int ret = send(socket_, data.c_str(), length, 0);
  if (ret < 0) {
    throw std::runtime_error("Can not send request");
  }
  if (ret != length) {
    throw std::runtime_error("Request was sent partly");
  }
}

void WebClient::setNonBlockMode(const bool isNonBlocking) {
  int arg = fcntl(socket_, F_GETFL, NULL);

  if (arg < 0) {
    throw("Error fcntl(..., F_GETFL)");
  }

  if (isNonBlocking == true) {
    arg |= O_NONBLOCK;
  } else {
    arg &= (~O_NONBLOCK);
  }

  if (fcntl(socket_, F_SETFL, arg) < 0) {
    throw("Error fcntl(..., F_SETFL)");
  }
}

void WebClient::connect() {
  int ret = 0;
  struct timeval tv = { connection_timeout_, 0 };
  struct sockaddr *addr = (struct sockaddr*) &server_ipv4_;
  size_t addr_size = sizeof(server_ipv4_);
  fd_set fdset;

  errno = 0;
  ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const void*) &tv,
                   sizeof(tv));
  if (ret != 0) {
    throw std::runtime_error("Error setting receive timeout on socket");
  }

  setNonBlockMode(true);
  ::connect(socket_, addr, addr_size);  // Check error on select

  FD_ZERO(&fdset);
  FD_SET(socket_, &fdset);

  errno = 0;
  ret = select(socket_ + 1, NULL, &fdset, NULL, &tv);
  if (ret > 0) {
    int optval;
    socklen_t optval_len = sizeof(optval);

    errno = 0;
    ret = getsockopt(socket_, SOL_SOCKET, SO_ERROR, &optval, &optval_len);
    if (ret == 0 && optval != 0) {
      switch (optval) {
        case ECONNREFUSED:
          throw std::runtime_error("Connection refused");
          break;
        case EHOSTDOWN:
          throw std::runtime_error("Host is down");
          break;
        case EHOSTUNREACH:
          throw std::runtime_error("No route to host");
          break;
        default:
          throw std::runtime_error(
              "Connection failed: unhandled getsockopt reason");
          break;
      }
    } else if (ret != 0) {
      throw std::runtime_error("Error getting getsockopt SO_ERROR");
    }
  } else {
    throw std::runtime_error("Max connect timeout reached");
  }
  setNonBlockMode(false);
}

const string WebClient::getHtmlBody() {
  if (!contentLenghtInited() || (response_.length()) < content_lenght_) {
    return "";
  }
  return &(response_[response_.length() - content_lenght_]);
}

void WebClient::writeToFile() {
  ofstream file;
  file.open(file_path_, ofstream::out);
  if (file.fail()) {
    file.close();
    throw std::runtime_error("Can not open file for writing");
  }

  file << getHtmlBody();
  file.close();
}

void WebClient::flushResponse() {
  if (file_path_.empty()) {
    cout << getHtmlBody() << flush;
    return;
  }
  writeToFile();
}

int WebClient::createSocket() {
  if (socket_ == -1) {
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == -1) {
      throw std::runtime_error("Can not open socket");
    }
  }

  return socket_;
}

void WebClient::setInAddr() {
  in_addr_t in_addr = inet_addr(url_.getHost().c_str());
  in_addr = (in_addr == -1U) ? resolveAddressIpv4() : in_addr;
  if (in_addr == -1U) {
    throw std::runtime_error("Can not get IP address");
  }
  server_ipv4_.sin_addr.s_addr = in_addr;
}

int WebClient::resolveAddressIpv4() {
  in_addr_t in_addr = -1U;
  struct hostent *he = NULL;
  struct in_addr **addr_list = NULL;

  if ((he = gethostbyname(url_.getHost().c_str())) == NULL) {
    throw std::runtime_error("Failed to resolve hostname");
  }

  addr_list = (struct in_addr **) he->h_addr_list;

  for (int i = 0; addr_list[i] != NULL; i++) {
    in_addr = addr_list[i]->s_addr;
    break;  // TODO useful if you want to get all IPs
  }
  return in_addr;
}
}
