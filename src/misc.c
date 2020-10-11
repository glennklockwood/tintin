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
*******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Peter Unold 1992                           *
******************************************************************************/

#include "tintin.h"


DO_COMMAND(do_all)
{
	char left[BUFFER_SIZE];
	struct session *sesptr, *next_ses;

	if (gts->next)
	{
		get_arg_in_braces(arg, left, TRUE);

		substitute(ses, left, left, SUB_VAR|SUB_FUN);

		for (sesptr = gts->next ; sesptr ; sesptr = next_ses)
		{
			next_ses = sesptr->next;

			script_driver(sesptr, -1, left);
		}
	}
	else
	{
		tintin_puts2(ses, "#BUT THERE AREN'T ANY SESSIONS AT ALL!");
	}
	return ses;
}


DO_COMMAND(do_bell)
{
	printf("\007");

	return ses;
}


DO_COMMAND(do_commands)
{
	char buf[BUFFER_SIZE] = { 0 }, add[BUFFER_SIZE];
	int cmd;

	tintin_header(ses, " %s ", "COMMANDS");

	for (cmd = 0 ; *command_table[cmd].name != 0 ; cmd++)
	{
		if (!is_abbrev(arg, command_table[cmd].name))
		{
			continue;
		}
		if (strlen(buf) + 20 > ses->cols)
		{
			tintin_puts2(ses, buf);
			buf[0] = 0;
		}
		sprintf(add, "%20s", command_table[cmd].name);
		strcat(buf, add);
	}
	if (buf[0])
	{
		tintin_puts2(ses, buf);
	}
	tintin_header(ses, "");

	return ses;
}


DO_COMMAND(do_cr)
{
	write_mud(ses, "", SUB_EOL);

	return ses;
}


DO_COMMAND(do_echo)
{
	char temp[BUFFER_SIZE], output[BUFFER_SIZE];
	int lnf;

	sprintf(temp, "result %s", arg);

	internal_variable(ses, "{result}");

	do_format(ses, temp);

	substitute(ses, "$result", temp, SUB_VAR|SUB_FUN|SUB_COL);

	lnf = !str_suffix(temp, "\\");

	substitute(ses, temp, temp, SUB_ESC);

	if (strip_vt102_strlen(ses->more_output) != 0)
	{
		sprintf(output, "\n\033[0m%s\033[0m", temp);
	}
	else
	{
		sprintf(output, "\033[0m%s\033[0m", temp);
	}

	add_line_buffer(ses, output, lnf);

	if (ses != gtd->ses)
	{
		return ses;
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		save_pos(ses);
		goto_rowcol(ses, ses->bot_row, 1);
	}

	printline(ses, output, lnf);

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		restore_pos(ses);
	}

	return ses;
}

DO_COMMAND(do_end)
{
	if (*arg)
	{
		quitmsg(arg);
	}
	else
	{
		quitmsg(NULL);
	}
	return NULL;
}


DO_COMMAND(do_forall)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, left,  TRUE);
	arg = get_arg_in_braces(arg, right, TRUE);

	substitute(ses, left, left, SUB_VAR|SUB_FUN);

	if (*left == 0 || *right == 0)
	{
		tintin_printf2(ses, "#ERROR: #FORALL - PROVIDE 2 ARGUMENTS");
	}
	else
	{
		arg = left;

		while (*arg)
		{
			arg = get_arg_in_braces(arg, temp, FALSE);

			RESTRING(gtd->cmds[0], temp);

			substitute(ses, right, temp, SUB_CMD);

			ses = script_driver(ses, -1, temp);
		}
	}
	return ses;
}

DO_COMMAND(do_gagline)
{
	tintin_printf2(ses, "#GAGLINE: Old command, please use #LINE GAG instead.");

	SET_BIT(ses->flags, SES_FLAG_GAG);

	return ses;
}


DO_COMMAND(do_info)
{
	int cnt;

	if (*arg == 'c')
	{
		show_cpu(ses);

		return ses;
	}

	if (*arg == 's')
	{
		dump_stack();

		return ses;
	}

	tintin_header(ses, " INFORMATION ");

	for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
	{
		if (!HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_SHOW))
		{
			continue;
		}
		tintin_printf2(ses, "%-20s  %5d  IGNORE %3s  MESSAGE %3s  DEBUG %3s",
			list_table[cnt].name_multi,
			ses->list[cnt]->count,
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF",
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF",
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_DEBUG)   ? "ON" : "OFF");
	}
	tintin_header(ses, "");

	return ses;
}


DO_COMMAND(do_loop)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE], command[BUFFER_SIZE];

	long long bound1, bound2, counter, step;

	arg = get_arg_in_braces(arg, temp,    0);
	arg = get_arg_in_braces(arg, command, 1);

	arg = get_arg_in_braces(temp, left, 0);
	arg = get_arg_in_braces(arg, right, 0);


	if (*command == 0 || *right == 0)
	{
		show_message(ses, LIST_MATH, "#LOOP: INVALID ARGUMENTS.");

		return ses;
	}

	bound1 = (long long) get_number(ses, left);
	bound2 = (long long) get_number(ses, right);

	if (bound1 <= bound2)
	{
		step = 1;
		bound2++;
	}
	else
	{
		step = -1;
		bound2--;
	}

	for (counter = bound1 ; counter != bound2 ; counter += step)
	{
		gtd->cmds[0] = refstring(gtd->cmds[0], "%lld", counter);

		substitute(ses, command, temp, SUB_CMD);

		ses = script_driver(ses, -1, temp);
	}

	return ses;
}



DO_COMMAND(do_nop)
{
	return ses;
}


DO_COMMAND(do_parse)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(arg, left,  0);
	arg = get_arg_in_braces(arg, right, 1);

	substitute(ses, left, left, SUB_VAR|SUB_FUN);

	if (*left == 0 || *right == 0)
	{
		tintin_printf2(ses, "#ERROR: #PARSE - PROVIDE 2 ARGUMENTS");
	}
	else
	{
		for (cnt = 0 ; left[cnt] != 0 ; cnt++)
		{
#ifdef BIG5
			if (left[cnt] & 0x80 && left[cnt+1] != 0)
			{
				sprintf(temp, "%c%c", left[cnt], left[cnt+1]);

				cnt++;
			}
			else
			{
				sprintf(temp, "%c", left[cnt]);
			}
#else
			sprintf(temp, "%c", left[cnt]);
#endif

			RESTRING(gtd->cmds[0], temp);

			substitute(ses, right, temp, SUB_CMD);

			ses = script_driver(ses, -1, temp);
		}
	}
	return ses;
}


DO_COMMAND(do_return)
{
	if (*arg)
	{
		internal_variable(ses, "{result} %s", arg);
	}
	return ses;
}

DO_COMMAND(do_send)
{
	char left[BUFFER_SIZE];

	push_call("do_send(%p,%p)",ses,arg);

	get_arg_in_braces(arg, left, TRUE);

	write_mud(ses, left, SUB_VAR|SUB_FUN|SUB_ESC|SUB_EOL);

	pop_call();
	return ses;
}

DO_COMMAND(do_showme)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[STRING_SIZE];
	int lnf;

	arg = get_arg_in_braces(arg, left, TRUE);
	lnf = !str_suffix(left, "\\");
	substitute(ses, left, temp, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);

	arg = get_arg_in_braces(arg, right, FALSE);
	substitute(ses, right, right, SUB_VAR|SUB_FUN);

	if (*right)
	{
		do_one_prompt(ses, temp, atoi(right));

		return ses;
	}

	do_one_line(temp, ses);

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		DEL_BIT(ses->flags, SES_FLAG_GAG);

		return ses;
	}

	if (strip_vt102_strlen(ses->more_output) != 0)
	{
		sprintf(right, "\n\033[0m%s\033[0m", temp);
	}
	else
	{
		sprintf(right, "\033[0m%s\033[0m", temp);
	}

	add_line_buffer(ses, right, lnf);

	if (ses != gtd->ses)
	{
		return ses;
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		save_pos(ses);
		goto_rowcol(ses, ses->bot_row, 1);
	}

	printline(ses, right, lnf);

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		restore_pos(ses);
	}

	return ses;
}

DO_COMMAND(do_snoop)
{
	struct session *sesptr = ses;
	char left[BUFFER_SIZE];

	get_arg_in_braces(arg, left, 1);
	substitute(ses, left, left, SUB_VAR|SUB_FUN);

	if (*left)
	{
		for (sesptr = gts->next ; sesptr ; sesptr = sesptr->next)
  		{
  			if (!strcmp(sesptr->name, left))
  			{
  				break;
  			}
  		}
		if (sesptr == NULL)
		{
			tintin_puts2(ses, "#NO SESSION WITH THAT NAME!");

			return ses;
		}
	}
	else
	{
		sesptr = ses;
	}

	if (HAS_BIT(sesptr->flags, SES_FLAG_SNOOP))
	{
		tintin_printf2(ses, "#UNSNOOPING SESSION '%s'", sesptr->name);
	}
	else
	{
		tintin_printf2(ses, "#SNOOPING SESSION '%s'", sesptr->name);
	}
	TOG_BIT(sesptr->flags, SES_FLAG_SNOOP);

	return ses;
}

DO_COMMAND(do_suspend)
{
	suspend_handler(0);

	return ses;
}


DO_COMMAND(do_while)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE];

	arg = get_arg_in_braces(arg, left, FALSE);
	arg = get_arg_in_braces(arg, right, TRUE);

	if (*left == 0 || *right == 0)
	{
		show_message(ses, LIST_MATH, "#SYNTAX: WHILE {CONDITION} {COMMANDS}");

		return ses;
	}

	while (get_number(ses, left))
	{
		ses = script_driver(ses, -1, right);
	}
	return ses;
}

DO_COMMAND(do_zap)
{
	tintin_puts(ses, "");

	tintin_puts(ses, "#ZZZZZZZAAAAAAAAPPPP!!!!!!!!! LET'S GET OUTTA HERE!!!!!!!!");

	if (ses == gts)
	{
		return do_end(NULL, "");
	}
	cleanup_session(ses);

	return gtd->ses;
}

