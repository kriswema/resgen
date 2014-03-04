#include <stdexcept>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include "test.h"

#include "../LinkedList.h"
#include "../vstring.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LinkedListTest);

void LinkedListTest::setUp()
{
}

void LinkedListTest::tearDown()
{
}

void LinkedListTest::testEmptyList()
{
    LinkedList list;
    CPPUNIT_ASSERT(list.GetCount() == 0);
}

void LinkedListTest::testAddHeadSingle()
{
    LinkedList list;
    int a = 3;
	list.AddHead(&a);
    
    CPPUNIT_ASSERT(list.GetCount() == 1);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
}

void LinkedListTest::testAddHeadMultiple()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddHead(&a);
	list.AddHead(&b);
	list.AddHead(&c);
    
    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(2) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
    CPPUNIT_ASSERT(list.GetAt(0) == &c);
}

void LinkedListTest::testAddTailSingle()
{
    LinkedList list;
    int a = 3;
	list.AddTail(&a);
    
    CPPUNIT_ASSERT(list.GetCount() == 1);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
}

void LinkedListTest::testAddTailMultiple()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);
    
    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
    CPPUNIT_ASSERT(list.GetAt(2) == &c);
}

void LinkedListTest::testGetAtInvalid()
{
    LinkedList list;
    int a = 3;
	list.AddTail(&a);
    
    CPPUNIT_ASSERT_THROW(list.GetAt(1), std::out_of_range);
}

int main()
{
    using namespace CppUnit;

    TextUi::TestRunner runner;
    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    const bool bSuccess = runner.run("", false);

    return !bSuccess;
}
