#include <stdint.h>

#include <optional>
#include <unordered_map>

struct TTEntry {
    uint_fast8_t depth;
    uint_fast16_t score;
    uint_fast8_t type;

    TTEntry(uint_fast8_t d, uint_fast16_t s, uint_fast8_t t) {
        depth = d;
        score = s;
        type = t;
    }
};

namespace TranspositionTable {
class __attribute__((unused)) TT {
    std::unordered_map<unsigned long long, TTEntry> hashtable;

   public:
    void record_hash(const State& target, int depth, int a, int hashDataType) {
        unsigned long long key = target.generateKey();
        if (hashtable.contains(key)) {
            TTEntry entry = hashtable[key];
            if (entry.depth >= depth) {
                hashtable[key] = TTEntry(depth, a, hashDataType);
            }
        } else {
            hashtable[key] = TTEntry(depth, a, hashDataType);
        }
    }

    auto probe_hash(const State& target) -> std::optional<TTEntry> {
        unsigned long long key = target.generateKey();
        if (hashtable.contains(key)) {
            return hashtable[key];
        } else {
            return std::nullopt;
        }
    }

    auto probe_hash_value(const State& target, int depth, int a, int b, bool &validityFlag) -> int {
        unsigned long long key = target.generateKey();
        TTEntry entry = hashtable[key];
        if (entry.depth >= depth) {
            if (entry.type == 0) {
                return entry.score;
            }
            if (entry.type == 1 && entry.score <= a) {
                return a;
            }
            if (entry.type == 2 && entry.score >= b) {
                return b;
            }
        }
        validityFlag = false;
        return 0;
    }
};
}  // namespace TranspositionTable
