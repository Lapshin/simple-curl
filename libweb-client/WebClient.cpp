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

WebClient::WebClient(string url)
{
    this->url = UrlParser(url);
    this->filePath = "";
    this->socket = -1;
    this->serverIpv4.sin_family = AF_INET;
    this->serverIpv4.sin_port = htons(this->url.getPort());
    this->connectionTimeout = 3;
    this->method = "GET";
    this->contentLenght = -1ULL;
}

WebClient::~WebClient()
{
    if (this->socket != -1)
    {
        int val = 1;
        setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        ::close(this->socket);
        this->socket = -1;
    }
}

bool WebClient::contentLenghtInited()
{
    return (this->contentLenght == -1ULL);
}

void WebClient::setFilePath(char *path)
{
    if (path == NULL)
    {
        return;
    }
    this->filePath = path;
    return;
}

void WebClient::setFilePath(string path)
{
    this->filePath = path;
    return;
}

void WebClient::fetch(void)
{
    this->createSocket();
    this->setInAddr();
    this->connect();
    this->sendRequest();
    this->receiveResponse();
}

string WebClient::buildHttpPacket()
{
    string packet = this->method + " " + this->url.getPath() + " HTTP/1.1\r\n";


    packet += "Host: " + this->url.getHost() + "\r\n";
    packet += "User-Agent: simple-curl/1.0\r\n";
    packet += "Accept: */*\r\n";
    packet += "\r\n";
    return packet;
}

size_t WebClient::getHeaderLenght(string &body)
{
    string line;
    istringstream f(body);
    size_t headerLenght = 0 ;
    while (std::getline(f, line)) {
        headerLenght += line.size() + 1;
        if (line.compare(0, 1, "\r") == 0)
        {
            return headerLenght;
        }
    }

    return -1;
}

void WebClient::setContentLenght(string &body)
{
#define CONTENT_LENGHT "content-length: "
    string line;
    istringstream f(body);
    while (std::getline(f, line)) {
        transform(line.begin(), line.end(), line.begin(), ::tolower);
        if (line.compare(0, sizeof(CONTENT_LENGHT) - 1, CONTENT_LENGHT) == 0)
        {
             long long length = stoll(&line[sizeof(CONTENT_LENGHT) - 1]);
             if (length < 0 && ((unsigned long long)length >= numeric_limits<size_t>::max() -1))
             {
                 throw std::runtime_error("Content-length is out of range");
             }
             this->contentLenght = length;
            return;
        }
    }

    throw std::runtime_error("Can not find Content-length in headers");
}

void WebClient::receiveResponse()
{
#define BUFFER_SIZE 10
    char buffer[BUFFER_SIZE];
    long long headerLenght = -1;
    int ret = -1;

    do
    {
        memset(buffer, 0, sizeof(buffer));
        errno = 0;
        ret = recv(this->socket, buffer, sizeof(buffer) - 1, 0);
        if (ret < 0)
        {
            throw std::runtime_error("Response receiving failed");
        }
        else if (ret == 0)
        {
            throw std::runtime_error("recv() returned 0. Connection closed");
        }
        if ((this->response.size() + ret) < this->response.size())
        {
            throw std::runtime_error("Too big response");
        }

        this->response += buffer;
        if (headerLenght == -1)
        {
            headerLenght = this->getHeaderLenght(this->response);
        }
        if (headerLenght > 0 && this->contentLenghtInited())
        {
            this->setContentLenght(this->response);
        }
    }
    while((headerLenght > 0 && this->contentLenghtInited()) ||
            this->response.size() < (size_t)(headerLenght + this->contentLenght));
}


void WebClient::sendRequest()
{
    string data = this->buildHttpPacket();
    int length = strlen(data.c_str());
    int ret = send(this->socket, data.c_str(), length, 0);
    if (ret < 0)
    {
        throw std::runtime_error("Can not send request");
    }
    if (ret != length)
    {
        throw std::runtime_error("Request was sent partly");
    }
}

void WebClient::setNonBlockMode(bool isNonBlocking)
{
    int arg = fcntl(this->socket, F_GETFL, NULL);

    if (arg < 0)
    {
         throw ("Error fcntl(..., F_GETFL)");
    }

    if (isNonBlocking == true)
    {
        arg |= O_NONBLOCK;
    }
    else
    {
        arg &= (~O_NONBLOCK);
    }

    if (fcntl(this->socket, F_SETFL, arg) < 0)
    {
        throw ("Error fcntl(..., F_SETFL)");
    }
}

void WebClient::connect()
{
    int ret = 0;
    struct timeval tv = {this->connectionTimeout, 0};
    struct sockaddr *addr = (struct sockaddr*) &this->serverIpv4;
    size_t addr_size = sizeof(this->serverIpv4);
    fd_set fdset;

    errno = 0;
    ret = setsockopt(this->socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv);
    if (ret != 0)
    {
        throw std::runtime_error("Error setting receive timeout on socket");
    }

    this->setNonBlockMode(true);
    ::connect(this->socket, addr, addr_size); // Check error on select

    FD_ZERO(&fdset);
    FD_SET(this->socket, &fdset);

    errno = 0;
    ret = select(this->socket + 1, NULL, &fdset, NULL, &tv);
    if (ret > 0)
    {
        int optval;
        socklen_t optval_len = sizeof(optval);

        errno = 0;
        ret = getsockopt(this->socket, SOL_SOCKET, SO_ERROR, &optval, &optval_len);
        if (ret == 0 && optval != 0)
        {
            switch (optval)
            {
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
                throw std::runtime_error("Connection failed: unhandled getsockopt reason");
                break;
            }
        }
        else if (ret != 0)
        {
            throw std::runtime_error("Error getting getsockopt SO_ERROR");
        }
    }
    else
    {
        throw std::runtime_error("Max connect timeout reached");
    }
    this->setNonBlockMode(false);
}

string WebClient::getHtmlBody()
{
    if (this->contentLenghtInited() || (this->response.length()) < this->contentLenght)
    {
        return "";
    }
    return &(this->response[this->response.length() - this->contentLenght]);
}

void WebClient::writeToFile()
{
    ofstream file;
    file.open(this->filePath, ofstream::out);
    if (file.fail())
    {
        file.close();
        throw std::runtime_error("Can not open file for writing");
    }

    file << this->getHtmlBody();
    file.close();
}

void WebClient::flushResponse()
{
    if (this->filePath.empty())
    {
        cout << this->getHtmlBody() << flush;
        return;
    }
    this->writeToFile();
}

int WebClient::createSocket()
{
    if (this->socket == -1)
    {
        this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (this->socket == -1)
        {
            throw std::runtime_error("Can not open socket");
        }
    }

    return this->socket;
}

void WebClient::setInAddr()
{
    in_addr_t in_addr = inet_addr(url.getHost().c_str());
    in_addr = (in_addr == -1U) ? this->resolveAddressIpv4() : in_addr;
    if (in_addr == -1U)
    {
        throw std::runtime_error("Can not get IP address");
    }
    this->serverIpv4.sin_addr.s_addr = in_addr;
}

int WebClient::resolveAddressIpv4()
{
    in_addr_t in_addr = -1U;
    struct hostent *he = NULL;
    struct in_addr **addr_list = NULL;

    if ((he = gethostbyname(this->url.getHost().c_str())) == NULL)
    {
        throw std::runtime_error("Failed to resolve hostname");
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++)
    {
        in_addr = addr_list[i]->s_addr;
        break; // TODO useful if you want to get all IPs
    }
    return in_addr;
}
