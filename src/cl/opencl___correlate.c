const char opencl___correlate[] = 


#if defined(OPENCL_DBG_MODE)


" /**                                                                      \n"
"  * @brief   OpenCL kernel used for calculation of corelation.            \n"
"  *                                                                       \n"
"  * @file    corelate.c                                                   \n"
"  * @author  OSi (Ondrej Sienczak)                                        \n"
"  */                                                                      \n"
"                                                                          \n"
"                                                                          \n"
" /**                                                                      \n"
"  * @brief   Indexes of arguments                                         \n"
"  */                                                                      \n"
" enum Corelate_ArgIdx                                                     \n"
" {                                                                        \n"
"     CORRELATE___WIDTH_CURRENT,  /**< @brief Current frame width */       \n"
"     CORRELATE___WIDTH_PREVIOUS, /**< @brief Previous frame width */      \n"
"     CORRELATE___FIELD_X,        /**< @brief X coordinate of field */     \n"
"     CORRELATE___FIELD_Y,        /**< @brief Y coordinate of field */     \n"
"     CORRELATE___FIELD_SIZE,     /**< @brief Size of field */             \n"
"     CORRELATE___HEIGHT,         /**< @brief Frame height */              \n"
"     CORRELATE___BPP,            /**< @brief Bytes per pixel */           \n"
"     CORRELATE___OFFSET_X,       /**< @brief X Offset */                  \n"
"     CORRELATE___OFFSET_Y,       /**< @brief Y Offset */                  \n"
"     CORRELATE___MAX_SHIFT,      /**< @brief Maximal correlation shift */ \n"
"     __CORRELATE___ARGS_CNT      /**< @brief Count of arguments */        \n"
" };                                                                       \n"
"                                                                          \n"
"                                                                          \n"
" /**                                                                      \n"
"  * @brief   Correlate frames to find best match                          \n"
"  *                                                                       \n"
"  * @param result    Result of all correlation for every shift            \n"
"  * @param current   Current image buffer                                 \n"
"  * @param previous  Previous (corelated) image buffer                    \n"
"  * @param args      Arguments                                            \n"
"  */                                                                      \n"
" void kernel correlate(global int*                 result,                \n"
"                       global const unsigned char* current,               \n"
"                       global const unsigned char* previous,              \n"
"                       global const int*           args)                  \n"
" {                                                                        \n"
"     unsigned       idx = get_global_id(0);                               \n"
"     unsigned int   sum = 0;                                              \n"
"     result[idx]  = sum;                                                  \n"
" }                                                                        \n"


#else /* defined(OPENCL_DBG_MODE) */


"enum _2_b{_2_1,_2_0,_2_7,_2_8,_2_2,_2_9,_2_a,_2_5,_2_6,_2_3,_2_4};void kerne"
"l correlate(global int*_2_17,global const unsigned char*_2_14,global const u"
"nsigned char*_2_11,global const int*_2_19){unsigned _2_1e=get_global_id(0);u"
"nsigned int _2_1c=0;_2_17[_2_1e]=_2_1c;}"


#endif /* defined(OPENCL_DBG_MODE) */


;


const unsigned opencl___correlate_len = sizeof(opencl___correlate);
