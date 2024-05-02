/*
 * assign6_test.cpp
 * Assignment 6 test runner.
 */
#include <iostream>

#ifdef TEST_SOLUTION
#include "assign6_soln.hpp"
#else
#include "disjoint_set.hpp"
#endif

#if defined(__WIN64) || defined(__WIN32)

 // Until I figure out how to enable ANSI codes in the terminal...
#define ANSI_NORMAL() ""
#define ANSI_GREEN()  ""
#define ANSI_YELLOW() ""
#define ANSI_RED()    ""
#define ANSI_UL()     ""

#else

#define ANSI_NORMAL() "\e[0m"
#define ANSI_GREEN()  "\e[0;92m"
#define ANSI_YELLOW() "\e[0;93m"
#define ANSI_RED()    "\e[0;91m"
#define ANSI_UL()     "\e[4m"

#endif

#define CHECK(cond, test, ...)                                                \
    try {                                                                     \
        do {                                                                      \
            std::cout << "Testing: " << test << "... ";                           \
            std::cout.flush();                                                    \
            if(not(cond)) {                                                       \
                std::cout << ANSI_RED() << "FAIL: " << __VA_ARGS__ << ANSI_NORMAL() << std::endl; \
                result = false;                                                   \
            }                                                                     \
            else                                                                  \
                std::cout << ANSI_GREEN() << "OK." << ANSI_NORMAL() << std::endl; \
        } while(false);                                                           \
    }                                                                             \
    catch(not_implemented&) {                                                     \
        std::cout << ANSI_YELLOW() << "Not implemented, skipping." << ANSI_NORMAL() << std::endl; \
        result = false;                                                           \
    }

 // A version of CHECK that doesn't print anything on success.
#define CHECK_SILENT(cond, test, ...)                                             \
    try {                                                                         \
        do {                                                                      \
            if(not(cond)) {                                                       \
                std::cout << ANSI_RED() << "Testing " << test << " FAIL: " << __VA_ARGS__ << ANSI_NORMAL() << std::endl; \
                result = false;                                                   \
            }                                                                     \
        } while(false);                                                           \
    }                                                                             \
    catch(not_implemented&) {                                                     \
        std::cout << "Testing " << test << " " << ANSI_YELLOW() <<                \
                     "Not implemented, skipping." << ANSI_NORMAL() << std::endl;  \
        result = false;                                                           \
    }    

#define TITLE(title)                                                          \
    std::cout << ANSI_UL() << "Testing " << title << ANSI_NORMAL() << std::endl;

// ============================================================================

class disjoint_set_tester {
public:
    bool start()
    {
        bool result = true;

        result = result & test_construction();
        result = result & test_copying();
        result = result & test_merging();
        result = result & test_path_compression();
        result = result & test_merge_by_rank();
        result = result & test_utilities();

        return result;
    }

    bool test_construction()
    {
        TITLE("disjoint set construction");

        disjoint_set ds(10);
        bool result = true;

        CHECK(ds.elem_count() == 10, "Initial elem_count()",
            "disjoint_set constructed with 10 elements, but elem_count() == " << ds.elem_count());

        CHECK(ds.set_count() == 10, "Initial set_count()",
            "disjoint_set constructed with 10 elements (no merging), but set_count() == " << ds.set_count());

        CHECK(ds.forest != nullptr, "Forest pointer",
            "disjoint_set constructed with 10 elements, but forest pointer == nullptr");

        // If the forest is allocated, check the individual nodes.
        if (ds.forest != nullptr)
            for (int i = 0; i < 10; ++i) {
                CHECK_SILENT(ds.forest[i] != nullptr, "Node pointers",
                    "disjoint_set constructed with 10 elements, but not all nodes are allocated.");

                if (ds.forest[i] != nullptr) {
                    CHECK_SILENT(ds.forest[i]->index == i, "Node index",
                        "Node at index " << i << " has its .index = " << ds.forest[i]->index);

                    CHECK_SILENT(ds.forest[i]->parent == nullptr, "Parent pointers",
                        "Node at index " << i << " has a non-null parent pointer");

                    CHECK_SILENT(ds.forest[i]->rank == 0, "Initial rank",
                        "All nodes should have initial rank of 0, but " << i << "'s rank = " <<
                        ds.forest[i]->rank);

                    try {
                        CHECK_SILENT(ds.in_same_set(i, i), "in_same_set(x,x)",
                            "in_same_set(x,x) should always be true, but in_same_set(" <<
                            i << "," << i << ") = " << ds.in_same_set(i, i));
                    }
                    catch (not_implemented&) {
                        std::cout << ANSI_YELLOW() << "in_same_set() not implemented, skipping"
                            << ANSI_NORMAL() << std::endl;
                        result = false;
                    }
                }
            }

        return result;
    }

    bool test_copying()
    {
        TITLE("copying");
        bool result = true;

        try {
            disjoint_set a(10);

            // Merge all even numbers in a
            for (int i = 0; i < a.elem_count() - 2; i += 2)
                a.merge(i, i + 2);

            // Copy constructor
            disjoint_set b = a;

            // Make sure b is identical to a
            CHECK(a.elem_count() == b.elem_count(), "identical elem. counts",
                "b is a copy of a (elem. count = 10), but b's elem. count = " <<
                b.elem_count());

            int root = b.rep(0);
            for (int i = 2; i < b.elem_count(); i += 2)
                CHECK_SILENT(b.rep(i) == root, "Same sets",
                    "b (copy of a) should have {0,2,4,6,8} in same set but it does not (" <<
                    "rep(0) = " << root << ", but rep(" << i << ") = " << b.rep(i) << ")");

            for (int i = 1; i < b.elem_count(); i += 2)
                CHECK_SILENT(b.rep(i) == i, "Same sets",
                    "b (copy of a) should have {" << i << "} in its own set but it does not");

            // Copy should not share pointers with the original
            for (int i = 0; i < b.elem_count(); ++i)
                CHECK_SILENT(a.forest[i] != b.forest[i], "Pointer sharing",
                    "b (copy of a) should have its own node*s, but it is using a's");

            // Copy's parent pointers should all point to nodes within the copy
            for (int i = 0; i < b.elem_count(); ++i) {
                bool found = false;

                // Ignore root nodes
                if (b.forest[i]->parent == nullptr)
                    continue;

                for (int j = 0; j < b.elem_count(); ++j)
                    if (b.forest[j] == b.forest[i]->parent) {
                        found = true;
                        break;
                    }

                CHECK(found, "Parent pointer sharing",
                    "b's (copy of a) parent pointers should all point to nodes within b, "
                    "but some do not");
            }

            // Modifying b should not affect a
            b.merge(1, 3);
            CHECK(a.rep(1) != a.rep(3), "Deep copy",
                "Modifying b (copy of a) should not affect a, but it did");

            // Copy assignment
            disjoint_set c(5);
            c = a;

            // Make sure c is identical to a
            CHECK(a.elem_count() == c.elem_count(), "identical elem. counts",
                "c is a copy of a (elem. count = 10), but c's elem. count = " <<
                c.elem_count());

            root = c.rep(0);
            for (int i = 2; i < c.elem_count(); i += 2)
                CHECK_SILENT(c.rep(i) == root, "Same sets",
                    "v (copy of a) should have {0,2,4,6,8} in same set but it does not (" <<
                    "rep(0) = " << root << ", but rep(" << i << ") = " << c.rep(i) << ")");

            for (int i = 1; i < c.elem_count(); i += 2)
                CHECK_SILENT(c.rep(i) == i, "Same sets",
                    "c (copy of a) should have {" << i << "} in its own set but it does not");

            // Modifying c should not affect a
            c.merge(1, 3);
            CHECK(a.rep(1) != a.rep(3), "Deep copy",
                "Modifying c (copy of a) should not affect a, but it did");
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "merge() not implemented, skipping." <<
                ANSI_NORMAL() << std::endl;

            return false;
        }

        return result;
    }

    bool test_merging()
    {
        TITLE("merging");

        bool result = true;

        disjoint_set ds(10);

        std::cout << "Constructing a disjoint set of size 10, then merging {3,6,9} and {4,8}..."
            << std::endl;

        // All elements divisible by 3 are in the same set, as are all elements 
        // divisible by 4 (not including 0).

        // This set should have three elements
        try {
            ds.merge(3, 6); // Rank = 1
            ds.merge(6, 9); // Rank = 1 ({9} merged with {3,6})

            // This set should have two elements
            ds.merge(4, 8); // Rank = 1

            // There should be 
            //     0, 1, 2, {3,6,9}, {4,8}, 5, 7
            // 7 sets in total now
            CHECK(ds.set_count() == 7, "Set count",
                "After merging {3,6,9} and {4,8} there should be 7 sets, but " <<
                "set_count() = " << ds.set_count());

            CHECK(ds.rep(4) == 4 or ds.rep(4) == 8, "Rep consistency",
                "{4} and {8} were merged but rep(4) = " << ds.rep(4));
            CHECK(ds.rep(8) == 4 or ds.rep(8) == 8, "Rep consistency",
                "{4} and {8} were merged but rep(4) = " << ds.rep(8));
            CHECK(ds.rep(4) == ds.rep(8), "Rep consistency",
                "{4} and {8} were merged by rep(4) != rep(8) (" <<
                ds.rep(4) << " != " << ds.rep(8) << ")");

            CHECK(ds.rep(3) == 3 or ds.rep(3) == 6 or ds.rep(3) == 9, "Rep consistency",
                "{3}, {6} and {9} were merged but rep(3) = " << ds.rep(3));
            CHECK(ds.rep(6) == 3 or ds.rep(6) == 6 or ds.rep(6) == 9, "Rep consistency",
                "{3}, {6} and {9} were merged but rep(6) = " << ds.rep(6));
            CHECK(ds.rep(9) == 3 or ds.rep(9) == 6 or ds.rep(9) == 9, "Rep consistency",
                "{3}, {6} and {9} were merged but rep(9) = " << ds.rep(3));
            CHECK(ds.rep(3) == ds.rep(6) and ds.rep(6) == ds.rep(9), "Rep consistency",
                "{3}, {6}, and {9} were merged but they still have different reps (" <<
                "rep(3) = " << ds.rep(3) << ", " <<
                "rep(6) = " << ds.rep(6) << ", " <<
                "rep(9) = " << ds.rep(9) << ")");

            // All other sets should still have the same reps
            for (int x : { 0, 1, 2, 5, 7 })
                CHECK_SILENT(ds.rep(x) == x, "Rep consistency",
                    "{" << x << "} was NOT merged, but its rep changed (" <<
                    "rep(" << x << ") = " << ds.rep(x));

            // As we've now called rep() on 3, 6, and 9, the full path should
            // be compressed, so we check that.
            int root = ds.rep(3);
            CHECK(ds.forest[3]->parent == nullptr or
                ds.forest[3]->parent == ds.forest[root], "Path compression",
                "After calling rep(3), rep(6), rep(9), path from 3 is not compressed");
            CHECK(ds.forest[6]->parent == nullptr or
                ds.forest[6]->parent == ds.forest[root], "Path compression",
                "After calling rep(3), rep(6), rep(9), path from 6 is not compressed");
            CHECK(ds.forest[9]->parent == nullptr or
                ds.forest[9]->parent == ds.forest[root], "Path compression",
                "After calling rep(3), rep(6), rep(9), path from 9 is not compressed");

            CHECK(ds.forest[root]->rank == 1, "Rank after merge",
                "{3,6,9} should have rank 1, but rank = " << ds.forest[root]->rank);

            for (int i = 0; i < 10; ++i)
                CHECK_SILENT(ds.in_same_set(i, i), "in_same_set(x,x)",
                    "in_same_set(x,x) should always be true, but in_same_set(" <<
                    i << "," << i << ") = " << ds.in_same_set(i, i));

            CHECK(ds.in_same_set(3, 6), "in_same_set(3,6)",
                "After merging {3,6,9}, in_same_set(3,6) should be true but it isn't");
            CHECK(ds.in_same_set(3, 9), "in_same_set(3,9)",
                "After merging {3,6,9}, in_same_set(3,9) should be true but it isn't");
            CHECK(ds.in_same_set(6, 9), "in_same_set(6,9)",
                "After merging {3,6,9}, in_same_set(6,9) should be true but it isn't");
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "merge() not implemented, skipping." <<
                ANSI_NORMAL() << std::endl;

            return false;
        }

        return result;
    }

    bool test_path_compression()
    {
        bool result = true;

        TITLE("path compression");

        // Depending on how students implement merge (when the ranks are 
        // equal), the pathological case may occur when merging (0,1), (1,2) ...
        // or when merging (8,9), (7,8), ... 
        try {
            disjoint_set ds(10);

            // Merge *all* sets
            for (int i = 8; i >= 0; --i)
                ds.merge(i, i + 1);

            CHECK(ds.set_count() == 1, "Singleton set",
                "After merging all sets, set_count() should be 1 but it = " <<
                ds.set_count());

            // At this point, 9 is possibly the bottom of the path. This should
            // compress the entire path.
            int root = ds.rep(9);

            for (int i = 0; i < 10; ++i)
                if (i != root)
                    CHECK_SILENT(ds.forest[i]->parent == ds.forest[root], "Path compression",
                        "Path should be compressed, but it isn't");
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "Not implemented, skipping"
                << ANSI_NORMAL() << std::endl;
            return false;
        }

        try {
            disjoint_set ds(10);

            // Merge *all* sets
            for (int i = 0; i < 9; ++i)
                ds.merge(i, i + 1);

            CHECK(ds.set_count() == 1, "Singleton set",
                "After merging all sets, set_count() should be 1 but it = " <<
                ds.set_count());

            // At this point, 0 is possibly the bottom of the path. This should
            // compress the entire path.
            int root = ds.rep(0);

            for (int i = 0; i < 10; ++i)
                if (i != root)
                    CHECK_SILENT(ds.forest[i]->parent == ds.forest[root], "Path compression",
                        "Path should be compressed, but it isn't");
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "Not implemented, skipping"
                << ANSI_NORMAL() << std::endl;
            return false;
        }

        return result;
    }

    bool test_merge_by_rank()
    {
        TITLE("merge by rank");

        bool result = true;
        try {
            disjoint_set ds(8);

            // Merge pairs
            for (int i = 0; i < ds.elem_count(); i += 2)
                ds.merge(i, i + 1);

            CHECK(ds.set_count() == 4, "Set count",
                "After merging {0,1}, {2,3}, {4,5}, {6,7}, set count should be 4 but " <<
                "it is " << ds.set_count());

            for (int i = 0; i < ds.elem_count(); ++i)
                CHECK_SILENT(ds.forest[ds.rep(i)]->rank == 1, "Merge by rank",
                    "After merging two sets of rank 0, resulting rank should be 1 but it is "
                    << ds.forest[ds.rep(i)]->rank);

            // Merge further pairs; now there should be two sets.
            ds.merge(0, 2);
            ds.merge(4, 6);

            CHECK(ds.set_count() == 2, "Set count",
                "After merging {0,1,2,3}, {4,5,6,7}, set count should be 2 but " <<
                "it is " << ds.set_count());

            for (int i = 0; i < ds.elem_count(); ++i)
                CHECK_SILENT(ds.forest[ds.rep(i)]->rank == 2, "Merge by rank",
                    "After merging two sets of rank 1, resulting rank should be 2 but it is "
                    << ds.forest[ds.rep(i)]->rank);

            ds.merge(0, 4); // Just one set now.
            CHECK(ds.set_count() == 1, "Set count",
                "After merging {0,1,2,3,4,5,6,7}, set count should be 1 but " <<
                "it is " << ds.set_count());
            for (int i = 0; i < ds.elem_count(); ++i)
                CHECK_SILENT(ds.forest[ds.rep(i)]->rank == 3, "Merge by rank",
                    "After merging two sets of rank 2, resulting rank should be 3 but it is "
                    << ds.forest[ds.rep(i)]->rank);

            CHECK(ds.singleton(), "singleton()",
                "After merging all sets, singleton() should be true but it isn't");
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "Not implemented, skipping"
                << ANSI_NORMAL() << std::endl;
            return false;
        }

        return result;
    }

    bool test_utilities()
    {
        TITLE("utilities");

        bool result = true;

        try {
            disjoint_set ds(10);

            for (int i = 0; i < ds.elem_count(); ++i)
                CHECK_SILENT(ds.elements(i) == std::vector<int>{i}, "elements()",
                    "Initially, each set should be in its own set.");

            ds.merge(3, 6);
            ds.merge(6, 9);
            ds.merge(0, 3);

            for (int x : { 0, 3, 6, 9 }) {
                std::vector<int> s = ds.elements(x);

                CHECK(s.size() == 4, "elements()",
                    "After merging {0,3,6,9}, size of elements(" << x << ") " <<
                    "should be 4, but it is " << s.size());

                // All of 0, 3, 6, 9 should be elements of s
                for (int y : {0, 3, 6, 9}) {
                    bool found = true;
                    for (int z : s)
                        if (z == y) {
                            found = true;
                            break;
                        }

                    CHECK_SILENT(found, "elements() contains elements",
                        "After merging {0,3,6,9} elements(3) should contain all four " <<
                        "elements, but it doesn't");
                }
            }
        }
        catch (not_implemented&) {
            std::cout << ANSI_YELLOW() << "Not implemented, skipping"
                << ANSI_NORMAL() << std::endl;
            return false;
        }

        return result;
    }
};

int main()
{
    disjoint_set_tester dst;
    bool result = dst.start();

    if (result == true) {
        std::cout << ANSI_GREEN() << "**** All Tests Passed! ****"
            << ANSI_NORMAL() << std::endl;
        return 0;
    }
    else {
        std::cout << ANSI_RED() << "**** Some tests failed/not implemented ****"
            << ANSI_NORMAL() << std::endl;
    }

    return result == true ? 0 : 1;
}
