/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2005 (See CREDITS file)                                     *
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
*******************************************************************************/

/******************************************************************************
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                                                                             *
*                     coded by Igor van den Hoven 2006                        *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_cursor)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(arg, left,  TRUE);
	arg = get_arg_in_braces(arg, right, TRUE);
	substitute(ses, right, right, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		tintin_header(ses, " CURSOR OPTIONS ");

		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (*cursor_table[cnt].name)
			{
				convert_meta(cursor_table[cnt].code, temp);

				tintin_printf2(ses, "  [%-15s] [%-6s] %s", 
					cursor_table[cnt].name,
					temp,
					cursor_table[cnt].desc);
			}
		}

		tintin_header(ses, "");
	}
	else
	{
		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (is_abbrev(left, cursor_table[cnt].name))
			{
				cursor_table[cnt].fun(right);

				return ses;
			}
		}
		tintin_printf(ses, "#ERROR: #CURSOR {%s} IS NOT A VALID OPTION.", capitalize(left));
	}
	return ses;
}

DO_CURSOR(cursor_backspace)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	cursor_left("");
	cursor_delete("");
}


DO_CURSOR(cursor_buffer_down)
{
	buffer_down(gtd->ses, "");
}

DO_CURSOR(cursor_buffer_end)
{
	buffer_end(gtd->ses, "");
}

DO_CURSOR(cursor_buffer_home)
{
	buffer_home(gtd->ses, "");
}

DO_CURSOR(cursor_buffer_lock)
{
	buffer_lock(gtd->ses, "");
}

DO_CURSOR(cursor_buffer_up)
{
	buffer_up(gtd->ses, "");
}


DO_CURSOR(cursor_check_line)
{
	if (gtd->input_pos != gtd->input_cur)
	{
		if (gtd->input_cur < gtd->ses->cols - 2)
		{
			cursor_redraw_line("");

			return;
		}
	}

	if (gtd->input_pos > 2 && gtd->input_pos < gtd->ses->cols - 2)
	{
		return;
	}

	if (gtd->input_pos >= gtd->ses->cols - 2)
	{
		cursor_redraw_line("");

		return;
	}

	if (gtd->input_pos != gtd->input_cur && gtd->input_pos <= 2)
	{
		cursor_redraw_line("");

		return;
	}
}

DO_CURSOR(cursor_clear_left)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur, gtd->input_buf);

	memmove(&gtd->input_buf[0], &gtd->input_buf[gtd->input_cur+1], gtd->input_len - gtd->input_cur);

	input_printf("\033[%dD\033[%dP", gtd->input_cur, gtd->input_cur);

	gtd->input_len -= gtd->input_cur;

	gtd->input_buf[gtd->input_len] = 0;

	if (gtd->input_cur != gtd->input_pos)
	{
		gtd->input_cur  = 0;
		gtd->input_pos  = 0;
		cursor_redraw_line("");
	}
	else
	{
		gtd->input_cur  = 0;
		gtd->input_pos  = 0;
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}

DO_CURSOR(cursor_clear_line)
{
	if (gtd->input_len == 0)
	{
		return;
	}

	if (gtd->input_pos)
	{
		input_printf("\033[%dG\033[%dP", gtd->input_off, gtd->input_len);
	}
	else
	{
		input_printf("\033[%dP", gtd->input_len);
	}

	gtd->input_len = 0;
	gtd->input_cur = 0;
	gtd->input_pos = 0;
	gtd->input_buf[0] = 0;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}

DO_CURSOR(cursor_clear_right)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	strcpy(gtd->paste_buf, &gtd->input_buf[gtd->input_cur]);

	input_printf("\033[%dP", gtd->input_len - gtd->input_pos);

	gtd->input_buf[gtd->input_cur] = 0;

	gtd->input_len = gtd->input_cur;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}

DO_CURSOR(cursor_convert_meta)
{
	SET_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR);
}


DO_CURSOR(cursor_delete_exit)
{
	if (gtd->input_len == 0)
	{
		cursor_exit("");
	}
	else
	{
		cursor_delete("");
	}
}

DO_CURSOR(cursor_delete)
{
	if (gtd->input_len == 0)
	{
		return;
	}

	if (gtd->input_len == gtd->input_cur)
	{
		return;
	}

	memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[gtd->input_cur+1], gtd->input_len - gtd->input_cur);

	gtd->input_len--;

	input_printf("\033[1P");

	cursor_fix_line("");

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}

DO_CURSOR(cursor_delete_word_left)
{
	int index_cur;

	if (gtd->input_cur == 0)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_cur--;
		gtd->input_pos--;
	}

	while (gtd->input_cur && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		gtd->input_cur--;
		gtd->input_pos--;
	}

	sprintf(gtd->paste_buf, "%.*s", index_cur - gtd->input_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[index_cur], gtd->input_len - index_cur + 1);

	input_printf("\033[%dD\033[%dP", index_cur - gtd->input_cur, index_cur - gtd->input_cur);

	gtd->input_len -= index_cur - gtd->input_cur;

	cursor_check_line("");

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}


DO_CURSOR(cursor_delete_word_right)
{
	int index_cur, index_pos;

	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	index_cur = gtd->input_cur;
	index_pos = gtd->input_pos;

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur - index_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[index_cur], &gtd->input_buf[gtd->input_cur], gtd->input_len - gtd->input_cur + 1);

	input_printf("\033[%dP", gtd->input_cur - index_cur);

	gtd->input_len -= gtd->input_cur - index_cur;

	gtd->input_pos = index_pos;
	gtd->input_cur = index_cur;

	cursor_fix_line("");

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find("");
	}
}

DO_CURSOR(cursor_echo)
{
	if (*arg == 0)
	{
		TOG_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO);
	}
	else
	{
		tintin_printf(NULL, "#SYNTAX: #CURSOR {ECHO} {ON|OFF}.");
	}
}

DO_CURSOR(cursor_end)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}
	gtd->input_cur = gtd->input_len;

	cursor_redraw_line("");
}

DO_CURSOR(cursor_enter)
{
	input_printf("\n");

	gtd->input_buf[gtd->input_len] = 0;

	gtd->input_len    = 0;
	gtd->input_cur    = 0;
	gtd->input_pos    = 0;
	gtd->macro_buf[0] = 0;
	gtd->input_tmp[0] = 0;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		if (gtd->input_his)
		{
			strcpy(gtd->input_buf, gtd->input_his->left);
		}

		gtd->input_off = 1;
	}
	gtd->input_his = NULL;

	SET_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);
}

DO_CURSOR(cursor_exit)
{
	if (gtd->ses == gts)
	{
		do_end(NULL, "");
	}
	else
	{
		cleanup_session(gtd->ses);
	}
}

DO_CURSOR(cursor_history_next)
{
	struct listnode *node;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (gtd->input_his == NULL)
		{
			return;
		}

		for (node = gtd->input_his->next ; node ; node = node->next)
		{
			if (*gtd->input_buf && find(gtd->ses, node->left, gtd->input_buf))
			{
				break;
			}
		}

		if (node)
		{
			input_printf("\033[%dC   \033[0K%.*s\033[%dG", gtd->input_len - gtd->input_cur + 1, gtd->ses->cols - 16 - gtd->input_len, node->left, gtd->input_pos + 11);

			gtd->input_his = node;
		}
		return;
	}

	if (gtd->input_his == NULL)
	{
		return;
	}

	node = gtd->input_his->next;

	while (node && strncmp(gtd->input_tmp, node->left, strlen(gtd->input_tmp)))
	{
		node = node->next;
	}

	cursor_clear_line("");

	gtd->input_his = node;

	if (gtd->input_his == NULL)
	{
		strcpy(gtd->input_buf, gtd->input_tmp);
	}
	else
	{
		strcpy(gtd->input_buf, gtd->input_his->left);
	}

	gtd->input_len = strlen(gtd->input_buf);
	gtd->input_cur = gtd->input_len;
	gtd->input_pos = 0;

	cursor_redraw_line("");
}

DO_CURSOR(cursor_history_prev)
{
	struct listnode *node;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (gtd->input_his == NULL || *gtd->input_buf == 0)
		{
			return;
		}

		for (node = gtd->input_his->prev ; node ; node = node->prev)
		{
			if (find(gtd->ses, node->left, gtd->input_buf))
			{
				break;
			}
		}

		if (node)
		{
			input_printf("\033[%dC   \033[0K%.*s\033[%dG", gtd->input_len - gtd->input_cur + 1, gtd->ses->cols - 16 - gtd->input_len, node->left, gtd->input_pos + 11);

			gtd->input_his = node;
		}
		return;
	}

	if (gts->list[LIST_HISTORY]->l_node == NULL)
	{
		return;
	}

	if (gtd->input_his == NULL)
	{
		strcpy(gtd->input_tmp, gtd->input_buf);

		node = gts->list[LIST_HISTORY]->l_node;

		while (node && strncmp(gtd->input_tmp, node->left, strlen(gtd->input_tmp)))
		{
			node = node->prev;
		}
	}
	else
	{
		node = gtd->input_his->prev;

		while (node && strncmp(gtd->input_tmp, node->left, strlen(gtd->input_tmp)))
		{
			node = node->prev;
		}
	}

	if (node == NULL)
	{
		return;
	}

	gtd->input_his = node;

	cursor_clear_line("");

	strcpy(gtd->input_buf, gtd->input_his->left);

	gtd->input_len = strlen(gtd->input_his->left);
	gtd->input_cur = gtd->input_len;
	gtd->input_pos = 0;

	cursor_redraw_line("");
}

DO_CURSOR(cursor_history_search)
{
	if (gts->list[LIST_HISTORY]->l_node == NULL)
	{
		return;
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		strcpy(gtd->input_tmp, gtd->input_buf);

		cursor_clear_line("");

		SET_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->input_off = 11;

		input_printf("(search) [ ] \033[3D");
	}
	else
	{
		if (gtd->input_his)
		{
			strcpy(gtd->input_buf, gtd->input_his->left);

			input_printf("\033[1G\033[0K");

			gtd->input_len = strlen(gtd->input_buf);
			gtd->input_cur = gtd->input_len;
			gtd->input_pos = gtd->input_len;

			gtd->input_his = NULL;

			DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

			gtd->input_off = 1;

			cursor_redraw_line("");
		}
		else
		{
			strcpy(gtd->input_buf, gtd->input_tmp);

			input_printf("\033[1G\033[0K");

			gtd->input_len = strlen(gtd->input_buf);
			gtd->input_cur = gtd->input_len;
			gtd->input_pos = gtd->input_len;

			gtd->input_his = NULL;

			DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

			gtd->input_off = 1;

			cursor_redraw_line("");
		}
	}
}

DO_CURSOR(cursor_history_find)
{
	struct listnode *node;
	struct listroot *root;

	push_call("cursor_history_find(%s)", gtd->input_buf);

	root = gts->list[LIST_HISTORY];

	for (node = root->l_node ; node ; node = node->prev)
	{
		if (*gtd->input_buf && find(gtd->ses, node->left, gtd->input_buf))
		{
			break;
		}
	}

	gtd->input_his = node;

	if (node)
	{
		if (gtd->input_len == gtd->input_cur)
		{
			input_printf("\033[0K]   %.*s\033[%dG", gtd->ses->cols - 16 - gtd->input_len, node->left, gtd->input_pos + 11);
		}
		else
		{
			input_printf("\033[%dC\033[0K]\033[3C%.*s\033%dG", gtd->input_len - gtd->input_cur, gtd->ses->cols - 16 - gtd->input_len, node->left, gtd->input_pos + 1);
		}
	}
	else
	{
		if (gtd->input_len == gtd->input_cur)
		{
			input_printf("\033[0K]\033[%dG", gtd->input_pos + 11);
		}
		else
		{
			input_printf("\033[%dC\033[0K]\033[%dG", gtd->input_len - gtd->input_cur, gtd->input_pos + 11);
		}
	}
	pop_call();
}

DO_CURSOR(cursor_home)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	input_printf("\033[%dD", gtd->input_pos);

	if (gtd->input_cur != gtd->input_pos)
	{
		gtd->input_cur = 0;
		gtd->input_pos = 0;

		cursor_redraw_line("");
	}
	else
	{
		gtd->input_cur = 0;
		gtd->input_pos = 0;
	}
}

DO_CURSOR(cursor_insert)
{
	if (*arg == 0)
	{
		TOG_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else
	{
		tintin_printf(NULL, "#SYNTAX: #CURSOR {INSERT} {ON|OFF}.");
	}
}


DO_CURSOR(cursor_left)
{
	if (gtd->input_cur > 0)
	{
		gtd->input_cur--;
		gtd->input_pos--;
		input_printf("\033[1D");

		cursor_check_line("");
	}
}

DO_CURSOR(cursor_left_word)
{
	int index_cur;

	if (gtd->input_cur == 0)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_cur--;
		gtd->input_pos--;
	}

	while (gtd->input_cur && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		gtd->input_cur--;
		gtd->input_pos--;
	}

	input_printf("\033[%dD", index_cur - gtd->input_cur);

	cursor_check_line("");
}


DO_CURSOR(cursor_paste_buffer)
{
	ins_sprintf(&gtd->input_buf[gtd->input_cur], "%s", gtd->paste_buf);

	gtd->input_len += strlen(gtd->paste_buf);
	gtd->input_cur += strlen(gtd->paste_buf);

	cursor_redraw_line("");
}


DO_CURSOR(cursor_redraw_input)
{
	if (IS_SPLIT(gtd->ses))
	{
		cursor_redraw_line("");
	}
	else
	{
		input_printf("\033[1G\033[0K%s%s\033[0K", gtd->ses->more_output, gtd->input_buf);

		gtd->input_cur = gtd->input_len;

		gtd->input_pos = gtd->input_len % gtd->ses->cols;
	}
}


DO_CURSOR(cursor_redraw_line)
{
	input_printf("\033[%dG\033[%dX", gtd->input_off, gtd->input_len);

	if (gtd->input_pos != gtd->input_cur)
	{
		if (gtd->input_cur < gtd->ses->cols - 2)
		{
			gtd->input_pos = gtd->input_cur;
		}
	}

	if (gtd->input_pos >= gtd->ses->cols - 2)
	{
		while (gtd->input_pos >= gtd->ses->cols - 2)
		{
			gtd->input_pos -= gtd->ses->cols / 3;
		}
	}

	if (gtd->input_pos != gtd->input_cur)
	{
		if (gtd->input_pos <= 2)
		{
			while (gtd->input_pos <= 2)
			{
				gtd->input_pos += gtd->ses->cols * 2 / 3;
			}
		}
	}

	if (gtd->input_pos != gtd->input_cur)
	{
		input_printf("<%.*s\033[%dG", gtd->ses->cols - 1, &gtd->input_buf[gtd->input_cur - gtd->input_pos + 1], gtd->input_pos+gtd->input_off);
	}
	else
	{
		input_printf("%.*s\033[%dG", gtd->ses->cols, &gtd->input_buf[gtd->input_cur - gtd->input_pos], gtd->input_pos+gtd->input_off);
	}

	cursor_fix_line("");
}

DO_CURSOR(cursor_fix_line)
{
	if (gtd->input_len - gtd->input_cur + gtd->input_pos > gtd->ses->cols)
	{
		input_printf("\033[%dG%.1s>\033[%dG", gtd->ses->cols - 1, &gtd->input_buf[gtd->input_cur + gtd->ses->cols - gtd->input_pos-2], gtd->input_pos+gtd->input_off);
	}
	else if (gtd->input_len - gtd->input_cur + gtd->input_pos == gtd->ses->cols)
	{
		input_printf("\033[%dG%.2s\033%dG",  gtd->ses->cols - 1, &gtd->input_buf[gtd->input_cur + gtd->ses->cols - gtd->input_pos-2], gtd->input_pos+gtd->input_off);
	}
}

DO_CURSOR(cursor_right)
{
	if (gtd->input_cur < gtd->input_len)
	{
		gtd->input_cur++;
		gtd->input_pos++;
		input_printf("\033[1C");
	}

	cursor_check_line("");
}

DO_CURSOR(cursor_right_word)
{
	int index_cur;

	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	input_printf("\033[%dC", gtd->input_cur - index_cur);

	cursor_check_line("");
}

DO_CURSOR(cursor_suspend)
{
	do_suspend(gtd->ses, "");
}

DO_CURSOR(cursor_tab)
{
	char tab[BUFFER_SIZE];
	struct listnode *node;

	int input_now;

	if (gtd->input_len == 0)
	{
		return;
	}

	input_now = gtd->input_len - 1;

	while (input_now && gtd->input_buf[input_now] != ' ')
	{
		input_now--;
	}

	if (input_now != 0 && input_now == gtd->input_len - 1)
	{
		return;
	}

	if (input_now)
	{
		input_now++;
	}

	if (gtd->ses->list[LIST_TAB]->f_node == NULL)
	{
		cursor_auto_tab(arg);

		return;
	}

	for (node = gtd->ses->list[LIST_TAB]->f_node ; node ; node = node->next)
	{
		substitute(gtd->ses, node->left, tab, SUB_VAR|SUB_FUN);

		if (is_abbrev(&gtd->input_buf[input_now], tab))
		{
			if (gtd->input_cur == gtd->input_len)
			{
				input_printf("\033[%dD\033[%dP%s", gtd->input_len - input_now, gtd->input_len - input_now, tab);
			}
			else
			{
				input_printf("\033[%dC\033[%dD\033[%dP%s", gtd->input_len - gtd->input_cur, gtd->input_len - input_now, gtd->input_len - input_now, tab);
			}
			strcpy(&gtd->input_buf[input_now], tab);

			gtd->input_len = input_now + strlen(tab);
			gtd->input_cur = gtd->input_len;
			gtd->input_pos = gtd->input_len;

			break;
		}
	}
}

char *get_arg_stop_space_sym(char *string, char *result)
{
	char *pto, *pti;
	int nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
#ifdef BIG5
		if (*pti & 0x80)
		{
			*pto++ = *pti++;

			if (*pti)
			{
				*pto++ = *pti++;
			}
			continue;
		}
#endif

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0 && !VERBATIM)
		{
			break;
		}
		else if (!isalpha(*pti) && nest == 0)
		{
			pti++;
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0';

	return pti;
}

DO_CURSOR(cursor_auto_tab)
{
	char tab[BUFFER_SIZE], buf[BUFFER_SIZE];
	int input_now, scroll_cnt, line_cnt;

	if (gtd->input_len == 0)
	{
		return;
	}

	input_now = gtd->input_len - 1;

	while (input_now && gtd->input_buf[input_now] != ' ')
	{
		input_now--;
	}

	if (input_now != 0 && input_now == gtd->input_len - 1)
	{
		return;
	}

	if (input_now)
	{
		input_now++;
	}

	line_cnt = 0;

	scroll_cnt = gtd->ses->scroll_row;

	do
	{
		if (scroll_cnt == gtd->ses->scroll_max -1)
		{
			scroll_cnt = 0;
		}
		else
		{
			scroll_cnt++;
		}

		if (gtd->ses->buffer[scroll_cnt] == NULL)
		{
			break;
		}

		if (str_hash_grep(gtd->ses->buffer[scroll_cnt], FALSE))
		{
			continue;
		}

		if (line_cnt++ >= gtd->ses->auto_tab)
		{
			break;
		}

		strip_vt102_codes(gtd->ses->buffer[scroll_cnt], buf);

		arg = buf;

		while (*arg)
		{
			arg = get_arg_stop_space_sym(arg, tab);

			if (!strncmp(tab, &gtd->input_buf[input_now], strlen(&gtd->input_buf[input_now])))
			{
				if (gtd->input_cur == gtd->input_len)
				{
					input_printf("\033[%dD\033[%dP%s", gtd->input_len - input_now, gtd->input_len - input_now, tab);
				}
				else
				{
					input_printf("\033[%dC\033[%dD\033[%dP%s", gtd->input_len - gtd->input_cur, gtd->input_len - input_now, gtd->input_len - input_now, tab);
				}
				strcpy(&gtd->input_buf[input_now], tab);

				gtd->input_len = input_now + strlen(tab);
				gtd->input_cur = gtd->input_len;
				gtd->input_pos = gtd->input_len;

				return;
			}

			if (*arg == ';')
			{
				arg++;
			}
		}
	}
	while (scroll_cnt != gtd->ses->scroll_row);
}
