#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <parlay/parallel.h>

void generate_n(std::vector<int>& all) {
    for (int i = 0; i < all.size(); i++) {
        all[i] = std::rand();
    }
}

bool is_sorted(std::vector<int>& all) {
    for (int i = 0; i < all.size() - 1; i++) {
        if (all[i] > all[i + 1]) {
            return false;
        }
    }
    return true;
}

int seq_partition(std::vector<int>& all, int l, int r) {
    int low = all[l], high = all[r];
    if (low > high) {
        int tmp = low;
        low = high;
        high = tmp;
    }
    int v = all[l + (r - l) / 2];
    if (v < low) v = low;
    else if (v > high) v = high;

    int i = l;
    int j = r;
    while (i <= j) {
        while (i <= r && all[i] < v) i++;
        while (j >= l && all[j] > v) j--;
        if (i >= j) break;
        int tmp = all[i];
        all[i] = all[j];
        all[j] = tmp;
        i++;
        j--;
    }
    return j;
}

void seq_quicksort(std::vector<int>& all, int l, int r) {
    if (l < r) {
        int q = seq_partition(all, l, r);
        seq_quicksort(all, l, q);
        seq_quicksort(all, q + 1, r);
    }
}

void par_quicksort(std::vector<int>& all, int l, int r) {
    if (l < r) {
        int q = seq_partition(all, l, r);
        if (r - l < 1000) {
            seq_quicksort(all, l, q);
            seq_quicksort(all, q + 1, r);
            return;
        }
        parlay::par_do(
            [&]() {
                par_quicksort(all, l, q);
            },
            [&]() {
                par_quicksort(all, q + 1, r);
            }
        );
    }
}

int main() {
    double seq_time = 0.0, par_time = 0.0;
    for (int i = 0; i < 5; i++) {
        std::vector<int> all(100000000);
        generate_n(all);
        std::vector<int> all_seq = all, all_par = all;
        auto now1 = std::chrono::steady_clock::now();
        seq_quicksort(all_seq, 0, all_seq.size() - 1);
        auto now2 = std::chrono::steady_clock::now();
        assert(is_sorted(all_seq));
        seq_time += std::chrono::duration<double>(now2 - now1).count();
        std::cout << "seq_time = " << std::chrono::duration<double>(now2 - now1).count() << '\n';


        auto now3 = std::chrono::steady_clock::now();
        par_quicksort(all_par, 0, all_par.size() - 1);
        auto now4 = std::chrono::steady_clock::now();
        assert(is_sorted(all_par));
        par_time += std::chrono::duration<double>(now4 - now3).count();
        std::cout << "par_time = " << std::chrono::duration<double>(now4 - now3).count() << '\n';
    }
    
    std::cout << "Mean time for seq_sort = " << seq_time / 5 << '\n';
    std::cout << "Mean time for par_sort = " << par_time / 5 << '\n';
    std::cout << "par_time is " << seq_time / par_time << " faster\n";

    return 0;
}
