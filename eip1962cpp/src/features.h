#ifndef H_ELEMENT
#define H_ELEMENT

bool in_fuzzing() {
    #ifdef FUZZING
    return true;
    #else
    return false;
    #endif
}

#endif