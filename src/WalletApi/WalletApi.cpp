// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <atomic>

#include <Common/SignalHandler.h>

#include <config/CliHeader.h>

#include <iostream>

#include <thread>

#include <WalletApi/ApiDispatcher.h>
#include <WalletApi/ParseArguments.h>

int main(int argc, char **argv)
{
    Config config = parseArguments(argc, argv);

    std::cout << CryptoNote::getProjectCLIHeader() << std::endl;

    std::thread apiThread;

    std::atomic<bool> ctrl_c(false);

    std::shared_ptr<ApiDispatcher> api(nullptr);

    try
    {
        /* Trigger the shutdown signal if ctrl+c is used */
        Tools::SignalHandler::install([&ctrl_c] { ctrl_c = true; });

        /* Init the API */
        api = std::make_shared<ApiDispatcher>(
            config.port, config.rpcBindIp, config.rpcPassword,
            config.corsHeader
        );

        /* Launch the API */
        apiThread = std::thread(&ApiDispatcher::start, api.get());

        std::cout << "Want documentation on how to use the wallet-api?\n"
                     "See https://www.futuregadget.xyz/api-docs/\n\n";

        std::string address = "http://" + config.rpcBindIp + ":" + std::to_string(config.port);

        std::cout << "The api has been launched on " << address
                  << ".\nType exit to save and shutdown." << std::endl;

        while (!ctrl_c)
        {
            std::string input;

            if (!std::getline(std::cin, input) || input == "exit" || input == "quit")
            {
                break;
            }

            if (input == "help")
            {
                std::cout << "Type exit to save and shutdown." << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Unexpected error: " << e.what()
                  << "\nPlease report this error, and what you were doing to "
                     "cause it.\n";
    }

    std::cout << ("\nSaving and shutting down...\n");

    if (api != nullptr)
    {
        api->stop();
    }

    if (apiThread.joinable())
    {
        apiThread.join();
    }
}
