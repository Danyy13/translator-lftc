struct myStruct {
    int a;
    double b;
};

// void err3() {
//     return 3;
// }

// int err6() {
//     return;
// }

int err34(int a) { }
int err43noParam() { }

int err35(double err35param) { }

int err36(int err36param) { }

int err37() { }

int main() {
    // 1. "the if condition must be a scalar value"
    // int err1[10];    
    // if(err1) {
    //     return 1;
    // }

    // 2. "the while condition must be a scalar value"
    // int err2[10];    
    // while(err2) {
    //     return 1;
    // }

    // 3. "a void function cannot return a value"
    // err3();

    // 4. "the return value must be a scalar value"
    // int err4[10];
    // return err4;

    // 5. "cannot convert the return expression type to the function return type"
    // struct myStruct err5;
    // return err5;

    // 6. "a non-void function must return a value"
    // err6();

    // 7. "the assign destination must be a left-value"
    // int err7;
    // 3 = err7;

    // 8. "the assign destination cannot be constant"
    // int err9[10];
    // err9 = 1;

    // 9. "the assign destination must be scalar"
    // struct myStruct err9a;
    // int err9b;
    // err9a = err9b;

    // 10. "the assign source must be scalar"
    // int err10[10];
    // int a;
    // a = err10;

    // 11. "the assign source cannot be converted to destination"
    // struct myStruct err11;
    // int a;
    // a = err11;

    // 12. "invalid operand type for ||"
    // struct myStruct err12a;
    // struct myStruct err12b;
    // if(err12a || err12b) { }

    // 13. "invalid operand type for &&"
    // struct myStruct err13a;
    // struct myStruct err13b;
    // if(err13a && err13b) { }

    // 14. "invalid operand type for <"
    // struct myStruct err14a;
    // struct myStruct err14b;
    // if(err14a < err14b) { }

    // 15. "invalid operand type for <="
    // struct myStruct err15a;
    // struct myStruct err15b;
    // if(err15a <= err15b) { }

    // 16. "invalid operand type for <"
    // struct myStruct err16a;
    // struct myStruct err16b;
    // if(err16a > err16b) { }

    // 17. "invalid operand type for <"
    // struct myStruct err17a;
    // struct myStruct err17b;
    // if(err17a >= err17b) { }

    // 18. "invalid operand type for +"
    // struct myStruct err18a;
    // struct myStruct err18b;
    // struct myStruct err18c;
    // err18c = err18a + err18b;

    // 19. "invalid operand type for -"
    // struct myStruct err19a;
    // struct myStruct err19b;
    // struct myStruct err19c;
    // err19c = err19a - err19b;

    // 20. "invalid operand type for *"
    // struct myStruct err20a;
    // struct myStruct err20b;
    // struct myStruct err20c;
    // err20c = err20a * err20b;

    // 21. "invalid operand type for /"
    // struct myStruct err21a;
    // struct myStruct err21b;
    // struct myStruct err21c;
    // err21c = err21a / err21b;

    // 22. "cannot convert to a struct type"
    // int err22a;
    // struct myStruct err22b;
    // err22b = (struct myStruct)err22a;

    // 23. "cannot convert a struct"
    // int err23a;
    // struct myStruct err23b;
    // err23a = (int)err23b;

    // 24. "an array can be converted only to another array"
    // int err24;
    // int err24v[10];
    // err24 = (int)err24v;

    // 25. "a scalar can be converted only to another scalar"
    // int err25;
    // int err25v[10];
    // err25v = (int[])err25;

    // 26. "unary ! must have a scalar operand"
    // int err26[10];
    // if(!err26) { }

    // 27. "unary - must have a scalar operand"
    // int err27[10];
    // err27 = -err27;

    // 28. "only an array can be indexed"
    // int err28;
    // if(err28[0]) { }

    // 29. "the index is not convertible to int"
    // struct myStruct err29i;
    // int err29[10];
    // if(err29[err29i]) { }

    // 30. "a field can only be selected from a struct"
    // int err30;
    // if(err30.value) { }

    // 31. "the structure %s does not have a field %s"
    // struct myStruct err31;
    // if(err31.err31val) { }

    // 32. "undefined id: %s"
    // err32 = 1;

    // 33. "only a function can be called"
    // int err33;
    // err33();

    // 34. "too many arguments in function call"
    // int a;
    // int b;
    // err34(a, b);
    // err43noParam(a);

    // 35. "in call, cannot convert the argument type to the parameter type"
    // int err35param[10];
    // err35(err35param);

    // 36. "too few arguments in function call"
    // err36();

    // 37. "a function can only be called"
    // err37 = 1;

    return 0;
}