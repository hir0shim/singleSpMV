#ifdef INDEX_64
typedef int64_t idx_t;
#elif INDEX_32
typedef int idx_t;
#else
typedef int idx_t;
#endif

#ifndef SEGMENT_WIDTH
    #define SEGMENT_WIDTH ALIGNMENT/sizeof(double)
#endif 
#ifdef PADDING 
    #ifndef PADDING_SIZE
        #define PADDING_SIZE ALIGNMENT/sizeof(double)
    #endif
#endif

#ifndef N_BLOCK
    #define N_BLOCK 1
#endif

