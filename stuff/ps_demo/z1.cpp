#include <stdio.h>
#include <math.h>
#include <double\mconf.h>

void main()
{
	for (int k = 0; k <= 100; ++k)
	{
		double f = 0.01 * k;
		merror = 0;
		printf("%d %f", k, f);
		double d = ndtri(f);
		if (!merror)
		{
  		printf(" %f\n", d);
		}
	}
}
