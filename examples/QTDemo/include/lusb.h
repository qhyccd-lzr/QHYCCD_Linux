/*
 * lusb.h
 *
 *  Created on: 23.05.2011
 *      Author: gm
 */

#ifndef LUSB_H_
#define LUSB_H_

class lusb
{
public:
	static int  initialize( void );
	static int  release( void );
	static bool is_initialized( void );
private:
	static int m_init_cnt;
};

#endif /* LUSB_H_ */
