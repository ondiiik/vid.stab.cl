const char opencl___blur_h[] = 


#if defined(OPENCL_DBG_MODE)


" /**                                                                                                               \n"
"  * @brief   OpenCL kernel used for horizontal bluring.                                                            \n"
"  *                                                                                                                \n"
"  * @file    blur_h.c                                                                                              \n"
"  * @author  OSi (Ondrej Sienczak)                                                                                 \n"
"  */                                                                                                               \n"
"                                                                                                                   \n"
"                                                                                                                   \n"
" /**                                                                                                               \n"
"  * @brief   Indexes of arguments                                                                                  \n"
"  */                                                                                                               \n"
" enum BlurH_ArgIdx                                                                                                 \n"
" {                                                                                                                 \n"
"     BLUR_H___WIDTH,          /**< @brief Frame width */                                                           \n"
"     BLUR_H___HEIGHT,         /**< @brief Frame height */                                                          \n"
"     BLUR_H___DST_STRIVE,     /**< @brief ? */                                                                     \n"
"     BLUR_H___SRC_STRIVE,     /**< @brief ? */                                                                     \n"
"     BLUR_H___SIZE,           /**< @brief Count of items to be calculated */                                       \n"
"     __BLUR_H___ARGS_CNT      /**< @brief Count of arguments */                                                    \n"
" };                                                                                                                \n"
"                                                                                                                   \n"
"                                                                                                                   \n"
" /**                                                                                                               \n"
"  * @brief   Horizontal blur operation                                                                             \n"
"  *                                                                                                                \n"
"  * @param   dst     Destination buffer                                                                            \n"
"  * @param   src     Source buffer                                                                                 \n"
"  * @param   args    Arguments of filter                                                                           \n"
"  */                                                                                                               \n"
" void kernel blurH(global char*       dst,                                                                         \n"
"                   global const char* src,                                                                         \n"
"                   global const int*  args)                                                                        \n"
" {                                                                                                                 \n"
"     const int                   width       = args[BLUR_H___WIDTH];                                               \n"
"     const int                   height      = args[BLUR_H___HEIGHT];                                              \n"
"     const int                   dst_strive  = args[BLUR_H___DST_STRIVE];                                          \n"
"     const int                   src_strive  = args[BLUR_H___SRC_STRIVE];                                          \n"
"     const int                   size        = args[BLUR_H___SIZE];                                                \n"
"                                                                                                                   \n"
"     const int                   size2       = size / 2; /* Size of one side of the kernel without center */       \n"
"                                                                                                                   \n"
"     const int                   y           = get_global_id(0);        /* Current thread ID is current row */     \n"
"     global const unsigned char* start       = src  + y * src_strive;   /* Start and end of kernel */              \n"
"     global const unsigned char* end         = start;                                                              \n"
"     global unsigned char*       current     = dst  + y * dst_strive;   /* Current destination pixel */            \n"
"     unsigned int                acc         = (*start) * (size2 + 1);  /* Left half of kernel with first pixel */ \n"
"                                                                                                                   \n"
"     /* Right half of kernel */                                                                                    \n"
"     for (int k = 0; k < size2; ++k)                                                                               \n"
"     {                                                                                                             \n"
"         acc += (*end);                                                                                            \n"
"         end++;                                                                                                    \n"
"     }                                                                                                             \n"
"                                                                                                                   \n"
"     /* Go through the image */                                                                                    \n"
"     for (int x = 0; x < width; ++x)                                                                               \n"
"     {                                                                                                             \n"
"         acc = acc + (*end) - (*start);                                                                            \n"
"                                                                                                                   \n"
"         if (x > size2)                                                                                            \n"
"         {                                                                                                         \n"
"             start++;                                                                                              \n"
"         }                                                                                                         \n"
"                                                                                                                   \n"
"         if (x < (width - size2 - 1))                                                                              \n"
"         {                                                                                                         \n"
"             end++;                                                                                                \n"
"         }                                                                                                         \n"
"                                                                                                                   \n"
"         *current = acc / size;                                                                                    \n"
"         ++current;                                                                                                \n"
"     }                                                                                                             \n"
" }                                                                                                                 \n"


#else /* defined(OPENCL_DBG_MODE) */


"enum BlurH_ArgIdx{_0_3,_0_4,_0_0,_0_1,_0_5,_0_2};void kernel blurH(global ch"
"ar*_0_f,global const char*_0_11,global const int*_0_e){const int _0_b=_0_e[_"
"0_3];const int _0_9=_0_e[_0_4];const int _0_6=_0_e[_0_0];const int _0_7=_0_e"
"[_0_1];const int _0_d=_0_e[_0_5];const int _0_c=_0_d/2;const int y=get_globa"
"l_id(0);global const unsigned char*_0_a=_0_11+y*_0_7;global const unsigned c"
"har*_0_10=_0_a;global unsigned char*_0_8=_0_f+y*_0_6;unsigned int _0_12=(*_0"
"_a)*(_0_c+1);for(int k=0;k<_0_c;++k){_0_12+=(*_0_10);_0_10++;}for(int x=0;x<"
"_0_b;++x){_0_12=_0_12+(*_0_10)-(*_0_a);if(x>_0_c){_0_a++;}if(x<(_0_b-_0_c-1)"
"){_0_10++;}*_0_8=_0_12/_0_d;++_0_8;}}"


#endif /* defined(OPENCL_DBG_MODE) */


;


const unsigned opencl___blur_h_len = sizeof(opencl___blur_h);
