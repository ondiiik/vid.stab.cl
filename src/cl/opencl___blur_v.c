const char opencl___blur_v[] = 


#if defined(OPENCL_DBG_MODE)


" /**                                                                                                                        \n"
"  * @brief   OpenCL kernel used for vertical bluring.                                                                       \n"
"  *                                                                                                                         \n"
"  * @file    blur_h.c                                                                                                       \n"
"  * @author  OSi (Ondrej Sienczak)                                                                                          \n"
"  */                                                                                                                        \n"
"                                                                                                                            \n"
"                                                                                                                            \n"
" /**                                                                                                                        \n"
"  * @brief   Indexes of arguments                                                                                           \n"
"  */                                                                                                                        \n"
" enum BlurV_ArgIdx                                                                                                          \n"
" {                                                                                                                          \n"
"     BLUR_V___WIDTH,          /**< @brief Frame width */                                                                    \n"
"     BLUR_V___HEIGHT,         /**< @brief Frame height */                                                                   \n"
"     BLUR_V___DST_STRIVE,     /**< @brief ? */                                                                              \n"
"     BLUR_V___SRC_STRIVE,     /**< @brief ? */                                                                              \n"
"     BLUR_V___SIZE,           /**< @brief Count of items to be calculated */                                                \n"
"     __BLUR_V___ARGS_CNT      /**< @brief Count of arguments */                                                             \n"
" };                                                                                                                         \n"
"                                                                                                                            \n"
"                                                                                                                            \n"
" /**                                                                                                                        \n"
"  * @brief   Vertical blur operation                                                                                        \n"
"  *                                                                                                                         \n"
"  * @param   dst     Destination buffer                                                                                     \n"
"  * @param   src     Source buffer                                                                                          \n"
"  * @param   args    Arguments of filter                                                                                    \n"
"  */                                                                                                                        \n"
" void kernel blurV(global char*       dst,                                                                                  \n"
"                   global const char* src,                                                                                  \n"
"                   global const int*  args)                                                                                 \n"
" {                                                                                                                          \n"
"     const int                   width       = args[BLUR_V___WIDTH];                                                        \n"
"     const int                   height      = args[BLUR_V___HEIGHT];                                                       \n"
"     const int                   dst_strive  = args[BLUR_V___DST_STRIVE];                                                   \n"
"     const int                   src_strive  = args[BLUR_V___SRC_STRIVE];                                                   \n"
"     const int                   size        = args[BLUR_V___SIZE];                                                         \n"
"                                                                                                                            \n"
"     const int                   size2       = size / 2;                /* Size of one side of the kernel without center */ \n"
"                                                                                                                            \n"
"     const int                   x           = get_global_id(0);        /* Current thread ID is current row */              \n"
"     global const unsigned char* start       = src + x;                 /* Start and end of kernel */                       \n"
"     global const unsigned char* end         = start;                                                                       \n"
"     global unsigned char*       current     = dst + x;                 /* Current destination pixel */                     \n"
"     int                         acc         = (*start) * (size2 + 1);  /* Left half of kernel with first pixel */          \n"
"                                                                                                                            \n"
"     /* Right half of kernel */                                                                                             \n"
"     for (int k = 0; k < size2; k++)                                                                                        \n"
"     {                                                                                                                      \n"
"         acc += (*end);                                                                                                     \n"
"         end += src_strive;                                                                                                 \n"
"     }                                                                                                                      \n"
"                                                                                                                            \n"
"     /* Go through the image */                                                                                             \n"
"     for (int y = 0; y < height; y++)                                                                                       \n"
"     {                                                                                                                      \n"
"         acc = acc - (*start) + (*end);                                                                                     \n"
"                                                                                                                            \n"
"         if (y > size2)                                                                                                     \n"
"         {                                                                                                                  \n"
"             start += src_strive;                                                                                           \n"
"         }                                                                                                                  \n"
"                                                                                                                            \n"
"         if (y < height - size2 - 1)                                                                                        \n"
"         {                                                                                                                  \n"
"             end += src_strive;                                                                                             \n"
"         }                                                                                                                  \n"
"                                                                                                                            \n"
"         *current = acc / size;                                                                                             \n"
"         current += dst_strive;                                                                                             \n"
"     }                                                                                                                      \n"
" }                                                                                                                          \n"


#else /* defined(OPENCL_DBG_MODE) */


"enum BlurV_ArgIdx{_1_5,_1_3,_1_0,_1_1,_1_4,_1_2};void kernel blurV(global ch"
"ar*_1_f,global const char*_1_11,global const int*_1_e){const int _1_b=_1_e[_"
"1_5];const int _1_9=_1_e[_1_3];const int _1_6=_1_e[_1_0];const int _1_7=_1_e"
"[_1_1];const int _1_d=_1_e[_1_4];const int _1_c=_1_d/2;const int x=get_globa"
"l_id(0);global const unsigned char*_1_a=_1_11+x;global const unsigned char*_"
"1_10=_1_a;global unsigned char*_1_8=_1_f+x;int _1_12=(*_1_a)*(_1_c+1);for(in"
"t k=0;k<_1_c;k++){_1_12+=(*_1_10);_1_10+=_1_7;}for(int y=0;y<_1_9;y++){_1_12"
"=_1_12-(*_1_a)+(*_1_10);if(y>_1_c){_1_a+=_1_7;}if(y<_1_9-_1_c-1){_1_10+=_1_7"
";}*_1_8=_1_12/_1_d;_1_8+=_1_6;}}"


#endif /* defined(OPENCL_DBG_MODE) */


;


const unsigned opencl___blur_v_len = sizeof(opencl___blur_v);
