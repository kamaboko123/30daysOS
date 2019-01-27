#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "stdlibc.h"

void test_isdigit(void);

int main(){
    CU_pSuite testSuite;
    
    CU_initialize_registry();
    testSuite = CU_add_suite("StdlibcTest", NULL, NULL);
    
    CU_add_test(testSuite, "test of isdigit()", test_isdigit);
    
    CU_basic_run_tests();
    CU_cleanup_registry();
    
    return 0;
}

void test_isdigit(void){
    CU_ASSERT(_isdigit('A') == FALSE);
    CU_ASSERT(_isdigit('Z') == FALSE);
    CU_ASSERT(_isdigit('0') == TRUE);
    CU_ASSERT(_isdigit('9') == TRUE);
}
