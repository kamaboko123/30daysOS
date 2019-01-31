#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "stdlibc.h"

void test_isdigit(void);
void test_iscapital(void);
void test_upow(void);
void test_atoi(void);

int main(){
    CU_pSuite testSuite;
    
    CU_initialize_registry();
    testSuite = CU_add_suite("StdlibcTest", NULL, NULL);
    
    CU_add_test(testSuite, "test of _isdigit()", test_isdigit);
    CU_add_test(testSuite, "test of _iscapital()", test_iscapital);
    CU_add_test(testSuite, "test of _upow()", test_upow);
    CU_add_test(testSuite, "test of _atoi()", test_atoi);
    
    CU_basic_run_tests();
    CU_cleanup_registry();
    
    return 0;
}

void test_upow(void){
    CU_ASSERT(8 == _upow(2, 3));
    CU_ASSERT(10000 == _upow(10, 4));
    CU_ASSERT(4096 == _upow(16, 3));
}

void test_iscapital(){
    CU_ASSERT(_iscapital('A') == TRUE);
    CU_ASSERT(_iscapital('Z') == TRUE);
    CU_ASSERT(_iscapital('0') == FALSE);
    CU_ASSERT(_iscapital('9') == FALSE);
}

void test_isdigit(){
    CU_ASSERT(_isdigit('A') == FALSE);
    CU_ASSERT(_isdigit('Z') == FALSE);
    CU_ASSERT(_isdigit('0') == TRUE);
    CU_ASSERT(_isdigit('9') == TRUE);
}

void test_atoi(){
    CU_ASSERT(1234567890 == _atoi("1234567890"));
    CU_ASSERT(12345 == _atoi("12345a67890"));
    CU_ASSERT(0 == _atoi("a1234567890"));
}


