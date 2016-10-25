#ifndef GET_ARGS_H

#define GET_ARGS_H

#define MAX_ARGS 4096
#define MAX_ARG_STRING 16384

class GetArgs
{
public:

	const char **getArgs(const char *str,uint32_t &argc)
	{
		argc = 0;

		str = skipSpaces(str); // skip any leading spaces
		char *dest = mArgString;
		char *stop = &mArgString[MAX_ARG_STRING-1];

		while ( str && *str && dest < stop ) // if we have a valid string then we have at least one argument..
		{
			if ( *str == 34 ) // if it is a quoted string...
			{
				str++; // Skip the opening quote
				if ( *str == 34 ) // double quotes I guess we treat as an empty string
				{
					mArgv[argc] = "";
					argc++;
				}
				else
				{
					mArgv[argc] = dest; // store the beginning of the argument
					argc++;
					while ( *str && *str != 34 && dest < stop )
					{
						*dest++ = *str++;
					}
					*dest = 0; // null terminate the quoted argument.
					dest++;
				}
				if ( *str == 34 ) // skip closing quote
				{
					str++;
				}
				str = skipSpaces(str);
			}
			else
			{
				mArgv[argc] = dest;
				argc++;
				while ( *str && !isWhiteSpace(*str) && dest < stop )
				{
					*dest++ = *str++;
				}
				*dest = 0;
				dest++;
				str = skipSpaces(str);
			}
		}

		return mArgv;
	}

private:
	PX_INLINE bool isWhiteSpace(char c) const
	{
		if ( c == 32 || c == 9 ) return true;
		return false;
	}

	PX_INLINE const char *skipSpaces(const char *str) const
	{
		if ( str )
		{
			while ( *str == 32 || *str == 9 )
			{
				str++;
			}
		}
		return str;
	}

	const char	*mArgv[MAX_ARGS];
	char		mArgString[MAX_ARG_STRING];

};

#endif
