

#ifndef CHARITERTEST_H
#define CHARITERTEST_H

#include "intltest.h"
#include "unicode/uiter.h"

class CharIterTest: public IntlTest {
public:
    CharIterTest();
    
    void runIndexedTest( int32_t index, UBool exec, const char* &name, char* par = NULL );

    /**
     * Test Constructors and operators ==, != and a few other methods
     **/
    void TestConstructionAndEquality(void);
    /**
     * Test Constructors and operators ==, != and a few other methods for UChariter
     **/
    void TestConstructionAndEqualityUChariter(void);
    /**
     * test the iteration functionality in different ways
     **/
    void TestIteration(void);
     /**
     * test the iteration functionality in different ways with  unicodestring of UChar32's
     **/
    void TestIterationUChar32(void);

    void TestUCharIterator();
    void TestUCharIterator(UCharIterator *iter, CharacterIterator &ci, const char *moves, const char *which);
    void TestCoverage();
    void TestCharIteratorSubClasses();
};

#endif

