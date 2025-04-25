
    #if defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_M_ARM) || defined(__aarch64__) || defined(__ARM64__)
        #if defined(__aarch64__) || defined(__ARM64__)
            #error cmake_ARCH aarch64
        #elif defined(__ARM_ARCH_7A__)
            #error cmake_ARCH armv7l
        #endif
    #elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
        #error cmake_ARCH x86_64
    #endif
    #error cmake_ARCH unknown
    