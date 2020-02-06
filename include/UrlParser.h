/*
 * UrlParser.h
 *
 *  Created on: Feb 4, 2020
 *      Author: alapshin
 */

#ifndef URLPARSER_H_
#define URLPARSER_H_

#include <string>

class UrlParser
{
public:
    UrlParser();
    UrlParser(std::string url);
    UrlParser(char *url);
    unsigned short getPort();
    std::string getHost();
    std::string getPath();
    void parse(std::string);

    virtual ~UrlParser();
private:
    void init();
#ifdef CONF_HTTPS
    bool isHttps;
#endif
    void setPath();
    void setHost();
    void setProtocol(std::string);
    void setHost(std::string);
    void setPort(std::string);
    void setPath(std::string);

    unsigned short port;
    std::string host;
    std::string path;
};

#endif /* URLPARSER_H_ */
