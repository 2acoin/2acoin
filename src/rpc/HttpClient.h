// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "serialization/SerializationTools.h"

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <memory>
#include <system/TcpConnection.h>
#include <system/TcpStream.h>
#include <version.h>

namespace CryptoNote
{
    class ConnectException : public std::runtime_error
    {
      public:
        ConnectException(const std::string &whatArg);
    };

    class HttpClient
    {
      public:
        HttpClient(System::Dispatcher &dispatcher, const std::string &address, uint16_t port);

        ~HttpClient();

        void request(const HttpRequest &req, HttpResponse &res);

        bool isConnected() const;

      private:
        void connect();

        void disconnect();

        const std::string m_address;

        const uint16_t m_port;

        bool m_connected = false;

        System::Dispatcher &m_dispatcher;

        System::TcpConnection m_connection;

        std::unique_ptr<System::TcpStreambuf> m_streamBuf;

        /* Don't send two requests at once */
        std::mutex m_mutex;
    };

    template<typename Request, typename Response>
    void invokeJsonCommand(HttpClient &client, const std::string &url, const std::string &method, const Request &req, Response &res)
    {
        HttpRequest hreq;
        HttpResponse hres;

        hreq.addHeader("Content-Type", "application/json");

        std::stringstream userAgent;

        userAgent << "NodeRpcProxy/" << PROJECT_VERSION_LONG;

        hreq.addHeader("User-Agent", userAgent.str());

        hreq.setUrl(url);

        hreq.setMethod(method);

        if (method == "POST")
        {
            hreq.setBody(storeToJson(req));
        }

        client.request(hreq, hres);

        if (hres.getStatus() != HttpResponse::STATUS_200)
        {
            throw std::runtime_error("HTTP status: " + std::to_string(hres.getStatus()));
        }

        if (!loadFromJson(res, hres.getBody()))
        {
            throw std::runtime_error("Failed to parse JSON response");
        }
    }

    template<typename Request, typename Response>
    void invokeBinaryCommand(HttpClient &client, const std::string &url, const Request &req, Response &res)
    {
        HttpRequest hreq;
        HttpResponse hres;

        std::stringstream userAgent;

        userAgent << "NodeRpcProxy/" << PROJECT_VERSION_LONG;

        hreq.addHeader("User-Agent", userAgent.str());

        hreq.setUrl(url);
        hreq.setBody(storeToBinaryKeyValue(req));
        client.request(hreq, hres);

        if (!loadFromBinaryKeyValue(res, hres.getBody()))
        {
            throw std::runtime_error("Failed to parse binary response");
        }
    }

} // namespace CryptoNote
