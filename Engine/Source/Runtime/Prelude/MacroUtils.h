#pragma once

//references: https://github.com/kobalicek/ppmagic/blob/master/ppmagic.h

#define OMNI_INDIRECT(X) X //see http://jhnet.co.uk/articles/cpp_magic, a call will force argument to be evaluated, which can counter the effect of "expanded rule painted blue"
#define OMNI_CONCAT(A, B) A##B


#define OMNI_1(_1, ...) _1
#define OMNI_2(_1, _2, ...) _2
#define OMNI_3(_1, _2, _3, ...) _3
#define OMNI_4(_1, _2, _3, _4, ...) _4
#define OMNI_5(_1, _2, _3, _4, _5, ...) _5
#define OMNI_6(_1, _2, _3, _4, _5, _6, ...) _6
#define OMNI_7(_1, _2, _3, _4, _5, _6, _7, ...) _7
#define OMNI_8(_1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define OMNI_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9

#define OMNI_DUMMY_VA(...) dummy,##__VA_ARGS__
#define OMNI_IS(...) OMNI_INDIRECT(OMNI_2(__VA_ARGS__, 0))
#define OMNI_PROBE() ~, 1
#define OMNI_NOT(x) OMNI_IS(OMNI_CONCAT(OMNI_NOT_, x)) //for x == 0, OMNI_NOT_0 will be a special case which expand to 2 other, for others, 0
#define OMNI_NOT_0 OMNI_PROBE()
#define OMNI_BOOL(x) OMNI_NOT(OMNI_NOT(x))

#define OMNI_IF(exp) OMNI_IF_INTERNAL(OMNI_BOOL(exp))
#define OMNI_IF_ELSE(exp) OMNI_IF_ELSE_INTERNAL(OMNI_BOOL(exp))

#define OMNI_IF_INTERNAL(exp) OMNI_CONCAT(OMNI_IF_INTERNAL_, exp)
#define OMNI_IF_INTERNAL_0(...)
#define OMNI_IF_INTERNAL_1(...) __VA_ARGS__

#define OMNI_IF_ELSE_INTERNAL(exp) OMNI_CONCAT(OMNI_IF_ELSE_INTERNAL_, exp)
#define OMNI_IF_ELSE_INTERNAL_0(...) OMNI_IF_ELSE_INTERNAL_0_ELSE
#define OMNI_IF_ELSE_INTERNAL_0_ELSE(...)  __VA_ARGS__
#define OMNI_IF_ELSE_INTERNAL_1(...) __VA_ARGS__ OMNI_IF_ELSE_INTERNAL_1_ELSE
#define OMNI_IF_ELSE_INTERNAL_1_ELSE(...)

#define OMNI_COUNT_VA(...) OMNI_INDIRECT(OMNI_COUNT_INTERNAL0(OMNI_DUMMY_VA(__VA_ARGS__), 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
#define OMNI_COUNT_INTERNAL0(...) OMNI_INDIRECT(OMNI_COUNT_INTERNAL1(__VA_ARGS__))
#define OMNI_COUNT_INTERNAL1(_dummy,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,x, ...) x

#define OMNI_HAS_VA(...) OMNI_INDIRECT(OMNI_BOOL(OMNI_COUNT_VA(__VA_ARGS__)))

#define OMNI_COMMA_VA(...) OMNI_IF(OMNI_HAS_VA(__VA_ARGS__))(, __VA_ARGS__)

#define OMNI_STRINGIFY(...) OMNI_STRINGIFY_INTERNAL(__VA_ARGS__)
#define OMNI_STRINGIFY_INTERNAL(...) OMNI_INDIRECT(#__VA_ARGS__)

#define OMNI_EVAL(...)      OMNI_EVAL1024(OMNI_EVAL1024(__VA_ARGS__))
#define OMNI_EVAL1024(...)  OMNI_EVAL512(OMNI_EVAL512(__VA_ARGS__))
#define OMNI_EVAL512(...)   OMNI_EVAL256(OMNI_EVAL256(__VA_ARGS__))
#define OMNI_EVAL256(...)   OMNI_EVAL128(OMNI_EVAL128(__VA_ARGS__))
#define OMNI_EVAL128(...)   OMNI_EVAL64(OMNI_EVAL64(__VA_ARGS__))
#define OMNI_EVAL64(...)    OMNI_EVAL32(OMNI_EVAL32(__VA_ARGS__))
#define OMNI_EVAL32(...)    OMNI_EVAL16(OMNI_EVAL16(__VA_ARGS__))
#define OMNI_EVAL16(...)    OMNI_EVAL8(OMNI_EVAL8(__VA_ARGS__))
#define OMNI_EVAL8(...)     OMNI_EVAL4(OMNI_EVAL4(__VA_ARGS__))
#define OMNI_EVAL4(...)     OMNI_EVAL2(OMNI_EVAL2(__VA_ARGS__))
#define OMNI_EVAL2(...)     OMNI_EVAL1(OMNI_EVAL1(__VA_ARGS__))
#define OMNI_EVAL1(...) __VA_ARGS__

#define OMNI_EMPTY()
#define OMNI_DEFER(m) m OMNI_EMPTY()
#define OMNI_DEFER2(m) m OMNI_EMPTY OMNI_EMPTY()()
#define OMNI_MAP(m, first, ...)\
    m(first)\
    OMNI_IF(OMNI_HAS_VA(__VA_ARGS__))(\
        OMNI_DEFER2(_OMNI_MAP)()(m, __VA_ARGS__))

#define _OMNI_MAP() OMNI_MAP

#define OMNI_CONFIG_LIST(condition, value, ...)\
    OMNI_IF_ELSE(condition)(value)(\
        OMNI_IF(OMNI_HAS_VA(__VA_ARGS__))(\
            OMNI_DEFER2(_OMNI_CONFIG_LIST)()(__VA_ARGS__)\
    ))

#define _OMNI_CONFIG_LIST() OMNI_CONFIG_LIST

