#include <stdexcept>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include "test.h"

#include "../LinkedList.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LinkedListTest);

int compareFunc(int* const& a, int* const& b)
{
	return *a - *b;
}

void LinkedListTest::setUp()
{
    a = 3;
    b = 5;
    c = -1;
	list1.AddTail(&a);
	list1.AddTail(&b);
	list1.AddTail(&c);
}

void LinkedListTest::tearDown()
{
}

void LinkedListTest::testEmptyList()
{
    LinkedList<int*> list;
    CPPUNIT_ASSERT(list.GetCount() == 0);
}

void LinkedListTest::testAddHeadSingle()
{
    LinkedList<int*> list;
    int a = 3;
	list.AddHead(&a);
    
    CPPUNIT_ASSERT(list.GetCount() == 1);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
}

void LinkedListTest::testAddHeadMultiple()
{
    LinkedList<int*> list;
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
    LinkedList<int*> list;
    int a = 3;
	list.AddTail(&a);
    
    CPPUNIT_ASSERT(list.GetCount() == 1);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
}

void LinkedListTest::testAddTailMultiple()
{
    CPPUNIT_ASSERT(list1.GetCount() == 3);
    CPPUNIT_ASSERT(list1.GetAt(0) == &a);
    CPPUNIT_ASSERT(list1.GetAt(1) == &b);
    CPPUNIT_ASSERT(list1.GetAt(2) == &c);
}

void LinkedListTest::testGetAtInvalid()
{
    LinkedList<int*> list;
    int a = 3;
	list.AddTail(&a);
    
    CPPUNIT_ASSERT_THROW(list.GetAt(-1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(list.GetAt(1), std::out_of_range);
}

void LinkedListTest::testInsertAtStart()
{
    LinkedList<int*> list;
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
    int d = -1;
	list1.InsertAt(&d, 1);
    
    CPPUNIT_ASSERT(list1.GetCount() == 4);
    CPPUNIT_ASSERT(list1.GetAt(0) == &a);
    CPPUNIT_ASSERT(list1.GetAt(1) == &d);
    CPPUNIT_ASSERT(list1.GetAt(2) == &b);
    CPPUNIT_ASSERT(list1.GetAt(3) == &c);
}

void LinkedListTest::testInsertAtEnd()
{
    LinkedList<int*> list;
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
    LinkedList<int*> list;
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
	list1.RemoveAt(0);

    CPPUNIT_ASSERT(list1.GetCount() == 2);
    CPPUNIT_ASSERT(list1.GetAt(0) == &b);
    CPPUNIT_ASSERT(list1.GetAt(1) == &c);
}

void LinkedListTest::testRemoveAtMiddle()
{
	list1.RemoveAt(1);

    CPPUNIT_ASSERT(list1.GetCount() == 2);
    CPPUNIT_ASSERT(list1.GetAt(0) == &a);
    CPPUNIT_ASSERT(list1.GetAt(1) == &c);
}

void LinkedListTest::testRemoveAtEnd()
{
	list1.RemoveAt(2);

    CPPUNIT_ASSERT(list1.GetCount() == 2);
    CPPUNIT_ASSERT(list1.GetAt(0) == &a);
    CPPUNIT_ASSERT(list1.GetAt(1) == &b);
}

void LinkedListTest::testRemoveAtInvalid()
{
    CPPUNIT_ASSERT_THROW(list1.RemoveAt(-1), std::out_of_range);
    CPPUNIT_ASSERT_THROW(list1.RemoveAt(3), std::out_of_range);

    CPPUNIT_ASSERT(list1.GetCount() == 3);
    CPPUNIT_ASSERT(list1.GetAt(0) == &a);
    CPPUNIT_ASSERT(list1.GetAt(1) == &b);
    CPPUNIT_ASSERT(list1.GetAt(2) == &c);
}

void LinkedListTest::testFind()
{
	// Use different addresses to make sure find is comparing by value
	int a2 = 3;
	int b2 = 5;
	int c2 = -1;
	CPPUNIT_ASSERT(list1.Find(&a2, compareFunc) == 0);
	CPPUNIT_ASSERT(list1.Find(&b2, compareFunc) == 1);
	CPPUNIT_ASSERT(list1.Find(&c2, compareFunc) == 2);
}

void LinkedListTest::testFindInvalid()
{
	// Use different addresses to make sure find is comparing by value
	int invalid = 4;
	CPPUNIT_ASSERT(list1.Find(&invalid, compareFunc) < 0);
}

void LinkedListTest::testInsertSorted()
{
	LinkedList<int*> list;
	CPPUNIT_ASSERT(list.InsertSorted(&a, compareFunc));
    CPPUNIT_ASSERT(list.GetCount() == 1);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);

	CPPUNIT_ASSERT(list.InsertSorted(&b, compareFunc));
    CPPUNIT_ASSERT(list.GetCount() == 2);
    CPPUNIT_ASSERT(list.GetAt(0) == &a);
    CPPUNIT_ASSERT(list.GetAt(1) == &b);

	CPPUNIT_ASSERT(list.InsertSorted(&c, compareFunc));
    CPPUNIT_ASSERT(list.GetCount() == 3);
    CPPUNIT_ASSERT(list.GetAt(0) == &c);
    CPPUNIT_ASSERT(list.GetAt(1) == &a);
    CPPUNIT_ASSERT(list.GetAt(2) == &b);
}

void LinkedListTest::metaTestCompareFunc()
{
	int a = 1;
	int b = 3;
	int c = 3;

	CPPUNIT_ASSERT(compareFunc(&a, &b) < 0);
	CPPUNIT_ASSERT(compareFunc(&b, &a) > 0);
	CPPUNIT_ASSERT(compareFunc(&b, &c) == 0);
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
