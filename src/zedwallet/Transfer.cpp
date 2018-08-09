// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

///////////////////////////////
#include <zedwallet/Transfer.h>
///////////////////////////////

#include <boost/algorithm/string.hpp>

#include <Common/StringTools.h>

#include "CryptoNoteConfig.h"

#include <CryptoNoteCore/CryptoNoteBasicImpl.h>
#include <CryptoNoteCore/CryptoNoteTools.h>
#include <CryptoNoteCore/TransactionExtra.h>

#include "IWallet.h"

/* NodeErrors.h and WalletErrors.h have some conflicting enums, e.g. they
   both export NOT_INITIALIZED, we can get round this by using a namespace */
namespace NodeErrors
{
    #include <NodeRpcProxy/NodeErrors.h>
}

#include <zedwallet/ColouredMsg.h>
#include <zedwallet/Fusion.h>
#include <zedwallet/Tools.h>
#include <zedwallet/WalletConfig.h>

namespace WalletErrors
{
    #include <Wallet/WalletErrors.h>
}

#include <Wallet/WalletGreen.h>

bool parseAmount(std::string strAmount, uint64_t &amount)
{
    boost::algorithm::trim(strAmount);
    /* If the user entered thousand separators, remove them */
    boost::erase_all(strAmount, ",");

    const size_t pointIndex = strAmount.find_first_of('.');
    const size_t numDecimalPlaces = WalletConfig::numDecimalPlaces;

    size_t fractionSize;

    if (std::string::npos != pointIndex)
    {
        fractionSize = strAmount.size() - pointIndex - 1;

        while (numDecimalPlaces < fractionSize && '0' == strAmount.back())
        {
            strAmount.erase(strAmount.size() - 1, 1);
            fractionSize--;
        }

        if (numDecimalPlaces < fractionSize)
        {
            return false;
        }

        strAmount.erase(pointIndex, 1);
    }
    else
    {
        fractionSize = 0;
    }

    if (strAmount.empty())
    {
        return false;
    }

    if (!std::all_of(strAmount.begin(), strAmount.end(), ::isdigit))
    {
        return false;
    }

    if (fractionSize < numDecimalPlaces)
    {
        strAmount.append(numDecimalPlaces - fractionSize, '0');
    }

    bool success = Common::fromString(strAmount, amount);

    if (!success)
    {
        return false;
    }

    return amount >= WalletConfig::minimumSend;
}

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo,
                        bool integratedAddress, uint32_t nodeFee)
{
    std::cout << std::endl
              << InformationMsg("Confirm Transaction?") << std::endl;

    std::cout << "You are sending "
              << SuccessMsg(formatAmount(t.destinations[0].amount))
              << ", with a network fee of " << SuccessMsg(formatAmount(t.fee))
              << " and a node fee of " << SuccessMsg(formatAmount(nodeFee));

    const std::string paymentID = getPaymentIDFromExtra(t.extra);

    if (paymentID != "")
    {
        std::cout << ", " << std::endl
                  << "and a Payment ID of " << SuccessMsg(paymentID);
    }
    else
    {
        std::cout << ".";
    }
    
    std::cout << std::endl << std::endl
              << "FROM: " << SuccessMsg(walletInfo->walletFileName)
              << std::endl
              << "TO: " << SuccessMsg(t.destinations[0].address)
              << std::endl << std::endl;

    if (confirm("Is this correct?"))
    {
        confirmPassword(walletInfo->walletPass);
        return true;
    }

    return false;
}

void splitTX(CryptoNote::WalletGreen &wallet, 
             const CryptoNote::TransactionParameters p, uint32_t nodeFee)
{
    std::cout << "Transaction is still too large to send, splitting into "
              << "multiple chunks." 
              << std::endl
              << "This may take a long time."
              << std::endl
              << "It may also slightly raise the fee you have to pay,"
              << std::endl
              << "and hence reduce the total amount you can send if"
              << std::endl
              << "your balance cannot cover it." << std::endl;

    if (!confirm("Is this OK?"))
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    CryptoNote::TransactionParameters restoreInitialTx = p;

    const uint64_t maxSize = wallet.getMaxTxSize();
    const size_t txSize = wallet.getTxSize(p);
    const uint64_t defaultFee = WalletConfig::defaultFee;

    for (int numTxMultiplier = 1; ; numTxMultiplier++)
    {
        amountDivider = 2;
    }

    /* Use for quick restoring to originalDivider once a tx suceeds */
    uint64_t originalDivider = amountDivider;

    int txNumber = 1;

    while (true)
    {
        auto p1 = p;

        p1.destinations[0].amount = totalAmount / amountDivider;

        /* If we have odd numbers, we can have an amount that is smaller
           than the remainder to send, but the remainder is less than
           2 * amount.
           So, we include this amount in our current transaction to prevent
           this change not being sent.
           If we're trying to send more than the remaining amount, set to
           the remaining amount. */
        if ((p1.destinations[0].amount != remainder &&
             remainder < (p1.destinations[0].amount * 2))
         || (p1.destinations[0].amount > remainder))
        {
            p1.destinations[0].amount = remainder;
        }
        else if (p1.destinations[0].amount + p1.fee + nodeFee > balance)
        {
            p1.destinations[0].amount = balance - p1.fee - nodeFee;

            if (p1.destinations[0].amount < WalletConfig::minimumSend)
            {
                return;
            }
        }

        if (p1.destinations[0].amount == 0)
        {
            p.destinations[0].amount = wallet.getActualBalance() - totalFee;
        }

        uint64_t totalNeeded = p1.destinations[0].amount + p1.fee + nodeFee;

        std::vector<CryptoNote::TransactionParameters> transfers;

        for (int i = 0; i < numTransactions; i++)
        {
            CryptoNote::TransactionParameters tmp = p;
            tmp.destinations[0].amount = amountPerTx;
            tmp.fee = feePerTx;
            transfers.push_back(tmp);
        }

        /* Add the extra change to the first transaction */
        transfers[0].destinations[0].amount += change;

        for (const auto &tx : transfers)
        {
            amountDivider *= 2;
            /* This can take quite a long time so let them know it's not frozen
            */
            std::cout << InformationMsg("Working...") << std::endl;
            continue;
        }

        std::cout << InformationMsg("Sending transaction number ")
                  << InformationMsg(std::to_string(txNumber))
                  << InformationMsg("...")
                  << std::endl;

        const size_t id = wallet.transfer(prepared);
        auto hash = wallet.getTransaction(id).hash;

        std::cout << SuccessMsg("Transaction has been sent!")
                  << std::endl
                  << SuccessMsg("Hash: ")
                  << SuccessMsg(Common::podToHex(hash))
                  << std::endl
                  << SuccessMsg("Amount: ")
                  << SuccessMsg(formatAmount(p1.destinations[0].amount))
                  << std::endl << std::endl;

        txNumber++;

        sentAmount += p1.destinations[0].amount;
        /* Remember to remove the fee as well from balance */
        balance = balance - p1.destinations[0].amount - p1.fee - nodeFee;
        remainder = totalAmount - sentAmount;

        /* We've sent the full amount required now */
        if (sentAmount == totalAmount)
        {
            std::cout << InformationMsg("All transactions have been sent!")
                      << std::endl;

            return;
        }

        sendMultipleTransactions(wallet, transfers);
        return;
    }
}

void transfer(std::shared_ptr<WalletInfo> walletInfo, uint32_t height,
              bool sendAll, std::string nodeAddress, uint32_t nodeFee)
{
    std::cout << InformationMsg("Note: You can type cancel at any time to "
                                "cancel the transaction")
              << std::endl << std::endl;


    const uint64_t balance = walletInfo->wallet.getActualBalance();

    const auto maybeAddress = getDestinationAddress();

    if (!maybeAddress.isJust)
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    const std::string address = maybeAddress.x;

    const auto maybeAmount = getTransferAmount();

    if (!maybeAmount.isJust)
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    const uint64_t amount = maybeAmount.x;

    if (balance < amount)
    {
        const auto maybeAmount = getTransferAmount();

        if (!maybeAmount.isJust)
        {
            std::cout << WarningMsg("Cancelling transaction.") << std::endl;
            return;
        }

        amount = maybeAmount.x;

        switch (doWeHaveEnoughBalance(amount, WalletConfig::defaultFee,
                                      walletInfo, height, nodeFee))
        {
            case NotEnoughBalance:
            {
                std::cout << WarningMsg("Cancelling transaction.") << std::endl;
                return;
            }
            case SetMixinToZero:
            {
                mixin = 0;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    const auto maybeFee = getFee();

    if (!maybeFee.isJust)
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    const uint64_t fee = maybeFee.x;

    switch (doWeHaveEnoughBalance(amount, fee, walletInfo, height, nodeFee))
    {
        case NotEnoughBalance:
        {
            std::cout << WarningMsg("Cancelling transaction.") << std::endl;
            return;
        }
        case SetMixinToZero:
        {
            mixin = 0;
            break;
        }
        default:
        {
            break;
        }
    }

    /* This doesn't account for dust. We should probably make a function to
       check for balance minus dust */
    if (sendAll)
    {
        if (WalletConfig::defaultMixin != 0 && balance != balanceNoDust)
        {
            uint64_t unsendable = balance - balanceNoDust;

            amount = balanceNoDust - fee - nodeFee;

            std::cout << WarningMsg("Due to dust inputs, we are unable to ")
                      << WarningMsg("send ")
                      << InformationMsg(formatAmount(unsendable))
                      << WarningMsg("of your balance.") << std::endl;

            if (!WalletConfig::mixinZeroDisabled ||
                height < WalletConfig::mixinZeroDisabledHeight)
            {
                std::cout << "Alternatively, you can set the mixin count to "
                          << "zero to send it all." << std::endl;

                if (confirm("Set mixin to 0 so we can send your whole balance? "
                            "This will compromise privacy."))
                {
                    mixin = 0;
                    amount = balance - fee - nodeFee;
                }
            }
            else
            {
                std::cout << "Sorry." << std::endl;
            }
        }
        else
        {
            amount = balance - fee - nodeFee;
        }
    }

    /* Don't need to prompt for payment ID if they used an integrated
       address */
    if (!integratedAddress)
    {
        const auto maybeExtra = getExtra();

        if (!maybeExtra.isJust)
        {
            std::cout << WarningMsg("Cancelling transaction.") << std::endl;
            return;
        }

        extra = maybeExtra.x;
    }

    doTransfer(address, amount, fee, extra, walletInfo, height,
               integratedAddress, mixin, nodeAddress, nodeFee);
}

BalanceInfo doWeHaveEnoughBalance(uint64_t amount, uint64_t fee,
                                  std::shared_ptr<WalletInfo> walletInfo,
                                  uint64_t height, uint32_t nodeFee)
{
    const uint64_t balance = walletInfo->wallet.getActualBalance();

    const uint64_t balanceNoDust = walletInfo->wallet.getBalanceMinusDust
    (
        {walletInfo->walletAddress}
    );

    /* They have to include at least a fee of this large */
    if (balance < amount + fee + nodeFee)
    {
        std::cout << std::endl
                  << WarningMsg("You don't have enough funds to cover ")
                  << WarningMsg("this transaction!") << std::endl << std::endl
                  << "Funds needed: "
                  << InformationMsg(formatAmount(amount+fee+nodeFee))
                  << " (Includes a network fee of "
                  << InformationMsg(formatAmount(fee))
                  << " and a node fee of "
                  << InformationMsg(formatAmount(nodeFee))
                  << ")"
                  << std::endl
                  << "Funds available: "
                  << SuccessMsg(formatAmount(balance))
                  << std::endl << std::endl;

        return NotEnoughBalance;
    }
    else if (WalletConfig::defaultMixin != 0 &&
             balanceNoDust < amount + WalletConfig::minimumFee + nodeFee)
    {
        std::cout << std::endl
                  << WarningMsg("This transaction is unable to be sent ")
                  << WarningMsg("due to dust inputs.") << std::endl
                  << "You can send "
                  << InformationMsg(formatAmount(balanceNoDust))
                  << " without issues (includes a network fee of "
                  << InformationMsg(formatAmount(fee)) << " and "
                  << " a node fee of "
                  << InformationMsg(formatAmount(nodeFee))
                  << ")"
                  << std::endl;
        return;
    }

    const auto maybeExtra = getExtra();

    if (!maybeExtra.isJust)
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    const std::string extra = maybeExtra.x;

    doTransfer(address, amount, fee, extra, walletInfo, height);
}

void doTransfer(std::string address, uint64_t amount, uint64_t fee,
                std::string extra, std::shared_ptr<WalletInfo> walletInfo,
                uint32_t height, bool integratedAddress, uint64_t mixin,
                std::string nodeAddress, uint32_t nodeFee)
{
    const uint64_t balance = walletInfo->wallet.getActualBalance();

    if (balance < amount + fee + nodeFee)
    {
        std::cout << WarningMsg("You don't have enough funds to cover this ")
                  << WarningMsg("transaction!")
                  << std::endl
                  << InformationMsg("Funds needed: ")
                  << InformationMsg(formatAmount(amount + fee + nodeFee))
                  << std::endl
                  << SuccessMsg("Funds available: " + formatAmount(balance))
                  << std::endl;
        return;
    }

    CryptoNote::TransactionParameters p;

    p.destinations = std::vector<CryptoNote::WalletOrder>
    {
        {address, amount}
    };
    
    if (!nodeAddress.empty() && nodeFee != 0) {
      p.destinations.push_back({nodeAddress, nodeFee});
    }

    p.fee = fee;
    p.mixIn = WalletConfig::defaultMixin;
    p.extra = extra;
    p.changeDestination = walletInfo->walletAddress;

    if (!confirmTransaction(p, walletInfo, integratedAddress, nodeFee))
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    sendTX(walletInfo, p, height, false, nodeFee);
}

void sendTX(std::shared_ptr<WalletInfo> walletInfo, 
            CryptoNote::TransactionParameters p, uint32_t height,
            bool retried, uint32_t nodeFee)
{
    try
    {
        try
        {
            if (walletInfo->wallet.txIsTooLarge(p))
            {
                return;
            }

            /* Reform with the optimized inputs */
            tx = walletInfo->wallet.formTransaction(p);

            /* If the transaction is still too large, lets split it up into 
               smaller chunks */
            if (walletInfo->wallet.txIsTooLarge(tx))
            {
                splitTX(walletInfo->wallet, p, nodeFee);
                return;
            }
        }

        const size_t id = walletInfo->wallet.transfer(tx);
        auto hash = walletInfo->wallet.getTransaction(id).hash;
        
        std::cout << SuccessMsg("Transaction has been sent!")
                  << std::endl
                  << SuccessMsg("Hash: ")
                  << SuccessMsg(Common::podToHex(hash))
                  << std::endl;
    }
    /* Lets handle the error and possibly resend the transaction */
    catch (const std::system_error &e)
    {
        bool setMixinToZero = handleTransferError(e, retried, height);

        if (setMixinToZero)
        {
            p.mixIn = 0;
            sendTX(walletInfo, p, height, true, nodeFee);
        }
    }
}

bool handleTransferError(const std::system_error &e,
                         bool retried,
                         uint32_t height)
{
    if (retried)
    {
        std::cout << WarningMsg("Failed to send transaction!")
                  << std::endl << "Error message: " << e.what()
                  << std::endl;

                if (walletInfo->wallet.txIsTooLarge(p))
                {
                    splitTx(walletInfo->wallet, p);
                }
                else
                {
                    
                    const size_t id = walletInfo->wallet.transfer(p);

                    const CryptoNote::WalletTransaction tx
                        = walletInfo->wallet.getTransaction(id);

                    std::cout << SuccessMsg("Transaction has been sent!")
                              << std::endl
                              << SuccessMsg("Hash:" 
                                          + Common::podToHex(tx.hash))
                              << std::endl;
                }
            }
            else
            {
                const size_t id = walletInfo->wallet.transfer(p);
                
                const CryptoNote::WalletTransaction tx 
                    = walletInfo->wallet.getTransaction(id);

                std::cout << SuccessMsg("Transaction has been sent!")
                          << std::endl
                          << SuccessMsg("Hash: " + 
                                        Common::podToHex(tx.hash))
                          << std::endl;
            }
        }
        catch (const std::system_error &e)
        {
            if (retried)
            {
                std::cout << WarningMsg("Failed to send transaction!")
                          << std::endl << "Error message: " << e.what()
                          << std::endl;
                return;
            }

            bool wrongAmount = false;

            switch (e.code().value())
            {
                case WalletErrors::CryptoNote::error::WRONG_AMOUNT:
                {
                    wrongAmount = true;
                    [[fallthrough]];
                }
                case WalletErrors::CryptoNote::error::MIXIN_COUNT_TOO_BIG:
                case NodeErrors::CryptoNote::error::INTERNAL_NODE_ERROR:
                {
            
                    if (wrongAmount)
                    {
                        std::cout << WarningMsg("Failed to send transaction "
                                                "- not enough funds!")
                                  << std::endl
                                  << "Unable to send dust inputs."
                                  << std::endl;
                    }
                    else
                    {
                        std::cout << WarningMsg("Failed to send transaction!")
                                  << std::endl
                                  << "Unable to find enough outputs to "
                                  << "mix with."
                                  << std::endl;
                    }

                    std::cout << "Try lowering the amount you are sending "
                              << "in one transaction." << std::endl;

                    /* If a mixin of zero is allowed, or we are below the
                       fork height when it's banned, ask them to resend with
                       zero */
                    if (!WalletConfig::mixinZeroDisabled ||
                         height < WalletConfig::mixinZeroDisabledHeight)
                    {
                        std::cout << "Alternatively, you can sent the mixin "
                                  << "count to 0." << std::endl;

                        if(confirm("Retry transaction with mixin of 0? "
                                   "This will compromise privacy."))
                        {
                            p.mixIn = 0;
                            retried = true;
                            continue;
                        }
                    }

                    std::cout << WarningMsg("Cancelling transaction.")
                              << std::endl;

                    break;
                }
                case NodeErrors::CryptoNote::error::NETWORK_ERROR:
                case NodeErrors::CryptoNote::error::CONNECT_ERROR:
                {
                    std::cout << WarningMsg("Couldn't connect to the network "
                                            "to send the transaction!")
                              << std::endl
                              << "Ensure " << WalletConfig::daemonName
                              << " or the remote node you are using is open "
                              << "and functioning."
                              << std::endl;
                    break;
                }
                default:
                {
                    /* Some errors don't have an associated value, just an
                       error string */
                    std::string msg = e.what();

                    if (msg == "Failed add key input: key image already spent")
                    {
                        std::cout << WarningMsg("Failed to send transaction - "
                                                "wallet is not synced yet!")
                                  << std::endl
                                  << "Use the " << InformationMsg("bc_height")
                                  << " command to view the wallet sync status."
                                  << std::endl;
                        return;
                    }

                    std::cout << WarningMsg("Failed to send transaction!")
                              << std::endl << "Error message: " << msg
                              << std::endl
                              << "Please report what you were doing to cause "
                              << "this error so we can fix it! :)"
                              << std::endl;
                    break;
                }
            }
        }

        break;
    }
}

Maybe<std::string> getPaymentID(std::string msg)
{
    while (true)
    {
        std::string paymentID;

        std::cout << msg
                  << WarningMsg("Warning: If you were given a payment ID,")
                  << std::endl
                  << WarningMsg("you MUST use it, or your funds may be lost!")
                  << std::endl
                  << "Hit enter for the default of no payment ID: ";

        std::getline(std::cin, paymentID);

        if (paymentID == "")
        {
            return Just<std::string>(paymentID);
        }

        if (paymentID == "cancel")
        {
            return Nothing<std::string>();
        }

        std::vector<uint8_t> extra;

        /* Convert the payment ID into an "extra" */
        if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
        {
            std::cout << WarningMsg("Failed to parse! Payment ID's are 64 "
                                    "character hexadecimal strings.")
                      << std::endl;
            continue;
        }

        return Just<std::string>(paymentID);
    }
}

std::string getExtraFromPaymentID(std::string paymentID)
{
    if (paymentID == "")
    {
        return paymentID;
    }

    std::vector<uint8_t> extra;

    /* Convert the payment ID into an "extra" */
    CryptoNote::createTxExtraWithPaymentId(paymentID, extra);

    /* Then convert the "extra" back into a string so we can pass
       the argument that walletgreen expects. Note this string is not
       the same as the original paymentID string! */
    std::string extraString;

    for (auto i : extra)
    {
        extraString += static_cast<char>(i);
    }

    return extraString;
}

Maybe<std::string> getExtra()
{
    std::stringstream msg;

    msg << std::endl
        << InformationMsg("What payment ID do you want to use?")
        << std::endl
        << "These are usually used for sending to exchanges."
        << std::endl;

    auto maybePaymentID = getPaymentID(msg.str());

    if (!maybePaymentID.isJust)
    {
        return maybePaymentID;
    }

    if (maybePaymentID.x == "")
    {
        return maybePaymentID;
    }

    return Just<std::string>(getExtraFromPaymentID(maybePaymentID.x));
}

Maybe<uint64_t> getFee()
{
    while (true)
    {
        std::string stringAmount;
        std::cout << std::endl 
                  << InformationMsg("What fee do you want to use?")
                  << std::endl
                  << "Hit enter for the default fee of "
                  << formatAmount(WalletConfig::defaultFee)
                  << ": ";

        std::getline(std::cin, stringAmount);

        if (stringAmount == "")
        {
            return Just<uint64_t>(WalletConfig::defaultFee);
        }

        if (stringAmount == "cancel")
        {
            return Nothing<uint64_t>();
        }

        uint64_t amount;

        if (parseFee(stringAmount))
        {
            parseAmount(stringAmount, amount);
            return Just<uint64_t>(amount);
        }
    }
}

Maybe<uint64_t> getTransferAmount()
{
    while (true)
    {
        std::string stringAmount;

        std::cout << std::endl
                  << InformationMsg("How much ")
                  << InformationMsg(WalletConfig::ticker)
                  << InformationMsg(" do you want to send?: ");

        std::getline(std::cin, stringAmount);

        if (stringAmount == "cancel")
        {
            return Nothing<uint64_t>();
        }

        uint64_t amount;

        if (parseAmount(stringAmount))
        {
            parseAmount(stringAmount, amount);
            return Just<uint64_t>(amount);
        }
    }
}

Maybe<std::string> getDestinationAddress()
{
    while (true)
    {
        std::string transferAddr;

        std::cout << InformationMsg("What address do you want to ")
                  << InformationMsg("transfer to?: ");

        std::getline(std::cin, transferAddr);
        boost::algorithm::trim(transferAddr);

        if (transferAddr == "cancel")
        {
            return Nothing<std::string>();
        }

        if (parseAddress(transferAddr))
        {
            return Just<std::string>(transferAddr);
        }
    }
}

bool parseFee(std::string feeString)
{
    uint64_t fee;

    if (!parseAmount(feeString, fee))
    {
        std::cout << WarningMsg("Failed to parse fee! Ensure you entered the "
                                "value correctly.")
                  << std::endl
                  << "Please note, you can only use "
                  << WalletConfig::numDecimalPlaces << " decimal places."
                  << std::endl;

        return false;
    }
    else if (fee < WalletConfig::minimumFee)
    {
        std::cout << WarningMsg("Fee must be at least ")
                  << formatAmount(WalletConfig::minimumFee) << "!"
                  << std::endl;

        return false;
    }

    return true;
}

Maybe<std::pair<std::string, std::string>> extractIntegratedAddress(
    std::string integratedAddress)
{
    if (integratedAddress.length() != WalletConfig::integratedAddressLength)
    {
        return Nothing<std::pair<std::string, std::string>>();
    }

    std::string decoded;
    uint64_t prefix;

    /* Need to be able to decode the string as an address */
    if (!Tools::Base58::decode_addr(integratedAddress, prefix, decoded))
    {
        return Nothing<std::pair<std::string, std::string>>();
    }

    /* The prefix needs to be the same as the base58 prefix */
    if (prefix !=
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX)
    {
        return Nothing<std::pair<std::string, std::string>>();
    }

    const uint64_t paymentIDLen = 64;

    /* Grab the payment ID from the decoded address */
    std::string paymentID = decoded.substr(0, paymentIDLen);

    /* The binary array encoded keys are the rest of the address */
    std::string keys = decoded.substr(paymentIDLen, std::string::npos);

    CryptoNote::AccountPublicAddress addr;
    CryptoNote::BinaryArray ba = Common::asBinaryArray(keys);

    if (!CryptoNote::fromBinaryArray(addr, ba))
    {
        return Nothing<std::pair<std::string, std::string>>();
    }

    /* Parse the AccountPublicAddress into a standard wallet address */
    /* Use the calculated prefix from earlier for less typing :p */
    std::string address = CryptoNote::getAccountAddressAsStr(prefix, addr);

    /* The address out should of course be a valid address */
    if (!parseStandardAddress(address))
    {
        return Nothing<std::pair<std::string, std::string>>();
    }
    
    std::vector<uint8_t> extra;

    /* And the payment ID out should be valid as well! */
    if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
    {
        return Nothing<std::pair<std::string, std::string>>();
    }
    
    return Just<std::pair<std::string, std::string>>({address, paymentID});
}

Maybe<std::pair<AddressType, std::string>> getAddress(std::string msg)
{
    while (true)
    {
        std::string address;

        std::cout << InformationMsg(msg);

        std::getline(std::cin, address);
        boost::algorithm::trim(address);

        if (address == "cancel")
        {
            return Nothing<std::pair<AddressType, std::string>>();
        }

        auto addressType = parseAddress(address);

        if (addressType != NotAnAddress)
        {
            return Just<std::pair<AddressType, std::string>>
            ({
                addressType, address
            });
        }
    }
}

AddressType parseAddress(std::string address)
{
    if (parseStandardAddress(address))
    {
        return StandardAddress;
    }

    if (parseIntegratedAddress(address))
    {
        return IntegratedAddress;
    }

    /* Failed to parse, lets try and diagnose a more accurate failure message */

    if (address.length() != WalletConfig::standardAddressLength &&
        address.length() != WalletConfig::integratedAddressLength)
    {
        std::cout << WarningMsg("Address is wrong length!") << std::endl
                  << "It should be " << WalletConfig::standardAddressLength
                  << " or " << WalletConfig::integratedAddressLength
                  << " characters long, but it is " << address.length()
                  << " characters long!" << std::endl << std::endl;

        return NotAnAddress;
    }

    if (address.substr(0, WalletConfig::addressPrefix.length()) !=
        WalletConfig::addressPrefix)
    {
        std::cout << WarningMsg("Invalid address! It should start with ")
                  << WarningMsg(WalletConfig::addressPrefix)
                  << WarningMsg("!")
                  << std::endl << std::endl;

        return NotAnAddress;
    }

    std::cout << WarningMsg("Failed to parse address, address is not a ")
              << WarningMsg("valid ")
              << WarningMsg(WalletConfig::ticker)
              << WarningMsg(" address!") << std::endl
              << std::endl;

    return NotAnAddress;
}

bool parseIntegratedAddress(std::string integratedAddress)
{
    return extractIntegratedAddress(integratedAddress).isJust;
}

bool parseStandardAddress(std::string address, bool printErrors)
{
    uint64_t prefix;

    CryptoNote::AccountPublicAddress addr;

    const bool valid = CryptoNote::parseAccountAddressString(prefix, addr,
                                                             address);

    if (address.length() != WalletConfig::addressLength)
    {
        std::cout << WarningMsg("Address is wrong length!") << std::endl
                  << "It should be " << WalletConfig::addressLength
                  << " characters long, but it is " << address.length()
                  << " characters long!" << std::endl << std::endl;

        return false;
    }
    /* We can't get the actual prefix if the address is invalid for other
       reasons. To work around this, we can just check that the address starts
       with TRTL, as long as the prefix is the TRTL prefix. This keeps it
       working on testnets with different prefixes. */
    else if (address.substr(0, WalletConfig::addressPrefix.length()) 
          != WalletConfig::addressPrefix)
    {
        std::cout << WarningMsg("Invalid address! It should start with ")
                  << WarningMsg(WalletConfig::addressPrefix)
                  << WarningMsg("!")
                  << std::endl << std::endl;

        return false;
    }
    /* We can return earlier by checking the value of valid, but then we don't
       get to give more detailed error messages about the address */
    else if (!valid)
    {
        std::cout << WarningMsg("Failed to parse address, address is not a ")
                  << WarningMsg("valid ")
                  << WarningMsg(WalletConfig::ticker)
                  << WarningMsg(" address!") << std::endl
                  << std::endl;

        return false;
    }

    return true;
}

bool parseAmount(std::string amountString)
{
    uint64_t amount;

    if (!parseAmount(amountString, amount))
    {
        std::cout << WarningMsg("Failed to parse amount! Ensure you entered "
                                "the value correctly.")
                  << std::endl
                  << "Please note, the minimum you can send is "
                  << formatAmount(WalletConfig::minimumSend) << ","
                  << std::endl
                  << "and you can only use " << WalletConfig::numDecimalPlaces
                  << " decimal places."
                  << std::endl;

        return false;
    }

    return true;
}
