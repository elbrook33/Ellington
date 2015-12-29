#ifndef HELPERS_H
#define HELPERS_H

#define min(a,b)	\
({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a < _b ? _a : _b;	\
})

#define max(a,b)	\
({	\
	typeof (a) _a = (a);	\
	typeof (b) _b = (b);	\
	_a > _b ? _a : _b;	\
})

int strcount(const char* text, char c)
{
	int count;
	for(count = 0; *text; text++)
		{ count += (*text == c); }
	return count;
}

#endif
