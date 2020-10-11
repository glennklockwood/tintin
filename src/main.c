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

#include <signal.h>
#include <sys/socket.h>

/*************** globals ******************/

struct session *gts;
struct tintin_data *gtd;

void pipe_handler(int signal)
{
	tintin_printf(NULL, "broken_pipe: dumping stack");

	dump_stack();
}

/*
	when the screen size changes, take note of it
*/

void winch_handler(int ssignal)
{
	struct session *ses;

	init_screen_size(gts);

	for (ses = gts->next ; ses ; ses = ses->next)
	{
		init_screen_size(ses);

		if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
		{
			send_sb_naws(ses, 0, NULL);
		}
	}

#if defined (__SVR4) && defined(__sun)
	/* we have to reinitialize the signals for sysv machines */
	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr("signal SIGWINCH");
	}
#endif
}


void abort_handler(int signal)
{
	if (gtd->ses->connect_retry > utime())
	{
		gtd->ses->connect_retry = 0;
	}
	else if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) && !HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		socket_printf(gtd->ses, 1, "%c", 3);
	}
	else
	{
		do_zap(gtd->ses, "");
	}
}

void suspend_handler(int signal)
{
	printf("\033[r\033[%d;%dH", gtd->ses->rows, 1);

	fflush(stdout);

	reset_terminal();

	kill(0, SIGSTOP);

	dirty_screen(gtd->ses);

	init_terminal();

	tintin_puts(NULL, "#RETURNING BACK TO TINTIN++.");
}

void trap_handler(int signal)
{
	static char crashed = FALSE;

	if (crashed)
	{
		exit(-1);
	}
	crashed = TRUE;

	reset_terminal();

	clean_screen(gtd->ses);

	dump_stack();

	fflush(NULL);

	exit(-1);
}


/****************************************************************************/
/* main() - show title - setup signals - init lists - readcoms - mainloop() */
/****************************************************************************/


int main(int argc, char **argv)
{
	#ifdef SOCKS
		SOCKSinit(argv[0]);
	#endif

	init_tintin();

	if (signal(SIGTERM, trap_handler) == BADSIG)
	{
		syserr("signal SIGTERM");
	}

	if (signal(SIGSEGV, trap_handler) == BADSIG)
	{
		syserr("signal SIGSEGV");
	}

	if (signal(SIGHUP, trap_handler) == BADSIG)
	{
		syserr("signal SIGHUP");
	}

	if (signal(SIGABRT, abort_handler) == BADSIG)
	{
		syserr("signal SIGTERM");
	}

	if (signal(SIGINT, abort_handler) == BADSIG)
	{
		syserr("signal SIGINT");
	}

	if (signal(SIGTSTP, suspend_handler) == BADSIG)
	{
		syserr("signal SIGSTOP");
	}

	if (signal(SIGPIPE, pipe_handler) == BADSIG)
	{
		syserr("signal SIGPIPE");
	}

	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr("signal SIGWINCH");
	}

/*
	if (getenv("HOME") != NULL)
	{
		char filename[256];

		sprintf(filename, "%s/%s", getenv("HOME"), HISTORY_FILE);

		read_history(gts, filename);
	}
*/

	srand48(time(NULL));

	if (argc > 1)
	{
		int c;

		while ((c = getopt(argc, argv, "e: h r: t: v")) != EOF)
		{
			switch (c)
			{
				case 'e':
					gtd->ses = script_driver(gtd->ses, -1, optarg);
					break;

				case 'h':
					tintin_printf(NULL, "Usage: %s [-v] [file] [-e \"command\"] [-r file] [-t title]", argv[0]);
					reset_terminal();
					exit(1);
					break;

				case 'r':
					gtd->ses = do_read(gtd->ses, optarg);
					break;

				case 't':
					printf("\033]0;%s\007", optarg);
					break;

				case 'v':
					do_configure(gtd->ses, "{VERBOSE} {ON}");
					break;

				default:
					tintin_printf(NULL, "Unknown option '%c'.", c);
					break;
			}
		}
		
		if (argv[optind] != NULL)
		{
			gtd->ses = do_read(gtd->ses, argv[optind]);
		}
	} 

	mainloop();

	return 0;
}


void init_tintin(void)
{
	int ref, cnt;

	gts = calloc(1, sizeof(struct session));

	for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
	{
		gts->list[cnt] = init_list(cnt);
	}

	gts->name           = strdup("gts");
	gts->class          = strdup("");
	gts->host           = strdup("");
	gts->port           = strdup("");
	gts->cmd_color      = strdup("");
	gts->telopts        = TELOPT_FLAG_ECHO;
	gts->flags          = SES_FLAG_MCCP;
	gts->socket         = 1;
	gts->read_max       = 4096;

	gtd                 = calloc(1, sizeof(struct tintin_data));

	gtd->ses            = gts;

	gtd->str_hash_size  = sizeof(struct str_hash_data);

	gtd->mccp_len       = 4096;
	gtd->mccp_buf       = calloc(1, gtd->mccp_len);

	gtd->mud_output_max = 4096;
	gtd->mud_output_buf = calloc(1, gtd->mud_output_max);

	gtd->input_off      = 1;

	for (cnt = 0 ; cnt < 100 ; cnt++)
	{
		gtd->vars[cnt] = strdup("");
		gtd->cmds[cnt] = strdup("");
	}

	for (ref = 0 ; ref < 26 ; ref++)
	{
		for (cnt = 0 ; *command_table[cnt].name != 0 ; cnt++)
		{
			if (*command_table[cnt].name == 'a' + ref)
			{
				gtd->command_ref[ref] = cnt;
				break;
			}
		}
	}

	init_screen_size(gts);

	/*
		Set application keypad mode and  ESC 0 prefix
	*/

	printf("\033=\033[?1h");

	do_configure(gts, "{AUTO TAB}         {5000}");
	do_configure(gts, "{BUFFER SIZE}     {20000}");
	do_configure(gts, "{COMMAND COLOR}   {<078>}");
	do_configure(gts, "{COMMAND ECHO}       {ON}");
	do_configure(gts, "{CONNECT RETRY}      {15}");
	do_configure(gts, "{HISTORY SIZE}     {2500}");
	do_configure(gts, "{LOG}               {RAW}");
	do_configure(gts, "{PACKET PATCH}        {0}");
	do_configure(gts, "{REPEAT CHAR}         {!}");
	do_configure(gts, "{REPEAT ENTER}      {OFF}");
	do_configure(gts, "{SCROLL LOCK}        {ON}");
	do_configure(gts, "{SPEEDWALK}         {OFF}");
	do_configure(gts, "{TINTIN CHAR}         {#}");
	do_configure(gts, "{VERBATIM}          {OFF}");
	do_configure(gts, "{VERBATIM CHAR}      {\\}");
	do_configure(gts, "{VERBOSE}           {OFF}");
	do_configure(gts, "{WORDWRAP}           {ON}");

	insertnode_list(gts,  "n",  "s",  "1", LIST_PATHDIR);
	insertnode_list(gts,  "e",  "w",  "2", LIST_PATHDIR);
	insertnode_list(gts,  "s",  "n",  "4", LIST_PATHDIR);
	insertnode_list(gts,  "w",  "e",  "8", LIST_PATHDIR);
	insertnode_list(gts,  "u",  "d", "16", LIST_PATHDIR);
	insertnode_list(gts,  "d",  "u", "32", LIST_PATHDIR);

	insertnode_list(gts, "ne", "sw",  "3", LIST_PATHDIR);
	insertnode_list(gts, "nw", "se",  "9", LIST_PATHDIR);
	insertnode_list(gts, "se", "nw",  "6", LIST_PATHDIR);
	insertnode_list(gts, "sw", "ne", "12", LIST_PATHDIR);

	init_terminal();

	do_showme(gts, "");
	do_showme(gts, "<068>      #<068>###################################################################<068>#");
	do_showme(gts, "<068>      #<078>                                                                   <068>#");
	do_showme(gts, "<068>      #<078>                           T I N T I N + +                         <068>#");
	do_showme(gts, "<068>      #<078>                                                                   <068>#");
	do_showme(gts, "<068>      #<078>            (<068>T<078>)he k(<068>I<078>)cki(<068>N<078>) (<068>T<078>)ickin d(<068>I<078>)kumud clie(<068>N<078>)t           <068>#");
	do_showme(gts, "<068>      #<078>                                                                   <068>#");
	do_showme(gts, "<068>      #<078>                 Original TINTIN code by Peter Unold               <068>#");
	do_showme(gts, "<068>      #<078>       new code by Bill Reiss, David A. Wagner, Rob Ellsworth,     <068>#");
	do_showme(gts, "<068>      #<078>                 Jeremy C. Jack, Igor van den Hoven                <068>#");
	do_showme(gts, "<068>      #<078>                             1992, 2009                            <068>#");
	do_showme(gts, "<068>      #<078>                                                                   <068>#");
	do_showme(gts, "<068>      #<078>                               "VERSION_NUM"                              <068>#");
	do_showme(gts, "<068>      #<078>                                                                   <068>#");
	do_showme(gts, "<068>      #<068>###################################################################<068>#<088>");
	do_showme(gts, "");
}


void quitmsg(char *message)
{
	struct session *ses;

	while ((ses = gts->next) != NULL)
	{
		cleanup_session(ses);
	}

	if (gtd->chat)
	{
		close(gtd->chat->fd);
	}

	check_all_events(gts, 0, 0, "PROGRAM TERMINATION");

/*
	if (gtd->history_size)
	{
		char filename[BUFFER_SIZE];

		sprintf(filename, "%s/%s", getenv("HOME"), HISTORY_FILE);

		history_write(gts, filename);
	}
*/
	reset_terminal();

	clean_screen(gts);

	if (message)
	{
		printf("\n%s\n", message);
	}

	printf("\nGoodbye from TinTin++\n\n");

	fflush(NULL);

	exit(0);
}
