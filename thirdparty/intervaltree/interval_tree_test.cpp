#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <time.h>
#include <assert.h>
#include "IntervalTree.h"

using namespace std;

typedef Interval<bool> interval;
typedef vector<interval> intervalVector;
typedef IntervalTree<bool> intervalTree;

template<typename K>
K randKey(K floor, K ceiling) {
    K range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

template<class T, typename K>
Interval<T,K> randomInterval(K maxStart, K maxLength, K maxStop, const T& value) {
    K start = randKey<K>(0, maxStart);
    K stop = min<K>(randKey<K>(start, start + maxLength), maxStop);
    return Interval<T,K>(start, stop, value);
}

int main() {
    typedef vector<std::size_t> countsVector;

    srand((unsigned)time(NULL));

    intervalVector intervals;
    intervalVector queries;

    // generate a test set of target intervals
    for (int i = 0; i < 10000; ++i) {
        intervals.push_back(randomInterval<bool>(100000, 1000, 100000 + 1, true));
    }
    // and queries
    for (int i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval<bool>(100000, 1000, 100000 + 1, true));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    // using brute-force search
    countsVector bruteforcecounts;
    Clock::time_point t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        for (intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
            if (i->start >= q->start && i->stop <= q->stop) {
                results.push_back(*i);
            }
        }
        bruteforcecounts.push_back(results.size());
    }
    Clock::time_point t1 = Clock::now();
    milliseconds ms = chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "brute force:\t" << ms.count() << "ms" << endl;

    // using the interval tree
    intervalTree tree = intervalTree(intervals);
    countsVector treecounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        tree.findContained(q->start, q->stop, results);
        treecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "interval tree:\t" << ms.count() << "ms" << endl;

    // check that the same number of results are returned
    countsVector::iterator b = bruteforcecounts.begin();
    for (countsVector::iterator t = treecounts.begin(); t != treecounts.end(); ++t, ++b) {
        assert(*b == *t);
    }

    return 0;
}

