const char opencl___corelate[] = 


#if defined(OPENCL_DBG_MODE)


" /**                                                                \n"
"  * @brief   OpenCL kernel used for calculation of corelation.      \n"
"  *                                                                 \n"
"  * @file    corelate.c                                             \n"
"  * @author  OSi (Ondrej Sienczak)                                  \n"
"  */                                                                \n"
"                                                                    \n"
"                                                                    \n"
" /**                                                                \n"
"  * @brief   Indexes of arguments                                   \n"
"  */                                                                \n"
" enum Corelate_ArgIdx                                               \n"
" {                                                                  \n"
"     CORELATE___WIDTH_CURRENT,  /**< @brief Current frame width */  \n"
"     CORELATE___WIDTH_PREVIOUS, /**< @brief Previous frame width */ \n"
"     CORELATE___HEIGHT,         /**< @brief Frame height */         \n"
"     CORELATE___BPP,            /**< @brief Bytes per pixel */      \n"
"     CORELATE___OFFSET_X,       /**< @brief X Offset */             \n"
"     CORELATE___OFFSET_Y,       /**< @brief Y Offset */             \n"
"     CORELATE___TRESHOLD,       /**< @brief Treshold */             \n"
"     __CORELATE___ARGS_CNT      /**< @brief Count of arguments */   \n"
" };                                                                 \n"
"                                                                    \n"
"                                                                    \n"
" /**                                                                \n"
"  * @brief   Corelated field                                        \n"
"  */                                                                \n"
" struct Corelate_Field                                              \n"
" {                                                                  \n"
"     int x;                                                         \n"
"     int y;                                                         \n"
"     int size;                                                      \n"
" };                                                                 \n"
"                                                                    \n"
"                                                                    \n"
" /**                                                                \n"
"  * @brief   Vertical blur operation                                \n"
"  *                                                                 \n"
"  * @param   dst     Destination buffer                             \n"
"  * @param   src     Source buffer                                  \n"
"  * @param   args    Arguments of filter                            \n"
"  */                                                                \n"
" void kernel corelate(global int*                   aResult,        \n"
"                      global const char*            aCurrent,       \n"
"                      global const char*            aPrevious,      \n"
"                      global struct Corelate_Field* aFields,        \n"
"                      global const int*             aArgs)          \n"
" {                                                                  \n"
"                                                                    \n"
" }                                                                  \n"


#else /* defined(OPENCL_DBG_MODE) */


"enum Corelate_ArgIdx{CORELATE___WIDTH_CURRENT,CORELATE___WIDTH_PREVIOUS,CORE"
"LATE___HEIGHT,CORELATE___BPP,CORELATE___OFFSET_X,CORELATE___OFFSET_Y,CORELAT"
"E___TRESHOLD,__CORELATE___ARGS_CNT};struct Corelate_Field{int x;int y;int _2"
"_d;};void kernel corelate(global int*aResult,global const char*aCurrent,glob"
"al const char*aPrevious,global struct Corelate_Field*aFields,global const in"
"t*aArgs){}"


#endif /* defined(OPENCL_DBG_MODE) */


;


const unsigned opencl___corelate_len = sizeof(opencl___corelate);
