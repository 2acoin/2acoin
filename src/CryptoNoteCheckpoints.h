// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
// Copyright (c) 2018, 2ACoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <cstddef>
#include <initializer_list>

namespace CryptoNote {
struct CheckpointData {
  uint32_t index;
  const char* blockId;
};

const std::initializer_list<CheckpointData> CHECKPOINTS = {  
{       0, "be58c8366a1f8a4b352f69064b588494c7f4dbd07e19821d7861945b2f85a2dd"},
{    5000, "f4925e32a226889b515e82f904ab1d0f1d2bfe762b25f9d0ad827bdc08a2b422"},
{   10000, "fd56d9647b3012dd30a0d12705b68f3c52af195f0a5cd66f8e8aa5112b41e05c"},
{   20000, "7fa0e1e9f2efe37b7f847ef77c8799381d3c73ae4703dc12b0590f071e4b7cc3"}
};
}
