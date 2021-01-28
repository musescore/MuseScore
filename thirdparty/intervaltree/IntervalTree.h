#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>

enum class Limits {
    HARD_BOTH  = 0,
    SOFT_START = 1,
    SOFT_STOP  = 2,
    SOFT_BOTH  = 3,
};

template <class T, typename K = int>
class Interval {
public:
    K start;
    K stop;
    T value;
    Interval(K s, K e, const T& v)
        : start(s)
        , stop(e)
        , value(v)
    { }
};

template <class T, typename K>
int intervalStart(const Interval<T,K>& i) {
    return i.start;
}

template <class T, typename K>
int intervalStop(const Interval<T,K>& i) {
    return i.stop;
}

template <class T, typename K>
std::ostream& operator<<(std::ostream& out, Interval<T,K>& i) {
    out << "Interval(" << i.start << ", " << i.stop << "): " << i.value;
    return out;
}

template <class T, typename K = int>
class IntervalStartSorter {
public:
    bool operator() (const Interval<T,K>& a, const Interval<T,K>& b) {
        return a.start < b.start;
    }
};

template <class T, typename K = int>
class IntervalTree {

public:
    typedef Interval<T,K> interval;
    typedef std::vector<interval> intervalVector;
    typedef IntervalTree<T,K> intervalTree;

    intervalVector intervals;
    intervalTree* left;
    intervalTree* right;
    int center;

    IntervalTree<T,K>(void)
        : left(NULL)
        , right(NULL)
        , center(0)
    { }

    IntervalTree<T,K>(const intervalTree& other) {
        center = other.center;
        intervals = other.intervals;
        if (other.left) {
            left = (intervalTree*) malloc(sizeof(intervalTree));
            *left = *other.left;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new intervalTree();
            *right = *other.right;
        } else {
            right = NULL;
        }
    }

    IntervalTree<T,K>& operator=(const intervalTree& other) {
        center = other.center;
        intervals = other.intervals;
        if (other.left) {
            left = new intervalTree();
            *left = *other.left;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new intervalTree();
            *right = *other.right;
        } else {
            right = NULL;
        }
        return *this;
    }

    IntervalTree<T,K>(
            intervalVector& ivals,
            unsigned int depth = 16,
            unsigned int minbucket = 64,
            int leftextent = 0,
            int rightextent = 0,
            unsigned int maxbucket = 512
            )
        : left(NULL)
        , right(NULL)
    {

        --depth;
        IntervalStartSorter<T,K> intervalStartSorter;
        if (depth == 0 || (ivals.size() < minbucket && ivals.size() < maxbucket)) {
            sort(ivals.begin(), ivals.end(), intervalStartSorter);
            intervals = ivals;
        } else {
            if (leftextent == 0 && rightextent == 0) {
                // sort intervals by start
                sort(ivals.begin(), ivals.end(), intervalStartSorter);
            }

            int leftp = 0;
            int rightp = 0;
            int centerp = 0;

            if (leftextent || rightextent) {
                leftp = leftextent;
                rightp = rightextent;
            } else {
                leftp = ivals.front().start;
                std::vector<K> stops;
                stops.resize(ivals.size());
                transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<T,K>);
                rightp = *max_element(stops.begin(), stops.end());
            }

            //centerp = ( leftp + rightp ) / 2;
            centerp = ivals.at(ivals.size() / 2).start;
            center = centerp;

            intervalVector lefts;
            intervalVector rights;

            for (typename intervalVector::iterator i = ivals.begin(); i != ivals.end(); ++i) {
                interval& interval = *i;
                if (interval.stop < center) {
                    lefts.push_back(interval);
                } else if (interval.start > center) {
                    rights.push_back(interval);
                } else {
                    intervals.push_back(interval);
                }
            }

            if (!lefts.empty()) {
                left = new intervalTree(lefts, depth, minbucket, leftp, centerp);
            }
            if (!rights.empty()) {
                right = new intervalTree(rights, depth, minbucket, centerp, rightp);
            }
        }
    }

    void findOverlapping(K start, K stop, intervalVector& overlapping, Limits limits = Limits::HARD_BOTH) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            switch (limits) {
                case Limits::HARD_BOTH:
                default: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.stop >= start && interval.start <= stop) {
                            overlapping.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_START: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.stop > start && interval.start <= stop) {
                            overlapping.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_STOP: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.stop >= start && interval.start < stop) {
                            overlapping.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_BOTH: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.stop > start && interval.start < stop) {
                            overlapping.push_back(interval);
                        }
                    }
                    break;
                }
            }
        }

        if (left && start <= center) {
            left->findOverlapping(start, stop, overlapping, limits);
        }

        if (right && stop >= center) {
            right->findOverlapping(start, stop, overlapping, limits);
        }

    }

    void findContained(K start, K stop, intervalVector& contained, Limits limits = Limits::HARD_BOTH) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            switch (limits) {
                case Limits::HARD_BOTH:
                default: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.start >= start && interval.stop <= stop) {
                            contained.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_START: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.start > start && interval.stop <= stop) {
                            contained.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_STOP: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.start >= start && interval.stop < stop) {
                            contained.push_back(interval);
                        }
                    }
                    break;
                }
                case Limits::SOFT_BOTH: {
                    for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                        interval& interval = *i;
                        if (interval.start > start && interval.stop < stop) {
                            contained.push_back(interval);
                        }
                    }
                    break;
                }
            }
        }

        if (left && start <= center) {
            left->findContained(start, stop, contained, limits);
        }

        if (right && stop >= center) {
            right->findContained(start, stop, contained, limits);
        }

    }

    ~IntervalTree(void) {
        // traverse the left and right
        // delete them all the way down
        if (left) {
            delete left;
        }
        if (right) {
            delete right;
        }
    }

};

#endif
