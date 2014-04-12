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
    
    CPPUNIT_ASSERT_THROW(list.GetAt(-1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(list.GetAt(1), std::out_of_range);
}

void LinkedListTest::testInsertAtStart()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.InsertAt(&c, 0);
    
    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(0) == &c);
    CPPUNIT_ASSERT(list.GetAt(1) == &a);
    CPPUNIT_ASSERT(list.GetAt(2) == &b);
}

void LinkedListTest::testInsertAtMiddle()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
    int d = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);
	list.InsertAt(&d, 1);
    
    CPPUNIT_ASSERT(list.GetCount() == 4);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &d);
    CPPUNIT_ASSERT(list.GetAt(2) == &b);
    CPPUNIT_ASSERT(list.GetAt(3) == &c);
}

void LinkedListTest::testInsertAtEnd()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.InsertAt(&c, 2);
    
    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
    CPPUNIT_ASSERT(list.GetAt(2) == &c);
}

void LinkedListTest::testInsertAtInvalid()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);

    CPPUNIT_ASSERT_THROW(list.InsertAt(&c, -1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(list.InsertAt(&c, 3), std::out_of_range);
    
    CPPUNIT_ASSERT(list.GetCount() == 2);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
}

void LinkedListTest::testRemoveAtStart()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);
    
	list.RemoveAt(0);

    CPPUNIT_ASSERT(list.GetCount() == 2);
    CPPUNIT_ASSERT(list.GetAt(0) == &b);
    CPPUNIT_ASSERT(list.GetAt(1) == &c);
}

void LinkedListTest::testRemoveAtMiddle()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);
    
	list.RemoveAt(1);

    CPPUNIT_ASSERT(list.GetCount() == 2);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &c);
}

void LinkedListTest::testRemoveAtEnd()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);
    
	list.RemoveAt(2);

    CPPUNIT_ASSERT(list.GetCount() == 2);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
}

void LinkedListTest::testRemoveAtInvalid()
{
    LinkedList list;
    int a = 3;
    int b = 5;
    int c = -1;
	list.AddTail(&a);
	list.AddTail(&b);
	list.AddTail(&c);

    CPPUNIT_ASSERT_THROW(list.RemoveAt(-1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(list.RemoveAt(3), std::out_of_range);

    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);
    CPPUNIT_ASSERT(list.GetAt(2) == &c);
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
