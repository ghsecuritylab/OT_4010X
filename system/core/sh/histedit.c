
/*	$NetBSD: histedit.c,v 1.34 2003/10/27 06:19:29 lukem Exp $	*/


#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)histedit.c	8.2 (Berkeley) 5/4/95";
#else
__RCSID("$NetBSD: histedit.c,v 1.34 2003/10/27 06:19:29 lukem Exp $");
#endif
#endif /* not lint */

#include <sys/param.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"
#include "parser.h"
#include "var.h"
#include "options.h"
#include "main.h"
#include "output.h"
#include "mystring.h"
#include "myhistedit.h"
#include "error.h"
#ifndef SMALL
#include "eval.h"
#include "memalloc.h"

#define MAXHISTLOOPS	4	/* max recursions through fc */
#define DEFEDITOR	"ed"	/* default editor *should* be $EDITOR */

History *hist;	/* history cookie */
EditLine *el;	/* editline cookie */
int displayhist;
static FILE *el_in, *el_out;

STATIC const char *fc_replace(const char *, char *, char *);

#ifdef DEBUG
extern FILE *tracefile;
#endif

void
histedit(void)
{
	FILE *el_err;

#define editing (Eflag || Vflag)

	if (iflag) {
		if (!hist) {
			/*
			 * turn history on
			 */
			INTOFF;
			hist = history_init();
			INTON;

			if (hist != NULL)
				sethistsize(histsizeval());
			else
				out2str("sh: can't initialize history\n");
		}
		if (editing && !el && isatty(0)) { /* && isatty(2) ??? */
			/*
			 * turn editing on
			 */
			char *term, *shname;

			INTOFF;
			if (el_in == NULL)
				el_in = fdopen(0, "r");
			if (el_out == NULL)
				el_out = fdopen(2, "w");
			if (el_in == NULL || el_out == NULL)
				goto bad;
			el_err = el_out;
#if DEBUG
			if (tracefile)
				el_err = tracefile;
#endif
			term = lookupvar("TERM");
			if (term)
				setenv("TERM", term, 1);
			else
				unsetenv("TERM");
			shname = arg0;
			if (shname[0] == '-')
				shname++;
			el = el_init(shname, el_in, el_out, el_err);
			if (el != NULL) {
				if (hist)
					el_set(el, EL_HIST, history, hist);
				el_set(el, EL_PROMPT, getprompt);
				el_set(el, EL_SIGNAL, 1);
			} else {
bad:
				out2str("sh: can't initialize editing\n");
			}
			INTON;
		} else if (!editing && el) {
			INTOFF;
			el_end(el);
			el = NULL;
			INTON;
		}
		if (el) {
			if (Vflag)
				el_set(el, EL_EDITOR, "vi");
			else if (Eflag)
				el_set(el, EL_EDITOR, "emacs");
			el_source(el, NULL);
		}
	} else {
		INTOFF;
		if (el) {	/* no editing if not interactive */
			el_end(el);
			el = NULL;
		}
		if (hist) {
			history_end(hist);
			hist = NULL;
		}
		INTON;
	}
}


void
sethistsize(const char *hs)
{
	int histsize;
	HistEvent he;

	if (hist != NULL) {
		if (hs == NULL || *hs == '\0' ||
		   (histsize = atoi(hs)) < 0)
			histsize = 100;
		history(hist, &he, H_SETSIZE, histsize);
	}
}

void
setterm(const char *term)
{
	if (el != NULL && term != NULL)
		if (el_set(el, EL_TERMINAL, term) != 0) {
			outfmt(out2, "sh: Can't set terminal type %s\n", term);
			outfmt(out2, "sh: Using dumb terminal settings.\n");
		}
}

int
inputrc(argc, argv)
	int argc;
	char **argv;
{
	if (argc != 2) {
		out2str("usage: inputrc file\n");
		return 1;
	}
	if (el != NULL) {
		if (el_source(el, argv[1])) {
			out2str("inputrc: failed\n");
			return 1;
		} else
			return 0;
	} else {
		out2str("sh: inputrc ignored, not editing\n");
		return 1;
	}
}

int
histcmd(int argc, char **argv)
{
	int ch;
	const char *editor = NULL;
	HistEvent he;
	int lflg = 0, nflg = 0, rflg = 0, sflg = 0;
	int i, retval;
	const char *firststr, *laststr;
	int first, last, direction;
	char *pat = NULL, *repl;	/* ksh "fc old=new" crap */
	static int active = 0;
	struct jmploc jmploc;
	struct jmploc *volatile savehandler;
	char editfile[MAXPATHLEN + 1];
	FILE *efp;
#ifdef __GNUC__
	/* Avoid longjmp clobbering */
	(void) &editor;
	(void) &lflg;
	(void) &nflg;
	(void) &rflg;
	(void) &sflg;
	(void) &firststr;
	(void) &laststr;
	(void) &pat;
	(void) &repl;
	(void) &efp;
	(void) &argc;
	(void) &argv;
#endif

	if (hist == NULL)
		error("history not active");

	if (argc == 1)
		error("missing history argument");

	optreset = 1; optind = 1; /* initialize getopt */
	while (not_fcnumber(argv[optind]) &&
	      (ch = getopt(argc, argv, ":e:lnrs")) != -1)
		switch ((char)ch) {
		case 'e':
			editor = optionarg;
			break;
		case 'l':
			lflg = 1;
			break;
		case 'n':
			nflg = 1;
			break;
		case 'r':
			rflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case ':':
			error("option -%c expects argument", optopt);
			/* NOTREACHED */
		case '?':
		default:
			error("unknown option: -%c", optopt);
			/* NOTREACHED */
		}
	argc -= optind, argv += optind;

	/*
	 * If executing...
	 */
	if (lflg == 0 || editor || sflg) {
		lflg = 0;	/* ignore */
		editfile[0] = '\0';
		/*
		 * Catch interrupts to reset active counter and
		 * cleanup temp files.
		 */
		if (setjmp(jmploc.loc)) {
			active = 0;
			if (*editfile)
				unlink(editfile);
			handler = savehandler;
			longjmp(handler->loc, 1);
		}
		savehandler = handler;
		handler = &jmploc;
		if (++active > MAXHISTLOOPS) {
			active = 0;
			displayhist = 0;
			error("called recursively too many times");
		}
		/*
		 * Set editor.
		 */
		if (sflg == 0) {
			if (editor == NULL &&
			    (editor = bltinlookup("FCEDIT", 1)) == NULL &&
			    (editor = bltinlookup("EDITOR", 1)) == NULL)
				editor = DEFEDITOR;
			if (editor[0] == '-' && editor[1] == '\0') {
				sflg = 1;	/* no edit */
				editor = NULL;
			}
		}
	}

	/*
	 * If executing, parse [old=new] now
	 */
	if (lflg == 0 && argc > 0 &&
	     ((repl = strchr(argv[0], '=')) != NULL)) {
		pat = argv[0];
		*repl++ = '\0';
		argc--, argv++;
	}
	/*
	 * determine [first] and [last]
	 */
	switch (argc) {
	case 0:
		firststr = lflg ? "-16" : "-1";
		laststr = "-1";
		break;
	case 1:
		firststr = argv[0];
		laststr = lflg ? "-1" : argv[0];
		break;
	case 2:
		firststr = argv[0];
		laststr = argv[1];
		break;
	default:
		error("too many args");
		/* NOTREACHED */
	}
	/*
	 * Turn into event numbers.
	 */
	first = str_to_event(firststr, 0);
	last = str_to_event(laststr, 1);

	if (rflg) {
		i = last;
		last = first;
		first = i;
	}
	/*
	 * XXX - this should not depend on the event numbers
	 * always increasing.  Add sequence numbers or offset
	 * to the history element in next (diskbased) release.
	 */
	direction = first < last ? H_PREV : H_NEXT;

	/*
	 * If editing, grab a temp file.
	 */
	if (editor) {
		int fd;
		INTOFF;		/* easier */
		snprintf(editfile, sizeof(editfile), "%s_shXXXXXX", _PATH_TMP);
		if ((fd = mkstemp(editfile)) < 0)
			error("can't create temporary file %s", editfile);
		if ((efp = fdopen(fd, "w")) == NULL) {
			close(fd);
			error("can't allocate stdio buffer for temp");
		}
	}

	/*
	 * Loop through selected history events.  If listing or executing,
	 * do it now.  Otherwise, put into temp file and call the editor
	 * after.
	 *
	 * The history interface needs rethinking, as the following
	 * convolutions will demonstrate.
	 */
	history(hist, &he, H_FIRST);
	retval = history(hist, &he, H_NEXT_EVENT, first);
	for (;retval != -1; retval = history(hist, &he, direction)) {
		if (lflg) {
			if (!nflg)
				out1fmt("%5d ", he.num);
			out1str(he.str);
		} else {
			const char *s = pat ?
			   fc_replace(he.str, pat, repl) : he.str;

			if (sflg) {
				if (displayhist) {
					out2str(s);
				}

				evalstring(strcpy(stalloc(strlen(s) + 1), s), 0);
				if (displayhist && hist) {
					/*
					 *  XXX what about recursive and
					 *  relative histnums.
					 */
					history(hist, &he, H_ENTER, s);
				}
			} else
				fputs(s, efp);
		}
		/*
		 * At end?  (if we were to lose last, we'd sure be
		 * messed up).
		 */
		if (he.num == last)
			break;
	}
	if (editor) {
		char *editcmd;

		fclose(efp);
		editcmd = stalloc(strlen(editor) + strlen(editfile) + 2);
		sprintf(editcmd, "%s %s", editor, editfile);
		evalstring(editcmd, 0);	/* XXX - should use no JC command */
		INTON;
		readcmdfile(editfile);	/* XXX - should read back - quick tst */
		unlink(editfile);
	}

	if (lflg == 0 && active > 0)
		--active;
	if (displayhist)
		displayhist = 0;
	return 0;
}

STATIC const char *
fc_replace(const char *s, char *p, char *r)
{
	char *dest;
	int plen = strlen(p);

	STARTSTACKSTR(dest);
	while (*s) {
		if (*s == *p && strncmp(s, p, plen) == 0) {
			while (*r)
				STPUTC(*r++, dest);
			s += plen;
			*p = '\0';	/* so no more matches */
		} else
			STPUTC(*s++, dest);
	}
	STACKSTRNUL(dest);
	dest = grabstackstr(dest);

	return (dest);
}

int
not_fcnumber(char *s)
{
	if (s == NULL)
		return 0;
        if (*s == '-')
                s++;
	return (!is_number(s));
}

int
str_to_event(const char *str, int last)
{
	HistEvent he;
	const char *s = str;
	int relative = 0;
	int i, retval;

	retval = history(hist, &he, H_FIRST);
	switch (*s) {
	case '-':
		relative = 1;
		/*FALLTHROUGH*/
	case '+':
		s++;
	}
	if (is_number(s)) {
		i = atoi(s);
		if (relative) {
			while (retval != -1 && i--) {
				retval = history(hist, &he, H_NEXT);
			}
			if (retval == -1)
				retval = history(hist, &he, H_LAST);
		} else {
			retval = history(hist, &he, H_NEXT_EVENT, i);
			if (retval == -1) {
				/*
				 * the notion of first and last is
				 * backwards to that of the history package
				 */
				retval = history(hist, &he,
						last ? H_FIRST : H_LAST);
			}
		}
		if (retval == -1)
			error("history number %s not found (internal error)",
			       str);
	} else {
		/*
		 * pattern
		 */
		retval = history(hist, &he, H_PREV_STR, str);
		if (retval == -1)
			error("history pattern not found: %s", str);
	}
	return (he.num);
}
#else
int
histcmd(int argc, char **argv)
{
	error("not compiled with history support");
	/* NOTREACHED */
}
int
inputrc(int argc, char **argv)
{
	error("not compiled with history support");
	/* NOTREACHED */
}
#endif
