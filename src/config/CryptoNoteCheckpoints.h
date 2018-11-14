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
{   15000, "09c6ff3a2130cd175a9cc2cd00485d484ec0ed647d2c79575f01e6b42335df4b"},
{   20000, "7fa0e1e9f2efe37b7f847ef77c8799381d3c73ae4703dc12b0590f071e4b7cc3"},
{   25000, "1658532624c18f326092c8014ad69c1d565045bd51d7f7d0ff0abc11bf036049"},
{   30000, "3ec3ce36f0134655c80b1c9e9977d03c85cf5675ea09f08e54a2245f2d6c4406"},
{   35000, "81610d559594a2a8d9b87b0ef051a06a6e40a0cd2d88fc312e1cd1733de0b3d2"},
{   40000, "b82f45521363c28c8dc5d1a0f89b757fde2abb67b02b98c24c65f4b14b839264"},
{   45000, "dfe435831de24aca7fd1b44fabbe424f04f1b6412b39204ef71245c700a938a0"},
{   50000, "0f0d57dd7cde882bd7beee55aeef3eb86d79005f474636fbacf8529cfc7c4dfb"},
{   55000, "7a04ff3aa5b75acb9357fc2622156ab8520859b96520e43769be75968c6b0e50"},
{   60000, "0cfd1499436a6b949cf7a1e5c2d2c653db932bb0348af63e35bead4f22f3de6e"},
{   65000, "a0e5f7160d7a5741da62669deea099c240993021afeb427fdd39fe0ee6528837"},
{   70000, "d8476e2e5ab742e1b0d9a6a30d9fcc28220a055489dcab9fd3b8e57f74e40988"},
{   75000, "f9e116fd0948a874406eb77f39e58957ae39d71a5dccf0962f6444264dcc9d33"},
{   80000, "3ca4969286e1aa8be7b9c6ad7c2e518e011bdc17a6ebacd78d1e8bb39bbf7579"},
{   85000, "8cec855eda8f09ca4cd17c51d1229353d64e37ed8dff13649ceb1946f29a9710"},
{   90000, "79b75442f1aa3341fb26ef341a60b359a09d878980185a666141796290c0f07f"},
{   95000, "5cc0d391a5c713cba3e0c59b866e1f21c9103842007d783fafae3b362fae283b"}
};
}
