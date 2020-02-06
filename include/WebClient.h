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

class WebClient
{
public:
    WebClient(std::string);
    virtual ~WebClient();

    void setFilePath(char *);
    void setFilePath(std::string);
    void fetch(void);
    void flushResponse();

private:
    int createSocket();
    void setInAddr();
    int resolveAddressIpv4();
    void setNonBlockMode(bool);
    void connect();
    void sendRequest();
    std::string buildHttpPacket();
    void receiveResponse();
    size_t getHeaderLenght(std::string&);
    void setContentLenght(std::string&);
    std::string getHtmlBody();
    void writeToFile();
    bool contentLenghtInited();

    int socket;
    UrlParser url;
    std::string method;
    std::string response;
    size_t contentLenght;
    std::string filePath;
    struct sockaddr_in serverIpv4;
#ifdef CONF_IPv6
    struct sockaddr_in6 serverIpv6;
#endif
    unsigned char connectionTimeout;
};

#endif /* WEBCLIENT_H_ */
