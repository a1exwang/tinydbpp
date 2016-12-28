#pragma once

namespace tinydbpp {

template<typename KeyT, size_t BRankMin, size_t BRankMax>
class BTreePlus;
typedef BTreePlus<size_t, 3, 5> TheBTree;
}