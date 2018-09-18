#!/usr/bin/python3
import re





kernels  = [
               [
                   "blur.c",
                   [
                       [ "_0", "_acc"        ],
                       [ "_1", "CENTER_X"    ],
                       [ "_2", "CENTER_Y"    ],
                       [ "_3", "out"         ],
                       [ "_5", "guessX"      ],
                       [ "_6", "guessY"      ],
                       [ "_7", "realityX"    ],
                       [ "_8", "realityY"    ],
                       [ "_9", "acc"         ],
                       [ "_A", "Arg"         ],
                       [ "_C", "ITEMS_CNT"   ], 
                       [ "_E", "_AIdx"       ],
                       [ "_a", "args"        ],
                       [ "_b", "begin"       ],
                       [ "_c", "cnt"         ],
                       [ "_e", "end"         ],
                       [ "_i", "items"       ],
                       [ "_r", "ratio"       ],
                       [ "_v", "Vector"      ],
                       [ "_x", "inX"         ],
                       [ "_y", "inY"         ],
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
