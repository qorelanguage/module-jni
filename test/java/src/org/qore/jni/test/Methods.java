package org.qore.jni.test;

class A {
    int f() { return 1; }
}

class B {
    int f() { return 2; }
}

class C {
    int f() { return 3; }
}

public class Methods {
    static C instance = new C();
}
