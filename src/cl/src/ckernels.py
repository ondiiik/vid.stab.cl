#!/usr/bin/python3
import re





kernels  = [
               [
                   "blur_h.c",
                   [
                       [ "BLUR_H___DST_STRIVE"  ],
                       [ "BLUR_H___SRC_STRIVE"  ],
                       [ "__BLUR_H___ARGS_CNT"  ],
                       [ "BLUR_H___WIDTH"       ],
                       [ "BLUR_H___HEIGHT"      ],
                       [ "BLUR_H___SIZE"        ],
                       [ "dst_strive"           ],
                       [ "src_strive"           ],
                       [ "current"              ],
                       [ "height"               ],
                       [ "start"                ],
                       [ "width"                ],
                       [ "size2"                ],
                       [ "size"                 ],
                       [ "args"                 ],
                       [ "dst"                  ],
                       [ "end"                  ],
                       [ "src"                  ],
                       [ "acc"                  ],
                   ]
               ],
               
               [
                   "blur_v.c",
                   [
                       [ "BLUR_V___DST_STRIVE"  ],
                       [ "BLUR_V___SRC_STRIVE"  ],
                       [ "__BLUR_V___ARGS_CNT"  ],
                       [ "BLUR_V___HEIGHT"      ],
                       [ "BLUR_V___SIZE"        ],
                       [ "BLUR_V___WIDTH"       ],
                       [ "dst_strive"           ],
                       [ "src_strive"           ],
                       [ "current"              ],
                       [ "height"               ],
                       [ "start"                ],
                       [ "width"                ],
                       [ "size2"                ],
                       [ "size"                 ],
                       [ "args"                 ],
                       [ "dst"                  ],
                       [ "end"                  ],
                       [ "src"                  ],
                       [ "acc"                  ],
                   ]
               ],
           ]




def _aliasses(txt, aliasses, kid):
    prefix = "_" + kid + "_"
    eidx   = 0
    for a in aliasses:
        txt   = re.sub(a[0], prefix + hex(eidx)[2:], txt)
        eidx += 1
    return txt
    
    
    
    
    
def _removeExp(txt, exp):
    return re.sub(" ?[" + exp + "] ?", exp, txt)
    
    
    
    
def _simplify(txt):
    txt = _removeExp(txt, "{")
    txt = _removeExp(txt, "}")
    txt = _removeExp(txt, "[")
    txt = _removeExp(txt, "]")
    txt = _removeExp(txt, "(")
    txt = _removeExp(txt, ")")
    txt = _removeExp(txt, ",")
    txt = _removeExp(txt, ";")
    txt = _removeExp(txt, "+")
    txt = _removeExp(txt, "-")
    txt = _removeExp(txt, "*")
    txt = _removeExp(txt, "/")
    txt = _removeExp(txt, "%")
    txt = _removeExp(txt, "=")
    txt = _removeExp(txt, ">")
    txt = _removeExp(txt, "<")
    return txt
    
    
    
    
def _buildKernel(kernel, kidx):
    kernelIn   = kernel[0]
    kernelName = kernel[0][:-2]
    kernelOut  = "opencl___"    + kernelName
    print("Processing kernel '" + kernelIn + "' --> '" + kernelOut + "'")
    
    fi    = open(kernelIn, "r")
    lines = fi.readlines()[5:]
    fi.close()
    
    txt = ""
    for ln in lines:
        txt += ln
    
    txt = re.sub("\/\*[^/]*\*\/", "",  txt)
    txt = re.sub(" +",            " ", txt)
    txt = re.sub("\n", "", txt)
    txt = re.sub('"', '\\"', txt)
    
    for i in range(32):
        txt = _simplify(txt)
        
    txt = _aliasses(txt, kernel[1], hex(kidx)[2:])
    
    blockSize = 76
    rng       = range(0, len(txt), blockSize)
    fc        = open("../" + kernelOut + ".c", "w")
    fh        = open("../" + kernelOut + ".h", "w")
    
    fc.write("const char "     + kernelOut + "[] = \n\n\n")
    fc.write("#if defined(OPENCL_DBG_MODE)\n")
    fc.write("\n\n")
    maxl = 0
    for ln in lines:
        l = len(ln)
        if maxl < l:
            maxl = l
    for ln in lines:
        ln = re.sub("\n", "", ln)
        fc.write(('" %-' + str(maxl) + 's\\n"\n') % ln)
    fc.write("\n\n")
    fc.write("#else /* defined(OPENCL_DBG_MODE) */\n")
    fc.write("\n\n")
    for i in rng:
        fc.write('"' + txt[i:i + blockSize] + '"\n')
    fc.write("\n\n")
    fc.write("#endif /* defined(OPENCL_DBG_MODE) */\n")
    fc.write("\n")
    fc.write("\n")
    fc.write(";\n")
    fc.write("\n")
    fc.write("\n")
    fc.write(       "const unsigned " + kernelOut + "_len = sizeof(" + kernelOut + ");\n")
    
    
    fh.write("#pragma once\n")
    fh.write("\n")
    fh.write("\n")
    fh.write("\n")
    fh.write("#ifdef __cplusplus\n")
    fh.write("extern \"C\" {\n")
    fh.write("#endif\n")
    fh.write("\n")
    fh.write("\n")
    fh.write("\n")
        
    fh.write("extern const char     " + kernelOut + "[];\n")
    fh.write("extern const unsigned " + kernelOut + "_len;\n")
    
    fh.write("\n")
    fh.write("\n")
    fh.write("\n")
    fh.write("#ifdef __cplusplus\n")
    fh.write("}\n")
    fh.write("#endif\n")
    
    fc.close()
    fh.close()





kidx = 0
for kernel in kernels:
    _buildKernel(kernel, kidx)
    kidx += 1
