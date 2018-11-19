#include <stdio.h>

#define STAT \
	printf("%d (%.2f) %.8f (%.3f%%) %.8f\n", i, (double)i / (60*24*365.25), (double)t / COIN, 100. * t / max, (double)r / COIN);

int main(void)
{
	const long long COIN = 100000000;
	long long t = 0, max = 126000000 * COIN, r, rp = 0;
	int i = 0, n = 100;

	while (t < max) {
		i++;
		r = 0;
		if (i >= 21 && i <= 80) {
			r = 1000000 * COIN;
		} else if (i <= 200) {
			r = 0;
		} else if (i <= 200 + 43200) {
			int j = i - 200;
			r = 20 * COIN * j / 43200;
		} else {
			int j = i - (200 + 43200);
			int d = 2200000 - 14400;
			int k = j / d;
			j %= d;
			if (k > 63)
				k = 63;
			r = (20 * COIN) >> k;
			r = r / 2 + r * (d - j) / (2 * d);

			if (r < 1 * COIN && rp >= 1 * COIN)
				STAT
		}
		t += r;

		if ((i <= 200 + 43200 && i != 81 && r < rp) || (i > 200 + 43200 && r > rp)) /* shouldn't happen */
			break;

		if ((i % n) == 0 || (t >= 100000000 * COIN && (t - r) < 100000000 * COIN)) {
			STAT
			if (n < 1000000)
				n *= 10;
		}

		rp = r;

		if (i > 200 && !r)
			break;
	}

	STAT

	return 0;
}
