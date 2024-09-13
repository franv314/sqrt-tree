/*
 * Task: https://www.codechef.com/problems/SEGPROD
 * Tests: Query/Merge with lambda
 */

#include <vector>
#include <functional>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <type_traits>
using namespace std;

template<typename T, typename F = plus<T>, bool Indexed = true>
class SqrtTree {

    template<bool Top = false>
    struct Item {
        int block_size;
        vector<T> suffix, prefix;
        conditional_t<Top, SqrtTree<T, F, false>, vector<vector<T>>> between;

        T query(int l, int r, const F& f) const {
            int fst_block = l / block_size;
            int lst_block = r / block_size;

            if (lst_block > fst_block + 1) {
                if constexpr (Top)
                    return f(f(suffix[l], between.query(fst_block + 1, lst_block)), prefix[r]);
                else
                    return f(f(suffix[l], between[fst_block + 1][lst_block - 1]), prefix[r]);
            }
            return f(suffix[l], prefix[r]);
        }

        decltype(between) build(const F& f) {
            for (int i = 0; i < prefix.size(); i += block_size) {
                for (int j =  1; j < block_size; j++)
                    prefix[i + j] = f(prefix[i + j - 1], prefix[i + j]);
                for (int j = block_size - 2; j >= 0; j--)
                    suffix[i + j] = f(suffix[i + j + 1], suffix[i + j]);
            }

            int blocks = prefix.size() / block_size;
            vector<T> block_vals(block_size);
            for (int i = 0; i < blocks; i++)
                block_vals[i] = suffix[i * block_size];

            if constexpr (Top) {
                return SqrtTree<T, F, false>(block_vals, f);
            } else {
                vector<vector<T>> bet(blocks, vector<T>(blocks));

                for (int i = 0; i < blocks; i++) {
                    bet[i][i] = block_vals[i];
                    for (int j = i + 1; j < blocks; j++)
                        bet[i][j] = f(bet[i][j - 1], block_vals[j]);
                }

                return bet;
            }
        }

        Item(typename vector<T>::iterator start, typename vector<T>::iterator end, int block_size, const F& f)
            : block_size(block_size), suffix(start, end), prefix(start, end), between(std::move(build(f))) { }
    };

    static constexpr int get_layer[] = {
        -1, -1,
        0,
        1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    };

    static constexpr int get_layer_shift[] = { 2, 4, 8, 16 };
    static constexpr unsigned int get_layer_mask[] = { 0x3, 0xf, 0xff, 0xffff, 0xffffffff };

    Item<Indexed> build() {
        int s = 1 << (int)ceil(log2(arr.size()));

        vector<T> tmp = arr;
        tmp.resize(s);

        int old = 1;
        for (int curr = 2; (1LL << curr) < s; curr *= 2, old *= 2) {
            int curr_size = 1 << curr;
            int block_size = 1 << old;

            vector<Item<>> layer;
            layer.reserve(s >> curr);

            for (int i = 0; i < s; i += curr_size)
                layer.emplace_back(tmp.begin() + i, tmp.begin() + i + curr_size, block_size, f);

            layers.push_back(std::move(layer));
        }

        return Item<Indexed>(tmp.begin(), tmp.end(), 1 << old, f);
    }

    F f;
    vector<T> arr;
    vector<vector<Item<>>> layers;
    Item<Indexed> top_layer;

public:

    T query(int l, int r) const {
        switch (r - l) {
        case 0: return T();
        case 1: return arr[l];
        case 2: return f(arr[l], arr[l + 1]);
        default:
            r--;
            int layer = get_layer[64 - __builtin_clzll(l ^ r)];

            if (layer == layers.size())
                return top_layer.query(l, r, f);

            int block = l >> get_layer_shift[layer];
            l &= get_layer_mask[layer];
            r &= get_layer_mask[layer];

            return layers[layer][block].query(l, r, f);
        }
    }

    SqrtTree(const vector<T> &arr, const F &f) : arr(arr), f(f), top_layer(std::move(build())) { }
    SqrtTree(const vector<T> &arr) : SqrtTree(arr, F()) { }
};


void solve() {
    int n, p, q; cin >> n >> p >> q;
    auto merge = [&](long long u, long long v) { return u * v % p; };

    vector<long long> arr(n);
    for (auto &x: arr) cin >> x;

    SqrtTree<long long, decltype(merge)> tree(arr, merge);

    int x = 0, l, r;
    int old; cin >> old;

    for (int i = 0; i < q; i++) {
        if (i % 64 == 0) {
             l = old;
             cin >> r;
             old = r;
        }

        tie(l, r) = minmax((l + x) % n, (r + x) % n);
        x = (tree.query(l, r + 1) + 1) % p;
    }

    cout << x << '\n';
}

int main() {
    ios_base::sync_with_stdio(false); cin.tie(nullptr);

    int t; cin >> t;
    while (t--) solve();
}