#include <zip_longest.hpp>

#include "helpers.hpp"

#include <vector>
#include <tuple>
#include <string>
#include <iterator>
#include <utility>
#include <iterator>
#include <sstream>
#include <iostream>

#include "catch.hpp"

using iter::zip_longest;

// reopening boost is the only way I can find that gets this to print
namespace boost {
template <typename T>
std::ostream& operator<<(std::ostream& out, const optional<T>& opt) {
    if (opt) {
        out << "Just " << *opt;
    } else {
        out << "Nothing";
    }
    return out;
}
}

template <typename... Ts>
using const_opt_tuple = std::tuple<boost::optional<const Ts&>...>;

TEST_CASE("zip longest: correctly detects longest at any position",
        "[zip_longest]") {

    const std::vector<int> ivec{2, 4, 6, 8, 10, 12};
    const std::vector<std::string> svec{"abc", "def", "xyz"};
    const std::string str{"hello"};

    SECTION("longest first") {
        using TP = const_opt_tuple<int, std::string, char>;
        using ResVec = std::vector<TP>;

        auto zl = zip_longest(ivec, svec, str);
        ResVec results(std::begin(zl), std::end(zl));
        ResVec rc = {
            TP{{ivec[0]},  {svec[0]},   {str[0]}},
            TP{{ivec[1]},  {svec[1]},   {str[1]}},
            TP{{ivec[2]},  {svec[2]},   {str[2]}},
            TP{{ivec[3]},  {},          {str[3]}},
            TP{{ivec[4]},  {},          {str[4]}},
            TP{{ivec[5]},  {},          {}    }
        };

        REQUIRE( results == rc );
    }

    SECTION("longest in middle") {
        using TP = const_opt_tuple<std::string, int, char>;
        using ResVec = std::vector<TP>;

        auto zl = zip_longest(svec, ivec, str);
        ResVec results(std::begin(zl), std::end(zl));
        ResVec rc = {
            TP{{svec[0]},   {ivec[0]},  {str[0]}},
            TP{{svec[1]},   {ivec[1]},  {str[1]}},
            TP{{svec[2]},   {ivec[2]},  {str[2]}},
            TP{{},          {ivec[3]},  {str[3]}},
            TP{{},          {ivec[4]},  {str[4]}},
            TP{{},          {ivec[5]},  {}    }
        };

        REQUIRE( results == rc );
    }

    SECTION("longest at end") {
        using TP = const_opt_tuple<std::string, char, int>;
        using ResVec = std::vector<TP>;

        auto zl = zip_longest(svec, str, ivec);
        ResVec results(std::begin(zl), std::end(zl));
        ResVec rc = {
            TP{{svec[0]},   {str[0]},   {ivec[0]}},
            TP{{svec[1]},   {str[1]},   {ivec[1]}},
            TP{{svec[2]},   {str[2]},   {ivec[2]}},
            TP{{},          {str[3]},   {ivec[3]}},
            TP{{},          {str[4]},   {ivec[4]}},
            TP{{},          {},         {ivec[5]}}
        };

        REQUIRE( results == rc );
    }
}

TEST_CASE("zip longest: when all are empty, terminates right away",
        "[zip_longest]") {
    const std::vector<int> ivec{};
    const std::vector<std::string> svec{};
    const std::string str{};

    auto zl = zip_longest(ivec, svec, str);
    REQUIRE( std::begin(zl) == std::end(zl) );
}

TEST_CASE("zip longest: can modify zipped sequences", "[zip_longest]") {
    std::vector<int> ns1 = {1, 2, 3};
    std::vector<int> ns2 = {10, 11, 12};
    for (auto&& t : zip_longest(ns1, ns2)) {
        *std::get<0>(t) = -1;
        *std::get<1>(t) = -1;
    }

    std::vector<int> vc = {-1, -1, -1};
    REQUIRE( ns1 == vc );
    REQUIRE( ns2 == vc );
}

TEST_CASE("zip longest: empty zip_longest() is empty", "[zip_longest]") {
    auto zl = zip_longest();
    REQUIRE( std::begin(zl) == std::end(zl) );
    REQUIRE_FALSE( std::begin(zl) != std::end(zl) );
}

TEST_CASE("zip_longest: binds to lvalues, moves rvalues", "[zip_longest]") {
    itertest::BasicIterable<char> b1{'x', 'y', 'z'};
    itertest::BasicIterable<char> b2{'a', 'b'};
    SECTION("bind to first, moves second") {
        zip_longest(b1, std::move(b2));
    }
    SECTION("move first, bind to second") {
        zip_longest(std::move(b2), b1);
    }
    REQUIRE_FALSE( b1.was_moved_from() );
    REQUIRE( b2.was_moved_from() );
}

TEST_CASE("zip_longest: doesn't move or copy elements", "[zip_longest]") {
    constexpr itertest::SolidInt arr[] = {{6}, {7}, {8}};
    for (auto&& t : zip_longest(arr, arr)) {
        (void)std::get<0>(t);
    }
}