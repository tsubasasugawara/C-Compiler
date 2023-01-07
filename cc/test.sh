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

assert 0 "main() {return 0;}"
assert 42 "main() {return 42;}"
assert 21 "main() {return 5+20-4;}"
assert 41 "main() {return 12  + 34 - 5 ;}"
assert 47 "main() {return 5+6*7;}"
assert 15 "main() {return 5*(9-6);}"
assert 4 "main() {return (3+5)/2;}"
assert 10 "main() {return -10+20;}"
assert 10 "main() {return - -10;}"
assert 10 "main() {return - - +10;}"
assert 0 "main() {return 0==1;}"
assert 1 "main() {return 42==42;}"
assert 1 "main() {return 0!=1;}"
assert 0 "main() {return 42!=42;}"
assert 1 "main() {return 0<1;}"
assert 0 "main() {return 1<1;}"
assert 0 "main() {return 2<1;}"
assert 1 "main() {return 0<=1;}"
assert 1 "main() {return 1<=1;}"
assert 0 "main() {return 2<=1;}"
assert 1 "main() {return 1>0;}"
assert 0 "main() {return 1>1;}"
assert 0 "main() {return 1>2;}"
assert 1 "main() {return 1>=0;}"
assert 1 "main() {return 1>=1;}"
assert 0 "main() {return 1>=2;}"
assert 52 "main() {
a = 52;
return a;
}"
assert 1 "main() {
a = 1;
return a;
}"
assert 14 "main() {
a = 3;
b = 5 * 6 - 8;
return a + b / 2;
}"
assert 6 "main() {
foo = 1;
bar = 2 + 3;
return foo + bar;
}"
assert 5 "main() {return 5;}"
assert 8 "main() {return 8;}"
assert 2 "main() {
if (1) return 2;
}"
assert 10 "main() {
if (0) return 2;
else return 10;
}"
assert 5 "main() {
a = 3;
b = 3;
if(a-b) return 1; else return a + b - 1;
}"
assert 5 "main() {
a = 1;
while (a!=5) a = a + 1;
return a;
}"
assert 13 "main() {
a = 10;
for (b = 0; b < 3; b = b + 1) a = a + 1;
return a;
}"
assert 11 "main() {
a = 10;
for (b = 0; b < 3; b = b + 1) if (a == 11) return a; else a = a + 1;
return a;
}"
assert 13 "main() {
a = 10;
b = 0;
for (; b < 3; b = b + 1) a = a + 1;
return a;
}"
assert 10 "main() {
a = 10;
for (b = 0;; b = b + 1) a = a + 1;
return a;
}"
assert 3 "main() {
for (b = 0; b < 3;) b = b + 1;
return b;
}"
assert 1 "main() {
a = 1;
for (;;) a = a + 1;
a;
}"
assert 1 "main() {
a = 1;
for (;;) return 2;
return 1;
}"
assert 3 "main() {
a = 1;
for(; a < 3;) a = a + 1;
return a;
}"
assert 4 "main() {
if (1) {
    a = 1;
    a = a + 3;
    return a;
} else {
    b = 2;
    return b;
}
return 5;
}"
assert 5 "main() {
b = 1;
for (a = 1; a < 5; a = a + 1) {
    b = b + 1;
}
return b;
}"
assert 3 "main() {
b = 1;
for (a = 1; a < 5; a = a + 1) {
    b = b + 1;
    return 3;
}
return b;
}"
assert 5 "main() {
b = 1;
while(b < 5) {
    b = b + 1;
}
return b;
}"
assert_output OK "main() {
return a();
}"
assert 45 "main() {
return b() + 5;
}"
assert 5 "main() {
return c(2, 3);
}"
assert 5 "
add() {
    return 2 + 2;
}

main() {
    return add() + 1;
}
"

echo OK
