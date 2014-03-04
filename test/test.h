#include <cppunit/extensions/HelperMacros.h>

class LinkedListTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(LinkedListTest);

    CPPUNIT_TEST(testEmptyList);
    CPPUNIT_TEST(testAddHeadSingle);
    CPPUNIT_TEST(testAddHeadMultiple);
    CPPUNIT_TEST(testAddTailSingle);
    CPPUNIT_TEST(testAddTailMultiple);
    CPPUNIT_TEST(testGetAtInvalid);

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
};
