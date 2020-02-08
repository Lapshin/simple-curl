/*
 * UrlParser.cpp
 *
 *  Created on: Feb 4, 2020
 *      Author: alapshin
 */

#include <algorithm>
#include <iostream>
#include <map>
#include "UrlParser.h"

using namespace std;
namespace simple_curl
{
UrlParser::UrlParser()
    :
#ifdef CONF_HTTPS
  is_https_(false),
#endif
      port_(0),
      path_("/"),
      url_regex_(R"(^(([^:\/?#]+):\/\/)?([-_@+a-z.0-9]*):?([0-9]*)?(\/.*)?)",
                 std::regex::extended) {
}

UrlParser::UrlParser(string url)
    : UrlParser() {
  UrlParser();
  parse(url);
}

UrlParser::UrlParser(char *url_cstr)
    : UrlParser() {
  UrlParser();
  parse(string(url_cstr));
}

UrlParser::~UrlParser() {
}

unsigned short UrlParser::getPort() {
  return port_;
}

const string UrlParser::getPath() {
  return path_;
}

const string UrlParser::getHost() {
  return host_;
}

void UrlParser::setProtocol(string protocol) {
  if (protocol.length() == 0) {
    port_ = 80;
    return;
  }
  map<string, unsigned short> protocols = {
#ifdef CONF_HTTPS
      {"https", 443},
#endif
      { "http", 80 }, };
  for (pair<string, unsigned short> element : protocols) {
    if (protocol.compare(element.first) == 0) {
      port_ = element.second;
      break;
    }
  }

  if (port_ == 0) {
    throw std::runtime_error("Present protocol is not supported");
  }
}

void UrlParser::setPort(string port) {
  if (port.length() == 0) {
    return;
  }
  port_ = stoi(port);
}

void UrlParser::setHost(string host) {
  if (host.length() == 0) {
    throw std::runtime_error("Host is not presented in URL");
  }
  host_ = host;
}

void UrlParser::setPath(string path) {
  if (path.length() > 0) {
    path_ = path;
  }
}

void UrlParser::parse(std::string url) {
  int counter = 0;
  transform(url.begin(), url.end(), url.begin(), ::tolower);
  smatch url_match_result;

  if (!regex_match(url, url_match_result, url_regex_)) {
    throw std::runtime_error("URL is not valid");
  }

  for (const auto& res : url_match_result) {
    switch (counter++) {
      case 2:
        setProtocol(res);
        break;
      case 3:
        setHost(res);
        break;
      case 4:
        setPort(res);
        break;
      case 5:
        setPath(res);
        break;
      default:
        continue;
    }
  }
}
}
