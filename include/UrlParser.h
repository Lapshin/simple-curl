/*
 * UrlParser.h
 *
 *  Created on: Feb 4, 2020
 *      Author: alapshin
 */

#ifndef URLPARSER_H_
#define URLPARSER_H_

#include <string>
#include <regex>

namespace simple_curl
{
class UrlParser {
 public:
  UrlParser();
  UrlParser(std::string url);
  UrlParser(char *url);

  unsigned short       getPort();
  const std::string    getHost();
  const std::string    getPath();

  virtual ~UrlParser();
 private:
#ifdef CONF_HTTPS
  bool is_https_;
#endif
  void parse(std::string);
  void setPath();
  void setHost();
  void setProtocol(std::string);
  void setHost(std::string);
  void setPort(std::string);
  void setPath(std::string);

  unsigned short port_;
  std::string    host_;
  std::string    path_;
  std::regex     url_regex_;
};
}
#endif /* URLPARSER_H_ */
