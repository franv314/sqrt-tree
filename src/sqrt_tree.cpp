#include <vector>
#include <functional>
#include <cmath>
using namespace std;

template<typename T, typename F = plus<T>>
class SqrtTree {
    struct Item {
        int block_size;
        vector<T> suffix, prefix;
        vector<vector<T>> between;

        T query(int l, int r, const F& f) const {
            int fst_block = l / block_size;
            int lst_block = r / block_size;

            if (lst_block > fst_block + 1)
                return f(f(suffix[l], between[fst_block + 1][lst_block - 1]), prefix[r]);
            return f(suffix[l], prefix[r]);
        }


        Item() { }
        Item(typename vector<T>::iterator start, typename vector<T>::iterator end, int block_size, const F& f)
            : block_size(block_size), suffix(start, end), prefix(start, end)
        {
            for (int i = 0; i < end - start; i += block_size) {
                for (int j =  1; j < block_size; j++)
                    prefix[i + j] = f(prefix[i + j - 1], prefix[i + j]);
                for (int j = block_size - 2; j >= 0; j--)
                    suffix[i + j] = f(suffix[i + j + 1], suffix[i + j]);
            }

            int blocks = (end - start) / block_size;
            between.assign(blocks, vector<T>(blocks));

            for (int i = 0; i < blocks; i++) {
                between[i][i] = suffix[i * block_size];
                for (int j = i + 1; j < blocks; j++)
                    between[i][j] = f(between[i][j - 1], suffix[j * block_size]);
            }
        }
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

    F f;
    vector<T> arr;
    vector<vector<Item>> layers;

public:

    T query(int l, int r) {
        switch (r - l) {
        case 0: return T();
        case 1: return arr[l];
        case 2: return f(arr[l], arr[l + 1]);
        default:
            r--;
            int layer = get_layer[64 - __builtin_clzll(l ^ r)];

            if (layer == layers.size() - 1)
                return layers[layer][0].query(l, r, f);

            int block = l >> get_layer_shift[layer];
            l &= get_layer_mask[layer];
            r &= get_layer_mask[layer];

            return layers[layer][block].query(l, r, f);
        }
    }

    SqrtTree(const vector<T> &arr, const F &f) : arr(arr), f(f) {
        int s = 1 << (int)ceil(log2(arr.size()));

        vector<T> tmp = arr;
        tmp.resize(s);

        int old = 1;
        for (int curr = 2; (1LL << curr) < s; curr *= 2, old *= 2) {
            int curr_size = 1 << curr;
            int block_size = 1 << old;

            vector<Item> layer(s >> curr);
            for (int i = 0; i < s; i += curr_size)
                layer[i >> curr] = Item(tmp.begin() + i, tmp.begin() + i + curr_size, block_size, f);

            layers.push_back(std::move(layer));
        }

        layers.push_back({Item(tmp.begin(), tmp.end(), 1 << old, f)});
    }

    SqrtTree(const vector<T> &arr) : SqrtTree(arr, F()) { }
};
