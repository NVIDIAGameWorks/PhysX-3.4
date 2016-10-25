/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __ARL_SORT_H__
#define __ARL_SORT_H__

namespace nvidia
{
namespace iofx
{

template <typename TKey, typename TValue>
class ARLSort
{
	static const int MAX_NUM_BIT = 8;  // largest bit num in sorting digit(might bast be tuned between 6 and 12)
	static const int MIN_NUM_BIT = 4;
	static const int INSERT_MAX  = 64; // any value below this, use Insertion sort

public:

	static void sort(TKey* keys, TValue* values, int numElems)
	{
		if (numElems >= INSERT_MAX)
		{
			TKey max = keys[0];
			for (int i = 1; i < numElems; ++i)
			{
				TKey key = keys[i];
				if (key > max)
				{
					max = key;
				}
			}
			if (max > 0)
			{
				max >>= 1;
				int leftBitNo = 0;
				// find highest bit set
				while (max > 0)
				{
					max >>= 1;
					leftBitNo++;
				}
				sortARLwithBorders(keys, values, 0, numElems - 1, leftBitNo);
			}
		}
		else
		{
			insertSort(keys, values, 0, numElems - 1);
		}
	}


private:
	static void insertSort(TKey* keys, TValue* values, int left, int right)
	{
		for (int k = left; k < right; ++k)
		{
			if (keys[k] > keys[k + 1])
			{
				TKey tmpKey = keys[k + 1];
				TValue tmpValue = values[k + 1];
				int i = k;
				do
				{
					keys[i + 1] = keys[i];
					values[i + 1] = values[i];
					--i;
				}
				while (i >= left && keys[i] > tmpKey);
				keys[i + 1] = tmpKey;
				values[i + 1] = tmpValue;
			}
		}
	}

	/**
	*  ARL sort with border array, The 2002 version Adaptive Left Radix with Insert-sort as a subalgorithm.
	*          Sorts positive integers from a[start] 'up to and including'  a[end]
	*           on bits: leftBitNo, leftBitNo-1,..,leftBitNo -numBit+1 (31..0)
	*          Uses only internal moves by shifting along permutation cycles <br>
	*           UNSTABLE
	*
	*   @Author: Arne Maus, Dept.of Informatics,Univ. of Oslo,  2002-2009
	Copyright (c) 2000,2009, Arne Maus, Dept. of Informatics, Univ. of Oslo, Norway.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above copyright
	      notice, this list of conditions and the following disclaimer in the
	      documentation and/or other materials provided with the distribution.
	    * Neither the name of the <organization> nor the
	      names of its contributors may be used to endorse or promote products
	      derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY ARNE MAUS, DEPT. OF INFORMATICS, UNIV. OF OSLO, NORWAY ''AS
	 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
	 EVENT SHALL ARNE MAUS, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

	static void sortARLwithBorders(TKey* keys, TValue* values, int start, int end, int leftBitNo)
	{
		int num = end - start + 1;

		int point[(1 << MAX_NUM_BIT) + 1];
		int border[(1 << MAX_NUM_BIT) + 1];

		// adaptive part - adjust numBit : number of bits to sort on in this pass
		// a) adapts to bits left to sort to sort AND cache-size level 1 (8-32KB)
		int numBit = leftBitNo + 1;
		if (numBit > MAX_NUM_BIT)
		{
			numBit = MAX_NUM_BIT;
		}
		// b) adapts to 'sparse' distribution
		while ((1 << (numBit - 1)) > num && numBit > MIN_NUM_BIT)
		{
			--numBit;
		}
		if (numBit == leftBitNo)
		{
			++numBit;    // eventually, do the last bit
		}

		// sort on leftmost 'numBits' starting at bit no: leftBitNo
		// setting constants
		int rBitNo = leftBitNo - numBit + 1;
		int lim = 1 << numBit;
		TKey mask =  TKey(lim - 1) << rBitNo;

		// sort on  'numBit' bits, from: leftBitNo'to 'rBitNo+1' in a[start..end]

		// c) count-scan 'numBit' bits
		memset(point, 0, sizeof(point));
		for (int i = start; i <= end ; i++)
		{
			point[(keys[i] & mask) >> rBitNo ]++;
		}

		int t1, t2 = point[0];
		point[0] = start;

		for (int i = 1; i <= lim; i++)
		{
			// d)  point [i] points to where bundle 'i' starts, stopvalue in borders[lim-1]
			t1 = t2;
			t2 = point[i];
			border[i - 1] = point[i] = point[i - 1] + t1;
		}

		border[lim - 1] = point[lim];
		border[lim] = end + 1;

		int currentBox = 0, pos = start;

		//  find next element to move in  permtation cycles
		// skip cycles of length =1

		while (point[currentBox] == border[currentBox])
		{
			currentBox++;
		}

		while (currentBox < lim)
		{
			// find next cycle, skip (most)cycles of length =1
			pos = point[currentBox];
			TKey key = keys[pos];
			TValue value = values[pos];

			// start of new permutation cycle
			int adr2 = point[(key & mask) >> rBitNo]++;

			if (adr2 > pos)
			{
				// permuttion cycle longer than 1 element
				do
				{
					TKey key2 = keys[adr2];
					TValue value2 = values[adr2];
					// central loop
					keys[adr2] = key;
					values[adr2] = value;

					adr2 = point[(key2 & mask) >> rBitNo]++;

					key = key2;
					value = value2;
				}
				while (adr2 > pos);

				keys[pos] = key;
				values[pos] = value;
			}// end perm cycle

			// find box where to find start of new  permutation cycle
			while (currentBox < lim && point[currentBox] == border[currentBox])
			{
				currentBox++;
			}

		} // end more to sort

		leftBitNo = leftBitNo - numBit;

		if (leftBitNo >= 0)
		{
			//  more to sort - recursively
			t2 = start;
			for (int i = 0; i < lim; i++)
			{
				t1 = t2;
				t2 = point[i];
				int newNum = t2 - t1;

				// call each cell if more than one number
				if (newNum > 1)
				{
					if (newNum <= INSERT_MAX)
					{
						insertSort(keys, values, t1, t2 - 1);
					}
					else
					{
						sortARLwithBorders(keys, values, t1, t2 - 1, leftBitNo);
					}
				} // if newNum > 1
			} // end for
		} // end if leftBitNo

	}// end sortARLwithBorders

};

}
} // namespace nvidia

#endif // __ARL_SORT_H__
