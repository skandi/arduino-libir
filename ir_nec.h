/*
 * Copyright (C) 2010 Andreas Wetzel
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef IRNEC_H_
#define IRNEC_H_

#define KEY_REPEAT 256
#define KEY_LAST KEY_REPEAT
#define KEY_NONE (KEY_LAST+1)

class IRemoteNEC {
public:
	IRemoteNEC( int irpin, int myremote);
	int read();
	void isr();

private:
	inline char bswap( char v);

	int m_irpin;
	int m_iraddress;
	unsigned long m_outkey;


};

#endif /* IRNEC_H_ */
