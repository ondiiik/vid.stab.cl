/*
 * common_dbg.h
 *
 *  Created on: 14. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include <iostream>


#define DBG_PRINT_VAR1(_txt_, _var_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << ' ' <<  '\t' << _txt_ << '\t' << \
        #_var_ " = " << _var_ << std::endl; \
    } \
    while (false)

#define DBG_PRINT_VAR2(_txt_, _var1_, _var2_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << ' ' <<  '\t' << _txt_ << '\t' << \
        #_var1_ " = " << _var1_  << ", " \
        #_var2_ " = " << _var2_  << std::endl; \
    } \
    while (false)

#define DBG_PRINT_VAR3(_txt_, _var1_, _var2_, _var3_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << ' ' <<  '\t' << _txt_ << '\t' << \
        #_var1_ " = " << _var1_  << ", " \
        #_var2_ " = " << _var2_  << ", " \
        #_var3_ " = " << _var3_  << std::endl; \
    } \
    while (false)

#define DBG_PRINT_VAR4(_txt_, _var1_, _var2_, _var3_, _var4_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << ' ' <<  '\t' << _txt_ << '\t' << \
        #_var1_ " = " << _var1_  << ", " \
        #_var2_ " = " << _var2_  << ", " \
        #_var3_ " = " << _var3_  << ", " \
        #_var4_ " = " << _var4_  << std::endl; \
    } \
    while (false)

#define DBG_PRINT_VAR5(_txt_, _var1_, _var2_, _var3_, _var4_, _var5_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << ' ' <<  '\t' << _txt_ << '\t' << \
        #_var1_ " = " << _var1_  << ", " \
        #_var2_ " = " << _var2_  << ", " \
        #_var3_ " = " << _var3_  << ", " \
        #_var4_ " = " << _var4_  << ", " \
        #_var5_ " = " << _var5_  << std::endl; \
    } \
    while (false)

#define DBG_PRINT_VAR6(_txt_, _var1_, _var2_, _var3_, _var4_, _var5_, _var6_) \
    do \
    { \
        std::cout << "[DBG] " << __FUNCTION__ << ":" << __LINE__ << '\t' << _txt_ << '\t' << \
        #_var1_ " = " << _var1_  << ", " \
        #_var2_ " = " << _var2_  << ", " \
        #_var3_ " = " << _var3_  << ", " \
        #_var4_ " = " << _var4_  << ", " \
        #_var5_ " = " << _var5_  << ", " \
        #_var6_ " = " << _var6_  << std::endl; \
    } \
    while (false)

