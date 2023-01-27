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

assert 0 "
int main() {
    return 0;
}"
assert 42 "
int main() {
    return 42;
}"
assert 21 "
int main() {
    return 5+20-4;
}"
assert 41 "
int main() {
    return 12  + 34 - 5 ;
}"
assert 47 "
int main() {
    return 5+6*7;
}"
assert 15 "
int main() {
    return 5*(9-6);
}"
assert 4 "
int main() {
    return (3+5)/2;
}"
assert 10 "
int main() {
    return -10+20;
}"
assert 10 "
int main() {
    return 10;
}"
assert 0 "
int main() {
    return 0==1;
}"
assert 1 "
int main() {
    return 42==42;
}"
assert 1 "
int main() {
    return 0!=1;
}"
assert 0 "
int main() {
    return 42!=42;
}"
assert 1 "
int main() {
    return 0<1;
}"
assert 0 "
int main() {
    return 1<1;
}"
assert 0 "
int main() {
    return 2<1;
}"
assert 1 "
int main() {
    return 0<=1;
}"
assert 1 "
int main() {
    return 1<=1;
}"
assert 0 "
int main() {
    return 2<=1;
}"
assert 1 "
int main() {
    return 1>0;
}"
assert 0 "
int main() {
    return 1>1;
}"
assert 0 "
int main() {
    return 1>2;
}"
assert 1 "
int main() {
    return 1>=0;
}"
assert 1 "
int main() {
    return 1>=1;
}"
assert 0 "
int main() {
    return 1>=2;
}"
assert 52 "
int main() {
    int a = 52;
    return a;
}"
assert 1 "
int main() {
    int a = 1;
    return a;
}"
assert 14 "
int main() {
    int a = 3;
    int b = 5 * 6 - 8;
    return a + b / 2;
}"
assert 6 "
int main() {
    int foo = 1;
    int bar = 2 + 3;
    return foo + bar;
}"
assert 5 "
int main() {
    return 5;
}"
assert 8 "
int main() {
    return 8;
}"
assert 2 "
int main() {
    if (1) return 2;
}"
assert 10 "
int main() {
    if (0) return 2;
    else return 10;
}"
assert 5 "
int main() {
    int a = 3;
    int b = 3;
    if(a-b) return 1; else return a + b - 1;
}"
assert 5 "
int main() {
    int a = 1;
    while (a!=5) a = a + 1;
    return a;
}"
assert 13 "
int main() {
    int a = 10;
    for (int b = 0; b < 3; b = b + 1) a = a + 1;
    return a;
}"
assert 11 "
int main() {
    int a = 10;
    for (int b = 0; b < 3; b = b + 1) if (a == 11) return a; else a = a + 1;
    return a;
}"
assert 13 "
int main() {
    int a = 10;
    int b = 0;
    for (; b < 3; b = b + 1) a = a + 1;
    return a;
}"
assert 10 "
int main() {
    int a = 10;
    for (int b = 0;; b = b + 1) a = a + 1;
    return a;
}"
assert 3 "
int main() {
    for (int b = 0; b < 3;) b = b + 1;
    return b;
}"
assert 1 "
int main() {
    int a = 1;
    for (;;) a = a + 1;
    return a;
}"
assert 1 "
int main() {
    int a = 1;
    for (;;) return 2;
    return 1;
}"
assert 3 "
int main() {
    int a = 1;
    for(; a < 3;) a = a + 1;
    return a;
}"
assert 4 "
int main() {
    if (1) {
        int a = 1;
        a = a + 3;
        return a;
    } else {
        int b = 2;
        return b;
    }
    return 5;
}"
assert 5 "
int main() {
    int b = 1;
    for (int a = 1; a < 5; a = a + 1) {
        b = b + 1;
    }
    return b;
}"
assert 3 "
int main() {
    int b = 1;
    for (int a = 1; a < 5; a = a + 1) {
        b = b + 1;
        return 3;
    }
    return b;
}"
assert 5 "
int main() {
    int b = 1;
    while(b < 5) {
        b = b + 1;
    }
    return b;
}"
assert_output OK "
int main() {
    return a();
}"
assert 45 "
int main() {
    return b() + 5;
}"
assert 5 "
int main() {
    return c(2, 3);
}"
assert 5 "
int add() {
    return 2 + 2;
}

int main() {
    return add() + 1;
}
"
assert 5 "
int add(int x, int y) {
    return x + y;}

int main() {
    return add(2, 3);}
"
assert 10 "
int add(int x, int y) {
    int a = 5;
    return a + x + y;
}

int main() {
    return add(2, 3);}
"
assert 10 "
int add(int x, int y) {
    int a = c(x, y);
    return a + x + y;
}

int main() {
    return add(2, 3);}
"
assert 55 "
int fibonacci(int x) {
    if (x < 2) return x;
    return fibonacci(x - 1) + fibonacci(x - 2);
}
int main() { return fibonacci(10); }"
assert 3 "
int main() {
    int x = 3;
    int y = &x;
    return *y;
}"
assert 3 "
int main() {
    int x = 3;
    int y = 5;
    int z = &y + 8;
    return *z;
}"
assert 3 "
int main() {
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}"
assert 6 "
int main() {
    int *p = alloc(6, 3);
    return *p;
}"
assert 3 "
int main() {
    int *p = alloc(6, 3);
    return *(p + 1);
}"
assert 4 "
int main() {
    int x;
    return sizeof(x);
}"
assert 8 "
int main() {
    int *x;
    return sizeof(x);
}"
assert 4 "
int main() {
    int x;
    return sizeof(x + 3);
}"
assert 8 "
int main() {
    int *x;
    return sizeof(x + 3);
}"
assert 4 "
int main() {
    int *x;
    return sizeof(3);
}"
assert 4 "
int main() {
    return sizeof(sizeof(1));
}"
assert 0 "
int main() {
    int a[10];
    return 0;
}"
assert 2 "
int main() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *(p + 1);
}"
assert 1 "
int main() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p;
}"
assert 3 "
int main() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}"
assert 6 "
int main() {
    int a[2];
    a[0] = 1;
    a[0 + 1] = 5;
    return a[0] + a[1];
}"
assert 0 "
int *foo;
int bar[10];
int *bazz() {}
int foobar() {}
int main() { return 0; }
"
assert 6 "
int foo;
int bar() {
    foo = 14;
    return foo;
}
int main() {
    int foo = 6;
    return foo;
}
"
assert 14 "
int foo;
int bar() {
    foo = 14;
    return foo;
}
int main() {
    int foo = 6;
    return bar();
}
"
assert 13 "
int bar[10];
int main() {
    bar[1] = 13;
    return bar[1];
}
"
assert 31 "
int foo;
int foobar() {
    return 11;
}
int main() {
    foo = 20;
    return foobar() + foo;
}
"
assert 3 "
int main(){
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}
"
assert 6 "
int main(){
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[1] + y;
}
"
assert 13 '
int main() {
    char *a = "Hello";
    printf(a);
    return 13;
}
'
assert_output Hello '
int main() {
    char *a = "Hello";
    return printf(a);
}'

echo OK
