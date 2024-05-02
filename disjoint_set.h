#include <vector>
#include <cassert>

struct not_implemented {};

class disjoint_set {
    struct node {
        int index;     // Set index
        node* parent;  // Parent set (nullptr for roots)
        int rank = 0;
    };

public:
    disjoint_set(int elem_count) 
        : total_elems(elem_count)
    {
        forest = new node * [elem_count];
        for (int i = 0; i < elem_count; ++i) {
            forest[i] = new node{ i, nullptr, 0 };
        }
    }

    ~disjoint_set() {
        for (int i = 0; i < total_elems; ++i) {
            delete forest[i];
        }
        delete[] forest;
    }

    disjoint_set(const disjoint_set& original)
        : total_elems(original.total_elems)
    {
        forest = new node * [total_elems];
        for (int i = 0; i < total_elems; ++i) {
            forest[i] = new node{ original.forest[i]->index, nullptr, original.forest[i]->rank };
        }

        // Update parent pointers after copying
        for (int i = 0; i < total_elems; ++i) {
            if (original.forest[i]->parent != nullptr) {
                forest[i]->parent = forest[original.forest[i]->parent->index];
            }
        }
    }

    disjoint_set& operator= (const disjoint_set& original) {
        if (this != &original) {
            // Copy new data
            total_elems = original.total_elems;
            forest = new node * [total_elems];
            for (int i = 0; i < total_elems; ++i) {
                forest[i] = new node{ original.forest[i]->index, nullptr, original.forest[i]->rank };
            }

            // Update parent pointers after copying
            for (int i = 0; i < total_elems; ++i) {
                if (original.forest[i]->parent != nullptr) {
                    forest[i]->parent = forest[original.forest[i]->parent->index];
                }
            }
        }
        return *this;
    }

    int elem_count() const {
        return total_elems;
    }

    int set_count() const {
        int count = 0;
        for (int i = 0; i < total_elems; ++i) {
            if (forest[i]->parent == nullptr) {
                ++count;
            }
        }
        return count;
    }

    bool singleton() const {
        if (set_count() == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    int rep(int n) {
        assert(n >= 0 and n < total_elems);
        if (forest[n]->parent == nullptr) {
            return n;
        }
        // Path compression
        forest[n]->parent = forest[rep(forest[n]->parent->index)];
        return forest[n]->parent->index;
    }

    int rep(int n) const {
        assert(n >= 0 and n < total_elems);
        if (forest[n]->parent == nullptr) {
            return n;
        }
        return forest[n]->parent->index;
    }

    bool in_same_set(int a, int b) {
        return rep(a) == rep(b);
    }

    bool merge(int a, int b) {
        assert(a >= 0 and a < total_elems);
        assert(b >= 0 and b < total_elems);

        int rep_a = rep(a);
        int rep_b = rep(b);

        if (rep_a == rep_b) {
            return false;  // Already in the same set
        }

        // Merge by rank
        if (forest[rep_a]->rank < forest[rep_b]->rank) {
            forest[rep_a]->parent = forest[rep_b];
        }
        else if (forest[rep_a]->rank > forest[rep_b]->rank) {
            forest[rep_b]->parent = forest[rep_a];
        }
        else {
            // If ranks are equal, arbitrarily choose one as parent and increment its rank
            forest[rep_a]->parent = forest[rep_b];
            forest[rep_b]->rank += 1;
        }

        return true;
    }

    std::vector<int> elements(int n) {
        assert(n >= 0 and n < total_elems);

        std::vector<int> result;
        int rep_n = rep(n);

        for (int i = 0; i < total_elems; ++i) {
            if (rep(i) == rep_n) {
                result.push_back(i);
            }
        }

        return result;
    }

public:
    node** forest = nullptr;
    int total_elems = -1;
};
