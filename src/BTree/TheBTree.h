#pragma once

namespace tinydbpp {

template<typename KeyT, size_t BRankMin = 2, size_t BRankMax = 3>
class BTreePlus;
typedef BTreePlus<size_t, 2, 3> TheBTree;
}