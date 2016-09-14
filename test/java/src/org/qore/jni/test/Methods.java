package org.qore.jni.test;

class A {
    int f() { return 1; }
}

class B extends A {
    int f() { return 2; }
}

class C extends B {
    int f() { return 3; }
    int m() { return 4; }
}

public class Methods {
    static C c = new C();
    static B b = new B();
}
