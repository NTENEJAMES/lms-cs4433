#include <gtest/gtest.h>
#include "Greeter.h"

TEST(GreeterTest, GreetsByName) {
    EXPECT_EQ(greet("Thabo"), "Hello, Thabo!");
}