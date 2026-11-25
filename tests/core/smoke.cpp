#include <catch2/catch_all.hpp>

// Le main des tests est fourni par Catch2WithMain (voir CMakeLists.txt)
import ecosim;

TEST_CASE("Le module ecosim est accessible", "[squelette]") {
    REQUIRE(ecosim::Version() == "0.1.0");
}
