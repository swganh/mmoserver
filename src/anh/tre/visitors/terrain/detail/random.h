// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
/*
 * Random.h
 *
 *  Created on: 17/12/2010
 *      Author: victor
 */
#pragma once

class Random {
	int seed;
	int unknown;

	int table[322];

public:
	Random() {
		unknown = 0;
		seed = 0;
	}

	void setSeed(int seed) {
		this->seed = -seed;
	}

	int next() {
		int v2, v8;

		if (seed <= 0 || unknown == 0) {
			v2 = -seed;

			if (v2 >= 1)
				seed = v2;
			else
				seed = 1;

			for (int i = 329; i >= 0; --i) {
				int valueTo1 = seed;
				int value2 = 16807 * valueTo1;
				int tempDiv = valueTo1 / 0x1F31D;
				int value3 = (0x7FFFFFFF * tempDiv);
				seed = value2 - value3;

				if (seed < 0)
					seed = seed + 0x7FFFFFFF;

				if (i < 322) {
					table[i] = seed;
				}
			}

			unknown = table[0];
		}

		int valueTo1 = seed;
		int value2 = 16807 * valueTo1;
		int tempDiv = valueTo1 / 0x1F31D;
		int value3 = (0x7FFFFFFF * tempDiv);

		seed = value2 - value3;

		if (seed < 0)
			seed = seed + 0x7FFFFFFF;

		v8 = (int)((double) unknown / 6669205.0); // random index?

		unknown = table[v8];
		table[v8] = seed;

		return unknown;
	}
};
