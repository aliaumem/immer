
#include <immu/rvektor.hpp>

#include <catch.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

using namespace immu;

TEST_CASE("instantiation")
{
    auto v = rvektor<int>{};
    CHECK(v.size() == 0u);
}

TEST_CASE("push back")
{
    SECTION("one element")
    {
        const auto v1 = rvektor<int>{};
        auto v2 = v1.push_back(42);
        CHECK(v1.size() == 0u);
        CHECK(v2.size() == 1u);
        CHECK(v2[0] == 42);
    }

    SECTION("many elements")
    {
        const auto n = 666u;
        auto v = rvektor<unsigned>{};
        for (auto i = 0u; i < n; ++i) {
            v = v.push_back(i * 42);
            CHECK(v.size() == i + 1);
            for (auto j = 0u; j < v.size(); ++j)
                CHECK(v[j] == j * 42);
        }
    }
}

TEST_CASE("update")
{
    const auto n = 42u;
    auto v = rvektor<unsigned>{};
    for (auto i = 0u; i < n; ++i)
        v = v.push_back(i);

    SECTION("assoc")
    {
        const auto u = v.assoc(3u, 13u);
        CHECK(u.size() == v.size());
        CHECK(u[2u] == 2u);
        CHECK(u[3u] == 13u);
        CHECK(u[4u] == 4u);
        CHECK(u[40u] == 40u);
        CHECK(v[3u] == 3u);
    }

    SECTION("assoc further")
    {
        for (auto i = n; i < 666; ++i)
            v = v.push_back(i);

        auto u = v.assoc(3u, 13u);
        u = u.assoc(200u, 7u);
        CHECK(u.size() == v.size());

        CHECK(u[2u] == 2u);
        CHECK(u[4u] == 4u);
        CHECK(u[40u] == 40u);
        CHECK(u[600u] == 600u);

        CHECK(u[3u] == 13u);
        CHECK(u[200u] == 7u);

        CHECK(v[3u] == 3u);
        CHECK(v[200u] == 200u);
    }

    SECTION("assoc further more")
    {
        auto v = immu::rvektor<unsigned, 4>{};

        for (auto i = n; i < 1000u; ++i)
            v = v.push_back(i);

        for (auto i = 0u; i < v.size(); ++i) {
            v = v.assoc(i, i+1);
            CHECK(v[i] == i+1);
        }
    }

    SECTION("update")
    {
        const auto u = v.update(10u, [] (auto x) { return x + 10; });
        CHECK(u.size() == v.size());
        CHECK(u[10u] == 20u);
        CHECK(v[40u] == 40u);

        const auto w = v.update(40u, [] (auto x) { return x - 10; });
        CHECK(w.size() == v.size());
        CHECK(w[40u] == 30u);
        CHECK(v[40u] == 40u);
    }
}

TEST_CASE("push_front")
{
    using vektor_t = rvektor<unsigned, 3>;

    const auto n = 666u;
    auto v = vektor_t{};

    for (auto i = 0u; i < n; ++i) {
        IMMU_TRACE("\n-- push_front: " << i);
        v = v.push_front(i);
        CHECK(v.size() == i + 1);
        for (auto j = 0; j < v.size(); ++j)
            CHECK(v[v.size() - j - 1] == j);
    }
}

TEST_CASE("concat")
{
    using vektor_t = rvektor<unsigned, 3>;

    const auto n = 666u;

    auto all_lhs = std::vector<vektor_t>{};
    auto all_rhs = std::vector<vektor_t>{};
    all_lhs.reserve(n);
    all_rhs.reserve(n);

    std::generate_n(std::back_inserter(all_lhs), n,
                    [v = vektor_t{},
                     i = 0u] () mutable {
                        auto r = v;
                        v = v.push_back(i++);
                        return r;
                    });

    auto v = vektor_t{};
    std::generate_n(std::back_inserter(all_rhs), n,
                    [v = vektor_t{},
                     i = n-1] () mutable {
                        auto r = v;
                        v = v.push_front(--i);
                        return r;
                    });

    SECTION("anywhere")
    {
        for (auto i = 0u; i < n; ++i) {
            auto c = all_lhs[n - i - 1] + all_rhs[i];
            IMMU_TRACE("\n-- concat: " << i);
            CHECK(c.size() == n - 1);
            for (auto j = 0u; j < c.size(); ++j)
                CHECK(c[j] == j);
        }
    }
}

TEST_CASE("reduce")
{
    SECTION("sum regular")
    {
        const auto n = 666u;
        auto v = rvektor<unsigned>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_back(i);

        auto sum = v.reduce(std::plus<unsigned>{}, 0u);
        auto expected = v.size() * (v.size() - 1) / 2;
        CHECK(sum == expected);
    }

    SECTION("sum relaxed")
    {
        const auto n = 666u;
        auto v = rvektor<unsigned>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_front(i);

        auto sum = v.reduce(std::plus<unsigned>{}, 0u);
        auto expected = v.size() * (v.size() - 1) / 2;
        CHECK(sum == expected);
    }

    SECTION("sum relaxed complex")
    {
        const auto n = 20u;

        auto v  = rvektor<unsigned, 3>{};
        for (auto i = 0u; i < n; ++i) {
            IMMU_TRACE("\n-- sum relaxed complex: " << i << " | " << v.size());
            v = v.push_front(i) + v;
        }
        /*
          0  // 0
          1  // 1 0 0
          4  // 2 1 0 0 1 0 0
          11 // 3 2 1 0 0 1 0 0 2 1 0 0 1 0 0
          26 // 4 3 2 1 0 0 1 0 0 2 1 0 0 1 0 0
        */
        auto sum = v.reduce(std::plus<unsigned>{}, 0u);
        auto expected = (1 << n) - n - 1;
        CHECK(sum == expected);
    }
}

TEST_CASE("take")
{
    const auto n = 666u;

    SECTION("anywhere")
    {
        auto v = rvektor<unsigned, 3>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_back(i);

        for (auto i = 0u; i < n; ++i) {
            auto vv = v.take(i);
            CHECK(vv.size() == i);
            for (auto j = 0u; j < i; ++j)
                CHECK(vv[j] == v[j]);
        }
    }

    SECTION("relaxed")
    {
        auto v = rvektor<unsigned, 3>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_front(i);

        for (auto i = 0u; i < n; ++i) {
            auto vv = v.take(i);
            CHECK(vv.size() == i);
            for (auto j = 0u; j < i; ++j)
                CHECK(vv[j] == v[j]);
        }
    }
}

TEST_CASE("drop")
{
    const auto n = 666u;

    SECTION("anywhere")
    {
        auto v = rvektor<unsigned, 3>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_back(i);

        for (auto i = 0u; i < n; ++i) {
            auto vv = v.drop(i);
            CHECK(vv.size() == n - i);
            for (auto j = 0u; j < n - i; ++j)
                CHECK(vv[j] == v[j + i]);
        }
    }

    SECTION("relaxed")
    {
        auto v = rvektor<unsigned, 3>{};
        for (auto i = 0u; i < n; ++i)
            v = v.push_front(i);

        SECTION("normal")
        {
            for (auto i = 0u; i < n; ++i) {
                auto vv = v.drop(i);
                CHECK(vv.size() == n - i);
                for (auto j = 0u; j < n - i; ++j)
                    CHECK(vv[j] == v[j + i]);
            }
        }
    }
}