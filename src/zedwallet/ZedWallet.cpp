// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////
#include <zedwallet/ZedWallet.h>
////////////////////////////////

#include <boost/algorithm/string.hpp>

#include <Common/SignalHandler.h>
#include <Common/StringTools.h>

#include <CryptoNoteCore/Currency.h>

#include <Logging/FileLogger.h>
#include <Logging/LoggerManager.h>

#include <NodeRpcProxy/NodeRpcProxy.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "version.h"

#ifdef _WIN32
/* Prevents windows.h redefining min/max which breaks compilation */
#define NOMINMAX
#include <windows.h>
#endif

#include <zedwallet/Commands.h>
#include <zedwallet/CommandImplementations.h>
#include <zedwallet/Fusion.h>
#include <zedwallet/Open.h>
#include <zedwallet/ParseArguments.h>
#include <zedwallet/Sync.h>
#include <zedwallet/Transfer.h>
#include <zedwallet/Tools.h>
#include <zedwallet/WalletConfig.h>

int main(int argc, char **argv)
{
    /* On ctrl+c the program seems to throw "zedwallet.exe has stopped
       working" when calling exit(0)... I'm not sure why, this is a bit of
       a hack, it disables that - possibly some deconstructers calling
       terminate() */
    #ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    #endif

    Config config = parseArguments(argc, argv);

    /* User requested --help or --version, or invalid arguments */
    if (config.exit)
    {
        return 0;
    }

    Logging::LoggerManager logManager;

    /* We'd like these lines to be in the below if(), but because some genius
       thought it was a good idea to pass everything by reference and then
       use them after the functions lifetime they go out of scope and break
       stuff */
    logManager.setMaxLevel(Logging::DEBUGGING);

    Logging::FileLogger fileLogger;

    if (config.debug)
    {
        fileLogger.init(WalletConfig::walletName + ".log");
        logManager.addLogger(fileLogger);
    }

    Logging::LoggerRef logger(logManager, WalletConfig::walletName);

    /* Currency contains our coin parameters, such as decimal places, supply */
    const CryptoNote::Currency currency 
        = CryptoNote::CurrencyBuilder(logManager).currency();

    System::Dispatcher localDispatcher;
    System::Dispatcher *dispatcher = &localDispatcher;

    /* Our connection to turtlecoind */
    std::unique_ptr<CryptoNote::INode> node(
        new CryptoNote::NodeRpcProxy(config.host, config.port, 
                                     logger.getLogger()));

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) 
                    {errorPromise.set_value(e); };

    node->init(callback);

    std::future<void> initNode = std::async(std::launch::async, [&]
    {
            if (error.get())
            {
                throw std::runtime_error("Failed to initialize node!");
            }
    });

    std::future_status status = initNode.wait_for(std::chrono::seconds(20));

    /* Connection took to long to remote node, let program continue regardless
       as they could perform functions like export_keys without being
       connected */
    if (status != std::future_status::ready)
    {
        if (config.host != "127.0.0.1")
        {
            std::cout << WarningMsg("Unable to connect to remote node, "
                                    "connection timed out.")
                      << std::endl
                      << WarningMsg("Confirm the remote node is functioning, "
                                    "or try a different remote node.")
                      << std::endl << std::endl;
        }
        else
        {
            std::cout << WarningMsg("Unable to connect to node, "
                                    "connection timed out.")
                      << std::endl << std::endl;
        }
    }

    /* Create the wallet instance */
    CryptoNote::WalletGreen wallet(*dispatcher, currency, *node, 
                                   logger.getLogger());

    /* Run the interactive wallet interface */
    run(wallet, *node, config);
}

void run(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node,
         Config &config)
{
    auto maybeWalletInfo = Nothing<std::shared_ptr<WalletInfo>>();

    Action action;

    do
    {
        std::cout << InformationMsg(getVersion()) << std::endl;

        /* Open/import/generate the wallet */
        action = getAction(config);
        maybeWalletInfo = handleAction(wallet, action, config);

    /* Didn't manage to get the wallet info, returning to selection screen */
    } while (!maybeWalletInfo.isJust);

    auto walletInfo = maybeWalletInfo.x;

    bool alreadyShuttingDown = false;

    /* This will call shutdown when ctrl+c is hit. This is a lambda function,
       & means capture all variables by reference */
    Tools::SignalHandler::install([&]
    {
        /* If we're already shutting down let control flow continue as normal */
        if (shutdown(walletInfo->wallet, node, alreadyShuttingDown))
        {
            exit(0);
        }
    });

    while (node.getLastKnownBlockHeight() == 0)
    {
        std::stringstream msg;

        msg << "It looks like " << WalletConfig::daemonName << " isn't open!"
            << std::endl << std::endl
            << "Ensure " << WalletConfig::daemonName
            << " is open and has finished initializing." << std::endl
            << "If it's still not working, try restarting "
            << WalletConfig::daemonName << ". The daemon sometimes gets stuck."
            << std::endl << "Alternatively, perhaps "
            << WalletConfig::daemonName << " can't communicate with any peers."
            << std::endl << std::endl
            << "The wallet can't function until it can communicate with "
            << "the network." << std::endl;

        std::cout << WarningMsg(msg.str()) << std::endl;

        bool proceed = false;

        while (true)
        {
            std::cout << "[" << InformationMsg("T") << "]ry again, "
                      << "[" << InformationMsg("E") << "]xit, or "
                      << "[" << InformationMsg("C") << "]ontinue anyway?: ";

            std::string answer;
            std::getline(std::cin, answer);

            const char c = std::tolower(answer[0]);

            /* Lets people spam enter in the transaction screen */
            if (c == 't' || c == '\0')
            {
                break;
            }
            else if (c == 'e' || c == std::ifstream::traits_type::eof())
            {
                shutdown(walletInfo->wallet, node, alreadyShuttingDown);
                return;
            }
            else if (c == 'c')
            {
                proceed = true;
                break;
            }
            else
            {
                std::cout << WarningMsg("Bad input: ") << InformationMsg(answer)
                          << WarningMsg(" - please enter either T, E, or C.")
                          << std::endl;
            }
        }

        if (proceed)
        {
            break;
        }

        std::cout << std::endl;
    }

    /* Scan the chain for new transactions. In the case of an imported 
       wallet, we need to scan the whole chain to find any transactions. 
       If we opened the wallet however, we just need to scan from when we 
       last had it open. If we are generating a wallet, there is no need
       to check for transactions as there is no way the wallet can have
       received any money yet. */
    if (action != Generate)
    {
        syncWallet(node, walletInfo);
    }
    else
    {
        std::cout << InformationMsg("Your wallet is syncing with the "
                                    "network in the background.")
                  << std::endl
                  << InformationMsg("Until this is completed new "
                                    "transactions might not show up.")
                  << std::endl
                  << InformationMsg("Use bc_height to check the progress.")
                  << std::endl << std::endl;
    }

    welcomeMsg();

    inputLoop(walletInfo, node);

    shutdown(walletInfo->wallet, node, alreadyShuttingDown);
}

Maybe<std::shared_ptr<WalletInfo>> handleAction(CryptoNote::WalletGreen &wallet,
                                                Action action, Config &config)
{
    if (action == Generate)
    {
        return Just<std::shared_ptr<WalletInfo>>(generateWallet(wallet));
    }
    else if (action == Open)
    {
        return openWallet(wallet, config);
    }
    else if (action == Import)
    {
        return Just<std::shared_ptr<WalletInfo>>(importWallet(wallet));
    }
    else if (action == SeedImport)
    {
        return Just<std::shared_ptr<WalletInfo>>(mnemonicImportWallet(wallet));
    }
    else if (action == ViewWallet)
    {
        return Just<std::shared_ptr<WalletInfo>>(createViewWallet(wallet));
    }
    else
    {
        throw std::runtime_error("Unimplemented action!");
    }
}

Action getAction(Config &config)
{
    if (config.walletGiven || config.passGiven)
    {
        return Open;
    }

    while (true)
    {
        std::cout << std::endl << "Welcome, please choose an option below:"
                  << std::endl << std::endl
                  
                  << "\t[" << InformationMsg("G") << "] - "
                  << "Generate a new wallet address"
                  << std::endl 

                  << "\t[" << InformationMsg("O") << "] - "
                  << "Open a wallet already on your system"
                  << std::endl
                  
                  << "\t[" << InformationMsg("S") << "] - "
                  << "Regenerate your wallet using a seed phrase of words"
                  << std::endl
                  
                  << "\t[" << InformationMsg("I") << "] - "
                  << "Import your wallet using a View Key and Spend Key"
                  << std::endl

                  << "\t[" << InformationMsg("V") << "] - "
                  << "Import a view only wallet (Unable to send transactions)"
                  << std::endl << std::endl

                  << "or, press CTRL_C to exit: ";

        std::string answer;
        std::getline(std::cin, answer);

        const char c = std::tolower(answer[0]);

        if (c == 'o')
        {
            return Open;
        }
        else if (c == 'g')
        {
            return Generate;
        }
        else if (c == 'i')
        {
            return Import;
        }
        else if (c == 's')
        {
            return SeedImport;
        }
        else if (c == 'v')
        {
            return ViewWallet;
        }
        else
        {
            std::cout << "Unknown command: " << WarningMsg(answer) << std::endl;
        }
    }
}

void welcomeMsg()
{
    std::cout << "Use the "
              << SuggestionMsg("help") 
              << " command to see the list of available commands."
              << std::endl
              << "Use "
              << SuggestionMsg("exit")
              << " when closing to ensure your wallet file doesn't get "
              << "corrupted."
              << std::endl << std::endl;
}

#ifdef HAVE_READLINE

char **getAutoCompleteMatches(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    rl_completion_append_character = '\0';
    return rl_completion_matches(text, getAutoCompleteMatch);
}

char *getAutoCompleteMatch(const char *text, int state)
{
    /* Need to maintain position in vector across calls */
    static long unsigned int i = 0;

    /* Function is being called again, make sure we reset i to zero, otherwise
       no matches will be returned. */
    if (state == 0)
    {
        i = 0;
    }

    const auto commands = allCommands();

    while (i < commands.size())
    {
        std::string name = commands[i].name;

        i++;

        /* Check if text is a prefix of name */
        if (name.find(std::string(text)) == 0)
        {
            /* Can't just return .c_str(), the underlying std::string will
               be cleaned up and we'll have a dangling ptr - need to make a
               copy. Readline will call free() on it for us, so it's OK. */

            /* +1 for \0 */
            char *ptr = new char[name.length() + 1];

            strcpy(ptr, name.c_str());

            return ptr;
        }
    }

    return NULL;
}

#endif

std::string getCommand(std::shared_ptr<WalletInfo> &walletInfo)
{
    #ifdef HAVE_READLINE

    /* disable the signal handlers libreadline installed so ctrl+c and
       so on still work as expected */
    rl_catch_signals = 0;

    rl_attempted_completion_function = getAutoCompleteMatches;

    std::string prompt = yellowANSIMsg(getPrompt(walletInfo));

    char *command = readline(prompt.c_str());

    while (command != nullptr)
    {
        if (strlen(command) > 0)
        {
            add_history(command);
        }

        std::string tmp = std::string(command);

        free(command);

        return tmp;
    }

    return std::string(command);
    
    #else
    
    std::string command;
    std::getline(std::cin, command);
    boost::algorithm::trim(command);
    return command;

    #endif     
}

std::string getInputAndDoWorkWhileIdle(std::shared_ptr<WalletInfo> &walletInfo)
{
    auto lastUpdated = std::chrono::system_clock::now();

    std::future<std::string> inputGetter = std::async(std::launch::async, 
                                                      [&walletInfo]
    {
        return getCommand(walletInfo);
    });


    while (true)
    {
        /* Check if the user has inputted something yet (Wait for zero seconds
           to instantly return) */
        std::future_status status = inputGetter
                                   .wait_for(std::chrono::seconds(0));

        /* User has inputted, get what they inputted and return it */
        if (status == std::future_status::ready)
        {
            return inputGetter.get();
        }

        const auto currentTime = std::chrono::system_clock::now();

        /* Otherwise check if we need to update the wallet cache */
        if ((currentTime - lastUpdated) > std::chrono::seconds(5))
        {
            lastUpdated = currentTime;
            checkForNewTransactions(walletInfo);
        }

        /* Sleep for enough for it to not be noticeable when the user enters
           something, but enough that we're not starving the CPU */
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

bool shutdown(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node,
              bool &alreadyShuttingDown)
{
    if (alreadyShuttingDown)
    {
        std::cout << "Patience little turtle, we're already shutting down!" 
                  << std::endl;

        return false;
    }
    else
    {
        alreadyShuttingDown = true;

        std::cout << InformationMsg("Saving wallet and shutting down, please "
                                    "wait...") << std::endl;
    }

    bool finishedShutdown = false;

    std::thread timelyShutdown([&finishedShutdown]
    {
        const auto startTime = std::chrono::system_clock::now();

        /* Has shutdown finished? */
        while (!finishedShutdown)
        {
            const auto currentTime = std::chrono::system_clock::now();

            /* If not, wait for a max of 20 seconds then force exit. */
            if ((currentTime - startTime) > std::chrono::seconds(20))
            {
                std::cout << WarningMsg("Wallet took too long to save! "
                                        "Force closing.") << std::endl
                          << "Bye." << std::endl;
                exit(0);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    #ifdef HAVE_READLINE
    /* clean readline to restore usual terminal history after shutdown */
    rl_cleanup_after_signal();
    #endif  

    wallet.save();
    wallet.shutdown();
    node.shutdown();

    finishedShutdown = true;

    /* Wait for shutdown watcher to finish */
    timelyShutdown.join();

    std::cout << "Bye." << std::endl;
    
    return true;
}

void inputLoop(std::shared_ptr<WalletInfo> &walletInfo, CryptoNote::INode &node)
{
    while (true)
    {
        #ifndef HAVE_READLINE
        std::cout << InformationMsg(getPrompt(walletInfo));
        #endif

        const std::string command = getInputAndDoWorkWhileIdle(walletInfo);

        /* User used exit command */
        if (dispatchCommand(walletInfo, node, command))
        {
            return;
        }
    }
}
