#include <cppunit/extensions/HelperMacros.h>

#include "../LinkedList.h"

class LinkedListTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(LinkedListTest);

    CPPUNIT_TEST(testEmptyList);
    CPPUNIT_TEST(testAddHeadSingle);
    CPPUNIT_TEST(testAddHeadMultiple);
    CPPUNIT_TEST(testAddTailSingle);
    CPPUNIT_TEST(testAddTailMultiple);
    CPPUNIT_TEST(testGetAtInvalid);
    CPPUNIT_TEST(testInsertAtStart);
    CPPUNIT_TEST(testInsertAtMiddle);
    CPPUNIT_TEST(testInsertAtEnd);
    CPPUNIT_TEST(testInsertAtInvalid);
    CPPUNIT_TEST(testRemoveAtStart);
    CPPUNIT_TEST(testRemoveAtMiddle);
    CPPUNIT_TEST(testRemoveAtEnd);
    CPPUNIT_TEST(testRemoveAtInvalid);
    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST(testFindInvalid);
    CPPUNIT_TEST(testInsertSorted);

    CPPUNIT_TEST(metaTestCompareFunc);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

protected:
    void testEmptyList();
    void testAddHeadSingle();
    void testAddHeadMultiple();
    void testAddTailSingle();
    void testAddTailMultiple();
    void testGetAtInvalid();
    void testInsertAtStart();
    void testInsertAtMiddle();
    void testInsertAtEnd();
    void testInsertAtInvalid();
    void testRemoveAtStart();
    void testRemoveAtMiddle();
    void testRemoveAtEnd();
    void testRemoveAtInvalid();
    void testFind();
    void testFindInvalid();
    void testInsertSorted();

    void metaTestCompareFunc();

    LinkedList list1;
    int a;
    int b;
    int c;
};
