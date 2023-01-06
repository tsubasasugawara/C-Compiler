#!/bin/sh
assert() {
    expected="$1"
    input="$2"

    ./cc "$input" > tmp.s
    cc -o tmp tmp.s ./test/foo.c
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert_output() {
    expected="$1"
    input="$2"

    ./cc "$input" > tmp.s
    cc -o tmp tmp.s ./test/foo.c
    ./tmp
    actual=$(./tmp)

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12  + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 10 "- -10;"
assert 10 "- - +10;"
assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"
assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"
assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"
assert 52 "a = 52;a;"
assert 1 "a = 1;
a;
"
assert 14 "a = 3;
b = 5 * 6 - 8;
a + b / 2;"
assert 6 "foo = 1;
bar = 2 + 3;
foo + bar;"
assert 5 "return 5;"
assert 8 "return 8;"
assert 14 "a = 3;
b = 5 * 6 - 8;
return a + b / 2;"
assert 2 "if (1) 2;"
assert 10 "if (0) 2; else 10;"
assert 5 "a = 3;
b = 3;
if(a-b) 1; else a + b - 1;"
assert 5 "a = 1;
while (a!=5) a = a + 1;
a;"
assert 13 "a = 10; for (b = 0; b < 3; b = b + 1) a = a + 1; a;"
assert 11 "a = 10; for (b = 0; b < 3; b = b + 1) if (a == 11) return a; else a = a + 1; a;"
assert 13 "a = 10; b = 0; for (; b < 3; b = b + 1) a = a + 1; a;"
assert 10 "a = 10; for (b = 0;; b = b + 1) a = a + 1; a;"
assert 3 "for (b = 0; b < 3;) b = b + 1; b;"
assert 1 "a = 1; for (;;) a = a + 1; a;"
assert 1 "a = 1; for (;;) return 2; return 1;"
assert 3 "{a = 1; for(; a < 3;) a = a + 1; return a;}"
assert 4 "if (1) {
    a = 1;
    a = a + 3;
    return a;
} else {
    b = 2;
    return b;
}
return 5;"
assert 5 "
b = 1;
for (a = 1; a < 5; a = a + 1) {
    b = b + 1;
}
return b;
"
assert 3 "
b = 1;
for (a = 1; a < 5; a = a + 1) {
    b = b + 1;
    return 3;
}
return b;
"
assert 5 "
b = 1;
while(b < 5) {
    b = b + 1;
}
return b;
"
assert_output OK "a();"
assert 5 "b(); return 5;"

echo OK
