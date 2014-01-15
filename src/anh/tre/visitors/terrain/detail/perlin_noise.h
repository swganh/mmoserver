// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
/*
 * PerlinNoise.h
 *
 *  Created on: 17/12/2010
 *      Author: victor
 */
#pragma once

/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */

#define PB 0x100
#define PBM 0xff

#define PN 0x1000
#define PNP 12   /* 2^N */
#define PNM 0xfff

#include "random.h"
#include <cmath>

class PerlinNoise {
	int p[PB + PB + 2];
	//float g3[B + B + 2][3];
	float g2[PB + PB + 2][2];
	float g1[PB + PB + 2];

	int start;

	Random* rand;

#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
		t = (double)vec[i] + (double)PN;\
		b0 = ((int)t) & PBM;\
		b1 = (b0+1) & PBM;\
		r0 = t - (int)t;\
		r1 = r0 - 1.;

public:
	void prep(Random* r)
	{
		rand = r;
		start = 1;
	}

	float noise2(double vec[2]) {
		int bx0, bx1, by0, by1, b00, b10, b01, b11;
		double rx0, rx1, ry0, ry1;
		float *q;
		double t, sx, sy, a, b, u, v;
		register int i, j;

		if (start) {
			start = 0;
			init();
		}

		setup(0, bx0,bx1, rx0,rx1);

		setup(1, by0,by1, ry0,ry1);

		i = p[ bx0 ];
		j = p[ bx1 ];

		b00 = p[ i + by0 ];
		b10 = p[ j + by0 ];
		b01 = p[ i + by1 ];
		b11 = p[ j + by1 ];

		sx = s_curve(rx0);

		sy = s_curve(ry0);


#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

		q = g2[ b00 ] ;
		u = at2(rx0,ry0);


		q = g2[ b10 ] ;
		v = at2(rx1,ry0);


		a = lerp(sx, u, v);

		q = g2[ b01 ] ;
		u = at2(rx0,ry1);

		q = g2[ b11 ] ;
		v = at2(rx1,ry1);

		b = lerp(sx, u, v);

		return (float)lerp(sy, a, b);
	}

	static void normalize2(float v[2]) {
		double s;

		s = sqrt(v[0] * v[0] + v[1] * v[1]);
		v[0] = (float)(v[0] / s);
		v[1] = (float)(v[1] / s);
	}

	static void normalize3(float v[3]) {
		double s;

		s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		v[0] = (float)(v[0] / s);
		v[1] = (float)(v[1] / s);
		v[2] = (float)(v[2] / s);
	}

	void init() {
		int i, j, k;

		for (i = 0 ; i < PB ; i++) {
			p[i] = i;

			g1[i] = (float)((double)((rand->next() % (PB + PB)) - PB) / PB);

			for (j = 0 ; j < 2 ; j++) {
				g2[i][j] = (float)((double)((rand->next() % (PB + PB)) - PB) / PB);
			}

			normalize2(g2[i]);

			/*for (j = 0 ; j < 3 ; j++)
				g3[i][j] = (double)((rand->generateTable() % (B + B)) - B) / B;

			normalize3(g3[i]);*/
		}

		while (--i) {
			k = p[i];
			p[i] = p[j = rand->next() % PB];
			p[j] = k;
		}

		for (i = 0 ; i < PB + 2 ; i++) {
			p[PB + i] = p[i];
			g1[PB + i] = g1[i];

			for (j = 0 ; j < 2 ; j++)
				g2[PB + i][j] = g2[i][j];

			/*for (j = 0 ; j < 3 ; j++)
				g3[B + i][j] = g3[i][j];*/
		}
	}

};
