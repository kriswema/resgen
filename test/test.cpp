#include <stdexcept>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include "test.h"

int main()
{
    using namespace CppUnit;

    TextUi::TestRunner runner;
    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    const bool bSuccess = runner.run("", false);

    return !bSuccess;
}
