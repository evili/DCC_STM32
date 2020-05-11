/*
 * pintf-stdarg.h
 *
 *  Created on: 24 d’abr. 2020
 *      Author: evili
 */

#ifndef __PRINTF_STDARG_H__
#define __PRINTF_STDARG_H__

int printf(const char *format, ...);
int snprintf( char *buf, unsigned int count, const char *format, ... );

#endif /* __PRINTF_STDARG_H__ */
