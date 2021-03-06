/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2004 (See CREDITS file)                                     *
*                                                                             *
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"


void printline(struct session *ses, char *str, int prompt)
{
	char wrapped_str[STRING_SIZE];

	push_call("printline(%p,%p,%d)",ses,str,prompt);

	if (ses->scroll_line != -1 && HAS_BIT(ses->flags, SES_FLAG_SCROLLLOCK))
 	{
		pop_call();
		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCAN) && !HAS_BIT(ses->flags, SES_FLAG_VERBOSE))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_WORDWRAP))
	{
		word_wrap(ses, str, wrapped_str, TRUE);
	}
	else
	{
		strcpy(wrapped_str, str);
	}

	if (prompt)
	{
		printf("%s", wrapped_str);
	}
	else
	{
		printf("%s\n", wrapped_str);
	}
	pop_call();
	return;
}

/*
	Word wrapper, only wraps scrolling region, returns nr of lines - Igor
*/

int word_wrap(struct session *ses, char *textin, char *textout, int scroll)
{
	char *pti, *pto, *lis, *los;
	int skip = 0, cnt = 0;

	pti = lis = textin;
	pto = los = textout;

	ses->cur_col = 1;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		while (*pti)
		{
			switch (*pti)
			{
				case ESCAPE:
					*pto++ = '\\';
					*pto++ = 'e';
					break;

				default:
					if (*pti < 32)
					{
						*pto++ = '\\';
						*pto++ = 'c';
						*pto++ = 'a' + *pti - 1;
					}
					else
					{
						*pto++ = *pti;
					}
					break;
			}
			pti++;
		}
		*pto = 0;

		strcpy(textin, textout);

		pti = lis = textin;
		pto = los = textout;
	}

	while (*pti != 0)
	{
		if (skip_vt102_codes(pti))
		{
			if (interpret_vt102_codes(ses, pti, TRUE))
			{
				for (skip = skip_vt102_codes(pti) ; skip > 0 ; skip--)
				{
					*pto++ = *pti++;
				}
			}
			else
			{
				pti += skip_vt102_codes(pti);
			}
			continue;
		}

		if (*pti == '\n')
		{
			*pto++ = *pti++;
			cnt = cnt + 1;
			los = pto;
			lis = pti;

			ses->cur_col = 1;

			continue;
		}

		if (*pti == ' ')
		{
			los = pto;
			lis = pti;
		}

		if (ses->cur_col > ses->cols)
		{
			cnt++;
			ses->cur_col = 1;

			if (pto - los > 15 || !SCROLL(ses))
			{
				*pto++ = '\n';
				los = pto;
				lis = pti;
			}
			else
			{
				pto = los;
				*pto++ = '\n';
				pti = lis;
				pti++;
			}
		}
		else
		{
			ses->cur_col++;

			*pto++ = *pti++;
		}
	}
	*pto = 0;

	return (cnt + 1);
}
