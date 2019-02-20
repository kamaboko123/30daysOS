#include <string.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "stdlibc.h"

void test_sprintf(void);
void test_to_dec_asc(void);
void test_to_hex_asc(void);
void test_ndigit(void);
void test_upow(void);
void test_iscapital(void);
void test_atoi(void);
void test_isdigit(void);
void test_memcpy(void);
void test_memset(void);

int main(){
    CU_pSuite testSuite;
    
    CU_initialize_registry();
    testSuite = CU_add_suite("StdlibcTest", NULL, NULL);
    
    CU_add_test(testSuite, "test of _sprintf()", test_sprintf);
    CU_add_test(testSuite, "test of _to_dec_asc()", test_to_dec_asc);
    CU_add_test(testSuite, "test of _to_dec_hex_asc()", test_to_hex_asc);
    CU_add_test(testSuite, "test of _ndigit()", test_ndigit);
    CU_add_test(testSuite, "test of _upow()", test_upow);
    CU_add_test(testSuite, "test of _iscapital()", test_iscapital);
    CU_add_test(testSuite, "test of _atoi()", test_atoi);
    CU_add_test(testSuite, "test of _isdigit()", test_isdigit);
    CU_add_test(testSuite, "test of _memcpy()", test_memcpy);
    CU_add_test(testSuite, "test of _memset()", test_memset);
    
    CU_basic_run_tests();
    CU_cleanup_registry();
    
    return 0;
}

void test_sprintf(){
    char buf[64];
    memset(buf, '\0', sizeof(buf));
    
    _sprintf(buf, "abc : %d,%3d,%03d,%x,%2x,%02x,%X,%2X,%02X  |  %d,%4d,%04d", 10, 10, 10, 10, 10, 10, 10, 10, 10, -10, -10, -10);
    CU_ASSERT_STRING_EQUAL(buf, "abc : 10, 10,010,a, a,0a,A, A,0A  |  -10, -10,-010");
}

void test_to_dec_asc(){
    char buf[16];
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_dec_asc(buf, 10) == 2);
    CU_ASSERT_STRING_EQUAL(buf, "10");
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_dec_asc(buf, 123456789) == 9);
    CU_ASSERT_STRING_EQUAL(buf, "123456789");
}

void test_to_hex_asc(){
    char buf[16];
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_hex_asc(buf, 0x20, FALSE) == 2);
    CU_ASSERT_STRING_EQUAL(buf, "20");
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_hex_asc(buf, 0xffff, FALSE) == 4);
    CU_ASSERT_STRING_EQUAL(buf, "ffff");
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_hex_asc(buf, 0x20, TRUE) == 2);
    CU_ASSERT_STRING_EQUAL(buf, "20");
    
    memset(buf, 0, sizeof(buf));
    CU_ASSERT(_to_hex_asc(buf, 0xffff, TRUE) == 4);
    CU_ASSERT_STRING_EQUAL(buf, "FFFF");
}

void test_ndigit(){
    CU_ASSERT(_ndigit(100, 10) == 3);
    CU_ASSERT(_ndigit(100, 16) == 2);
    CU_ASSERT(_ndigit(100, 2) == 7);
}

void test_upow(){
    CU_ASSERT(_upow(2, 3) == 8);
    CU_ASSERT(_upow(10, 4) == 10000);
    CU_ASSERT(_upow(16, 3) == 4096);
}

void test_iscapital(){
    CU_ASSERT(_iscapital('A') == TRUE);
    CU_ASSERT(_iscapital('Z') == TRUE);
    CU_ASSERT(_iscapital('0') == FALSE);
    CU_ASSERT(_iscapital('9') == FALSE);
}

void test_atoi(){
    CU_ASSERT(1234567890 == _atoi("1234567890"));
    CU_ASSERT(12345 == _atoi("12345a67890"));
    CU_ASSERT(0 == _atoi("a1234567890"));
}
void test_isdigit(){
    CU_ASSERT(_isdigit('A') == FALSE);
    CU_ASSERT(_isdigit('Z') == FALSE);
    CU_ASSERT(_isdigit('0') == TRUE);
    CU_ASSERT(_isdigit('9') == TRUE);
}

void test_memcpy(){
    char buf1[8] = "abc\0def";
    char buf2[8] = "zzzzzzz";
    buf1[7] = 'g';
    
    CU_ASSERT_NSTRING_EQUAL(buf1, "abc\0defg", sizeof(buf1));
    CU_ASSERT_NSTRING_EQUAL(buf2, "zzzzzzz\0", sizeof(buf2));
    
    CU_ASSERT(_memcpy(buf2, buf1, sizeof(buf1)) == buf2);
    CU_ASSERT_NSTRING_EQUAL(buf2, buf1, sizeof(buf1));
}

void test_memset(){
    char buf[8];
    
    for(int i = 0; i < sizeof(buf); i++){
        if(i % 2 == 0){
            buf[i] = 'A';
            continue;
        }
        buf[i] = '\0';
    }
    CU_ASSERT_NSTRING_EQUAL(buf, "A\0A\0A\0A\0", sizeof(buf));
    
    CU_ASSERT(_memset(buf, 'B', sizeof(buf)) == sizeof(buf));
    
    CU_ASSERT_NSTRING_EQUAL(buf, "BBBBBBBB", sizeof(buf));
}
