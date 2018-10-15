// Last Update:2018-05-23 15:57:56
/**
 * @file main.h
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-05-23
 */

#ifndef MAIN_H
#define MAIN_H

#define DBG_BASIC() printf("[%s %s() +%d] ", __FILE__, __FUNCTION__, __LINE__)
//#define DBG_LOG(args...) DBG_BASIC();printf(args)
#define LINE() DBG_LOG("++++++++++++++++++++++++\n")
 
#endif  /*MAIN_H*/
