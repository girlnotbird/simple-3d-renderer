#ifndef TESTCONTEXT_HPP
#define TESTCONTEXT_HPP

#include <functional>
#include <iostream>
#include <string_view>
#include <map>
#include <ranges>

#include "IOUtils.hpp"

enum class TestDisposition
{
    PASS,
    FAIL,
    SKIP
};

class TestContext
{
public:
    std::string name;
    std::map<std::string, std::pair<std::function<void()>, bool>> tests{};
    explicit TestContext(std::string name_) : name(std::move(name_)) {}

    ~TestContext() = default;
    TestContext(TestContext&& other) noexcept : name{std::move(other.name)}, tests{std::move(other.tests)} {};
    TestContext& operator=(TestContext&& other) noexcept
    {
        if (this != &other) { return *this; }
        name = other.name;
        tests = std::move(other.tests);
        return *this;
    };
    TestContext(const TestContext& other) = delete;
    TestContext& operator=(const TestContext& other) = delete;

    void test(const std::string& name, const std::function<void()>& test, bool skip = false)
    {
        this->tests[name] = std::pair(test, skip);
    };

    void runTests() const
    {
        std::map<std::string, TestDisposition> results;
        for (const auto& entry : this->tests)
        {
            auto [name, testAndSkip] = entry;
            auto [test, skip] = testAndSkip;
            if (skip)
            {
                results[name] = TestDisposition::SKIP;
                continue;
            }
            try
            {
                test();
                results[name] = TestDisposition::PASS;
            }
            catch (const std::exception& err)
            {
                results[name] = TestDisposition::FAIL;
                std::cerr << "\nError in test: " << name << "\n" << "Error: " << err.what() << std::endl;
            }
        }
        this->outputResults(results);
    };

    void runTests(const std::string& testName) const
    {
        std::map<std::string, TestDisposition> results;
        for (const auto& entry : this->tests)
        {
            auto [name, testAndSkip] = entry;
            auto [test, skip] = testAndSkip;
            if (skip || name != testName)
            {
                results[name] = TestDisposition::SKIP;
                continue;
            }
            try
            {
                test();
                results[name] = TestDisposition::PASS;
            }
            catch (const std::exception& err)
            {
                results[name] = TestDisposition::FAIL;
                std::cerr << "Error in test: " << name << "\n" << "Error: " << err.what() << "\n";
            }
        }
        this->outputResults(results);
    }

    template <typename T, typename U>
    friend void testAssertEq(const T& t, const U& u);

private:
    void outputResults(const std::map<std::string, TestDisposition>& results) const
    {
        std::cout << "*----------------------------------------------------*----------------*\n";
        std::cout << "| Testing " << RightPad(std::format("{}...", this->name), 60) << "|\n";
        std::cout << "*----------------------------------------------------*----------------*\n";
        std::cout << "| Test Name                                          | Passed?        |\n";
        std::cout << "*----------------------------------------------------*----------------*\n";
        for (const auto& entry : results)
        {
            auto [name, res] = entry;
            std::cout
                << "| "
                << RightPad(name, 49) << ' '
                << " | "
                << (res == TestDisposition::SKIP ? "\033[1;33m" : res == TestDisposition::FAIL ? "\033[1;31m" : "\033[1;32m")
                << RightPad((res == TestDisposition::SKIP ? "SKIP" : (res == TestDisposition::FAIL) ? "FAIL" : "PASS"), 14)
                << "\033[0m"
                << ' '
                << "|\n";
        }
        std::cout << "*----------------------------------------------------*----------------*\n";
        std::cout << "\033[2m///////////////////////////////////////////////////////////////////////\033[0m\n";
    };
};

inline TestContext* activeContext = nullptr;

template <typename T, typename U>
void testAssertEq(const T& t, const U& u)
{
    if (t != u)
    {
        throw std::runtime_error(
            std::format("\n   \033[1;31mExpected: {}\n   Received: {}\033[0m\n", u, t));
    }
};

#endif //TESTCONTEXT_HPP
