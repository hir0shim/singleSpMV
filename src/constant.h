#ifdef OPT_SS
    #ifdef COEF_SEGMENT_WIDTH
    #define SEGMENT_WIDTH ALIGNMENT/sizeof(double)*COEF_SEGMENT_WIDTH
    #endif 
    #define PADDING_SIZE ALIGNMENT/sizeof(double)
#endif

#ifdef OPT_CSS
    #define SEGMENT_WIDTH ALIGNMENT/sizeof(double)*2
    #ifdef PADDING
        #define PADDING_SIZE ALIGNMENT/sizeof(double)
    #endif
#endif
