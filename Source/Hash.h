#pragma once

inline int HashInt(int key) 
{
	key = ~key + (key << 15);
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057;
	key = key ^ (key >> 16);
	return key;
}

inline int HashTriplet(int x, int y, int z) 
{
	x = HashInt(x);
	y = HashInt(y);
	z = HashInt(z);
	return x ^ y ^ z;
}