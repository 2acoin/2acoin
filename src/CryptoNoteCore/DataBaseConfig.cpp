// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
// Copyright (c) 2019, 2ACoin Developers
//
// Please see the included LICENSE file for more information.

#include "DataBaseConfig.h"

#include <Common/Util.h>
#include "Common/StringTools.h"
#include "crypto/crypto.h"
#include <config/CryptoNoteConfig.h>

using namespace CryptoNote;

namespace {
  const uint64_t MEGABYTE = 1024 * 1024;
}

DataBaseConfig::DataBaseConfig() :
  dataDir(Tools::getDefaultDataDirectory()),
  backgroundThreadsCount(DATABASE_DEFAULT_BACKGROUND_THREADS_COUNT),
  maxOpenFiles(DATABASE_DEFAULT_MAX_OPEN_FILES),
  writeBufferSize(DATABASE_WRITE_BUFFER_MB_DEFAULT_SIZE * MEGABYTE),
  readCacheSize(DATABASE_READ_BUFFER_MB_DEFAULT_SIZE * MEGABYTE),
  testnet(false) {
}

bool DataBaseConfig::init(const std::string dataDirectory, const int backgroundThreads, const int openFiles, const int writeBufferMB, const int readCacheMB)
{
  dataDir = dataDirectory;
  backgroundThreadsCount = backgroundThreads;
  maxOpenFiles = openFiles;
  writeBufferSize = writeBufferMB * MEGABYTE;
  readCacheSize = readCacheMB * MEGABYTE;

  if (dataDir == Tools::getDefaultDataDirectory())
  {
    configFolderDefaulted = true;
  }

  return true;
}

bool DataBaseConfig::isConfigFolderDefaulted() const {
  return configFolderDefaulted;
}

std::string DataBaseConfig::getDataDir() const {
  return dataDir;
}

uint16_t DataBaseConfig::getBackgroundThreadsCount() const {
  return backgroundThreadsCount;
}

uint32_t DataBaseConfig::getMaxOpenFiles() const {
  return maxOpenFiles;
}

uint64_t DataBaseConfig::getWriteBufferSize() const {
  return writeBufferSize;
}

uint64_t DataBaseConfig::getReadCacheSize() const {
  return readCacheSize;
}

bool DataBaseConfig::getTestnet() const {
  return testnet;
}

void DataBaseConfig::setConfigFolderDefaulted(bool defaulted) {
  configFolderDefaulted = defaulted;
}

void DataBaseConfig::setDataDir(const std::string& dataDir) {
  this->dataDir = dataDir;
}

void DataBaseConfig::setBackgroundThreadsCount(uint16_t backgroundThreadsCount) {
  this->backgroundThreadsCount = backgroundThreadsCount;
}

void DataBaseConfig::setMaxOpenFiles(uint32_t maxOpenFiles) {
  this->maxOpenFiles = maxOpenFiles;
}

void DataBaseConfig::setWriteBufferSize(uint64_t writeBufferSize) {
  this->writeBufferSize = writeBufferSize;
}

void DataBaseConfig::setReadCacheSize(uint64_t readCacheSize) {
  this->readCacheSize = readCacheSize;
}

void DataBaseConfig::setTestnet(bool testnet) {
  this->testnet = testnet;
}
