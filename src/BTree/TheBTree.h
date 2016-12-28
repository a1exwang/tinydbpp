#pragma once

namespace tinydbpp {

template<typename KeyT, size_t BRankMin, size_t BRankMax>
class BTreePlus;
typedef BTreePlus<size_t, 26, 50> TheBTree;
}