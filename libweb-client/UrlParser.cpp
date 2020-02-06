/*
 * UrlParser.cpp
 *
 *  Created on: Feb 4, 2020
 *      Author: alapshin
 */

#include <algorithm>
#include <iostream>
#include <regex>
#include <map>
#include "UrlParser.h"

using namespace std;

void UrlParser::init()
{
    this->port = 0;
    this->path = "/";
}

UrlParser::UrlParser()
{
    this->init();
}
UrlParser::UrlParser(string url)
{
    this->init();
    this->parse(url);
}

UrlParser::UrlParser(char *url_cstr)
{
    this->init();
    this->parse(string(url_cstr));
}


UrlParser::~UrlParser()
{
}

unsigned short UrlParser::getPort()
{
    return this->port;
}

string UrlParser::getPath()
{
    return this->path;
}

string UrlParser::getHost()
{
    return this->host;
}

void UrlParser::setProtocol(string protocol)
{
    if (protocol.length() == 0)
    {
        this->port = 80;
        return;
    }
    map<string, unsigned short> protocols = {
#ifdef CONF_HTTPS
            {"https", 443},
#endif
            {"http", 80},
    };
    for (pair<string, unsigned short> element : protocols) {
        if (protocol.compare(element.first) == 0)
        {
            this->port = element.second;
            break;
        }
    }

    if (this->port == 0)
    {
        throw std::runtime_error("Present protocol is not supported");
    }
}

void UrlParser::setPort(string port)
{
    if (port.length() == 0)
    {
        return;
    }
    this->port = stoi(port);
}

void UrlParser::setHost(string host)
{
    if (host.length() == 0)
    {
        throw std::runtime_error("Host is not presented in URL");
    }
    this->host = host;
}

void UrlParser::setPath(string path)
{
    if (path.length() > 0)
    {
        this->path = path;
    }
}

void UrlParser::parse(std::string url)
{
    int counter = 0;
    transform(url.begin(), url.end(), url.begin(), ::tolower);
    std::regex url_regex (
    R"(^(([^:\/?#]+):\/\/)?([-_@+a-z.0-9]*):?([0-9]*)?(\/.*)?)",
    std::regex::extended
    );
    smatch url_match_result;

    if (!regex_match(url, url_match_result, url_regex)) {
        throw std::runtime_error("URL is not valid");
    }

    for (const auto& res : url_match_result) {
        switch (counter++)
        {
        case 2:
            this->setProtocol(res);
            break;
        case 3:
            this->setHost(res);
            break;
        case 4:
            this->setPort(res);
            break;
        case 5:
            this->setPath(res);
            break;
        default:
            continue;
        }
    }
}
