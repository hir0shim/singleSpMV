#ifdef GPU
    #include "opt_cusparse.cpp"
#endif

#if defined(CPU) || defined(MIC)
    #ifdef OPT_CRS
    #include "opt_crs.cpp"
    #endif
    #ifdef OPT_COO
    #include "opt_coo.cpp"
    #endif
    #ifdef OPT_ELL
    #include "opt_ell.cpp"
    #endif
    #ifdef OPT_JDS
    #include "opt_jds.cpp"
    #endif
    #ifdef OPT_MKL
    #include "opt_mkl.cpp"
    #endif
    #ifdef OPT_CUSPARSE
    #include "opt_cusparse.cpp"
    #endif
#endif
