/*
 * WebClient.h
 *
 *  Created on: Feb 3, 2020
 *      Author: alapshin
 */

#ifndef WEBCLIENT_H_
#define WEBCLIENT_H_

#include <memory>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include "UrlParser.h"

namespace simple_curl
{
  class WebClient {
  public:
    WebClient(std::string);
    virtual ~WebClient();

    void setFilePath(const char *);
    void setFilePath(const std::string);
    void fetch();
    void flushResponse();

  private:
    int createSocket();
    void setInAddr();
    int resolveAddressIpv4();
    void setNonBlockMode(const bool);
    void connect();
    void sendRequest();
    const std::string buildHttpPacket();
    void receiveResponse();
    size_t getHeaderLenght(const std::string&);
    void setContentLenght(const std::string&);
    const std::string getHtmlBody();
    bool contentLenghtInited();
    void writeToFile();

    int socket_;
    UrlParser url_;
    std::string method_;
    std::string response_;
    size_t content_lenght_;
    std::string file_path_;
    struct sockaddr_in server_ipv4_;
#ifdef CONF_IPv6
  struct             sockaddr_in6 server_ipv6_;
#endif
    unsigned char connection_timeout_;
  };
}
#endif /* WEBCLIENT_H_ */
