#!/usr/bin/python3
import re





kernels  = [
               [
                   "blur.c",
                   [
                       [ "_0", "WIDTH"       ],
                       [ "_1", "HEIGHT"      ],
                       [ "_2", "DST_STRIVE"  ],
                       [ "_3", "SRC_STRIVE"  ],
                       [ "_5", "SIZE"        ],
                       [ "_6", "width"       ],
                       [ "_7", "height"      ],
                       [ "_8", "dst_strive"  ],
                       [ "_9", "src_strive"  ],
                       [ "_A", "size"        ],
                       [ "_a", "args"        ],
                       [ "_b", "size2"       ],
                       [ "_c", "start"       ],
                       [ "_d", "dst"         ],
                       [ "_e", "end"         ],
                       [ "_f", "current"     ],
                       [ "_s", "src"         ],
                       [ "_z", "__ARGS_CNT"  ]
                   ]
               ]
           ]




def _aliasses(txt, aliasses):
    for a in aliasses:
        txt = re.sub(a[1], a[0], txt)
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
    
    
    
    
def _buildKernel(kernel):
    kernelIn   = kernel[0]
    kernelName = kernel[0][:-2]
    kernelOut  = "opencl___"    + kernelName
    print("Processing kernel '" + kernelIn + "' --> '" + kernelOut + "'")
    
    fi    = open(kernelIn, "r")
    lines = fi.readlines()[4:]
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
        
    txt = _aliasses(txt, kernel[1])
    
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






for kernel in kernels:
    _buildKernel(kernel)
