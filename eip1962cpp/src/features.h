#ifndef H_FEATURES
#define H_FEATURES

bool in_fuzzing() {
    #ifdef FUZZING
    return true;
    #else
    return false;
    #endif
}

#endif