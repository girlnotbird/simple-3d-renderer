#ifndef TESTRUNNER_HPP
#define TESTRUNNER_HPP

#include "TestContext.hpp"
namespace Testing
{
    class TestRunner
    {
    public:
        static std::unique_ptr<TestRunner> activeTestRunner;
    private:
        TestRunner()
        {
            activeTestRunner = std::unique_ptr<TestRunner>{this};
        };
    public:
        std::map<std::string, std::unique_ptr<TestContext>> testBlocks;

        // no public constructors.
        ~TestRunner() = default;
        TestRunner(const TestRunner& other) = delete;
        TestRunner& operator= (const TestRunner& other) = delete;
        TestRunner(TestRunner&& other) = delete;
        TestRunner& operator= (TestRunner&& other) = delete;

        // NOTE: Does not support glob matching... yet
        void runTests()
        {
            for (auto const& block : std::views::values(testBlocks))
            {
                block.get()->runTests();
            };
        };
        void runTests(const std::string& blockName)
        {
            for (auto const& [name, block] : testBlocks)
            {
                if (name != blockName) { continue; }
                block.get()->runTests();
                break;
            }
        };
        void runTests(const std::string& blockName, const std::string& testName)
        {
            for (auto const& [name, block] : testBlocks)
            {
                if (name != blockName) { continue; }
                block.get()->runTests(testName);
                break;
            }
        }
        void addBlock(TestContext&& ctx)
        {
            auto ptr = std::make_unique<TestContext>(std::move(ctx));
            testBlocks.insert_or_assign(ptr->name, std::move(ptr));
        }

        // singleton accessor
        static TestRunner& getInstance()
        {
            if (!activeTestRunner)
            {
                auto runner {std::unique_ptr<TestRunner>(new TestRunner{})};
                TestRunner::activeTestRunner = std::move(runner);
            }
            return *activeTestRunner;
        }

        // lists
        void listBlocks()
        {
            std::cout << "*----------------------------------------------------*----------------*\n";
            std::cout << "| " << RightPad(std::format("{}...", "Listing all tests"), 68) << "|\n";
            std::cout << "*----------------------------------------------------*----------------*\n";
            std::cout << "| Test Suite Name                                    | # tests        |\n";
            std::cout << "*----------------------------------------------------*----------------*\n";
            for (const auto& [name, block] : testBlocks)
            {
                auto numTests = std::to_string(block->tests.size());
                std::cout
                    << "| "
                    << RightPad(name, 49) << ' '
                    << " | "
                    << "\033[1;32m"
                    << RightPad(numTests, 14)
                    << "\033[0m"
                    << ' '
                    << "|\n";
            }
            std::cout << "*----------------------------------------------------*----------------*\n";
        }
    };

    // Public API for testing. No nesting supported (yet)
    inline void test(const std::string& name, const std::function<void()>& test, const bool skip = false)
    {
        TestRunner& runner = TestRunner::getInstance();
        if (activeContext == nullptr)
        {
            if (!runner.testBlocks.contains("Loose tests"))
            {
                runner.addBlock(TestContext{"Loose tests"});
            }
            activeContext = runner.testBlocks.at("Loose tests").get();
        }
        activeContext->test(name, test, skip);
    }
    inline void describe(const std::string& blockName, const std::function<void()>& tests)
    {
        Testing::TestRunner& runner = Testing::TestRunner::getInstance();
        runner.addBlock(TestContext {blockName});
        activeContext = runner.testBlocks.at(blockName).get();

        tests();

        activeContext = nullptr;
    }
}

// initialize activeTestRunner with singleton instance
inline std::unique_ptr<Testing::TestRunner> Testing::TestRunner::activeTestRunner {new Testing::TestRunner{}};

#endif //TESTRUNNER_HPP
