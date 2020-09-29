/* flex - tool to generate fast lexical analyzers */

/*  Copyright (c) 1990 The Regents of the University of California. */
/*  All rights reserved. */

/*  This code is derived from software contributed to Berkeley by */
/*  Vern Paxson. */

/*  The United States Government has rights in this work pursuant */
/*  to contract no. DE-AC03-76SF00098 between the United States */
/*  Department of Energy and the University of California. */

/*  This file is part of flex. */

/*  Redistribution and use in source and binary forms, with or without */
/*  modification, are permitted provided that the following conditions */
/*  are met: */

/*  1. Redistributions of source code must retain the above copyright */
/*     notice, this list of conditions and the following disclaimer. */
/*  2. Redistributions in binary form must reproduce the above copyright */
/*     notice, this list of conditions and the following disclaimer in the */
/*     documentation and/or other materials provided with the distribution. */

/*  Neither the name of the University nor the names of its contributors */
/*  may be used to endorse or promote products derived from this software */
/*  without specific prior written permission. */

/*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/*  PURPOSE. */


#include "flexdef.h"
#include "tables.h"

/* These typedefs are only used for computing footprint sizes,
 * You need to make sure they match reality in the skeleton file to 
 * get accurate numbers, but they don't otherwise matter.
 */
typedef char YY_CHAR;
struct yy_trans_info {int32_t yy_verify; int32_t yy_nxt;};

/* Helper fubctions */

static const char *cpp_get_int16_decl (void)
{
	return (gentables)
		? "static const flex_int16_t %s[%d] =\n    {   0,\n"
		: "static const flex_int16_t * %s = 0;\n";
}


static const char *cpp_get_int32_decl (void)
{
	return (gentables)
		? "static const flex_int32_t %s[%d] =\n    {   0,\n"
		: "static const flex_int32_t * %s = 0;\n";
}

static const char *cpp_get_state_decl (void)
{
	return (gentables)
		? "static const yy_state_type %s[%d] =\n    {   0,\n"
		: "static const yy_state_type * %s = 0;\n";
}

static const char *cpp_get_yy_char_decl (void)
{
	return (gentables)
		? "static const YY_CHAR %s[%d] =\n    {   0,\n"
		: "static const YY_CHAR * %s = 0;\n";
}

/* Methods */

static const char *cpp_suffix (void)
{
	char   *suffix;

	if (C_plus_plus)
	    suffix = "cc";
	else
	    suffix = "c";

	return suffix;
}

/* cpp_prolog - make rules prolog specific to cpp-using languages.
 *
 * If you don't ship this, you will effectively be assuming that your
 * parsers are always reentrant, always allow reject, always have a
 * yywrap() method, have a debug member in the wrapper class, and are
 * interactive.  This eliminates most of the boilerplate in the C/C++
 * scanner prolog.  It means such parsers will be a bit larger and
 * slower than C/C++ ones, but since we're not running on 1987's
 * hardware we officially do not care.
 *
 * A detail to beware of: If you're not issuing this prologue, you may
 * want to write your own definition of YY_CHAR in your skel
 * file. It's a typedef to an unsigned octet in C/C++, but if your
 * target language has a Unicode code-point type like Go's 'rune' is
 * may be appropriate.
 */

static void cpp_prolog (void)
{
	static char yy_stdinit[] = "FILE *yyin = stdin, *yyout = stdout;";
	static char yy_nostdinit[] =
		"FILE *yyin = NULL, *yyout = NULL;";

	if (!do_yywrap) {
		if (!C_plus_plus) {
			 if (reentrant)
				out_str ("\n#define %swrap(yyscanner) (/*CONSTCOND*/1)\n", prefix);
			 else
				out_str ("\n#define %swrap() (/*CONSTCOND*/1)\n", prefix);
		}
		outn ("#define YY_SKIP_YYWRAP");
	}

	if (ddebug)
		outn ("\n#define FLEX_DEBUG");

	OUT_BEGIN_CODE ();
	outn ("typedef flex_uint8_t YY_CHAR;");
	OUT_END_CODE ();

	if (C_plus_plus) {
		outn ("#define yytext_ptr yytext");

		if (interactive)
			outn ("#define YY_INTERACTIVE");
	}

	else {



		OUT_BEGIN_CODE ();
		/* In reentrant scanner, stdinit is handled in flex.skl. */
		if (do_stdinit) {
			if (reentrant){
                outn ("#ifdef VMS");
                outn ("#ifdef __VMS_POSIX");
                outn ("#define YY_STDINIT");
                outn ("#endif");
                outn ("#else");
                outn ("#define YY_STDINIT");
                outn ("#endif");
            }

			outn ("#ifdef VMS");
			outn ("#ifndef __VMS_POSIX");
			outn (yy_nostdinit);
			outn ("#else");
			outn (yy_stdinit);
			outn ("#endif");
			outn ("#else");
			outn (yy_stdinit);
			outn ("#endif");
		}

		else {
			if(!reentrant)
				outn (yy_nostdinit);
		}
		OUT_END_CODE ();
	}

	OUT_BEGIN_CODE ();
	if (fullspd)
		outn ("typedef const struct yy_trans_info *yy_state_type;");
	else if (!C_plus_plus)
		outn ("typedef int yy_state_type;");
	OUT_END_CODE ();

	if (lex_compat)
		outn ("#define YY_FLEX_LEX_COMPAT");

	if (!C_plus_plus && !reentrant) {
		outn ("extern int yylineno;");
		OUT_BEGIN_CODE ();
		outn ("int yylineno = 1;");
		OUT_END_CODE ();
	}

	if (C_plus_plus) {
		outn ("\n#include <FlexLexer.h>");

		if (!do_yywrap) {
			outn("\nint yyFlexLexer::yywrap() { return 1; }");
		}

		if (yyclass) {
			outn ("int yyFlexLexer::yylex()");
			outn ("\t{");
			outn ("\tLexerError( \"yyFlexLexer::yylex invoked but %option yyclass used\" );");
			outn ("\treturn 0;");
			outn ("\t}");

			out_str ("\n#define YY_DECL int %s::yylex()\n",
				 yyclass);
		}
	}

	else {

		/* Watch out: yytext_ptr is a variable when yytext is an array,
		 * but it's a macro when yytext is a pointer.
		 */
		if (yytext_is_array) {
			if (!reentrant)
				outn ("extern char yytext[];\n");
		}
		else {
			if (reentrant) {
				outn ("#define yytext_ptr yytext_r");
			}
			else {
				outn ("extern char *yytext;");

				outn("#ifdef yytext_ptr");
				outn("#undef yytext_ptr");
				outn("#endif");
				outn ("#define yytext_ptr yytext");
			}
		}

		if (yyclass)
			flexerror (_
				   ("%option yyclass only meaningful for C++ scanners"));
	}
}

static void cpp_epilog (void)
{
#if 0
	fprintf (header_out,
		 "#ifdef YY_HEADER_EXPORT_START_CONDITIONS\n");
	fprintf (header_out,
		 "/* Beware! Start conditions are not prefixed. */\n");

	/* Special case for "INITIAL" */
	fprintf (header_out,
		 "#undef INITIAL\n#define INITIAL 0\n");
	for (i = 2; i <= lastsc; i++)
		fprintf (header_out, "#define %s %d\n", scname[i], i - 1);
	fprintf (header_out,
		 "#endif /* YY_HEADER_EXPORT_START_CONDITIONS */\n\n");

	/* Kill ALL flex-related macros. This is so the user
	 * can #include more than one generated header file. */
	fprintf (header_out, "#ifndef YY_HEADER_NO_UNDEFS\n");
	fprintf (header_out,
		 "/* Undefine all internal macros, etc., that do no belong in the header. */\n\n");

        {
		const char * undef_list[] = {

                "BEGIN",
                "ECHO",
                "EOB_ACT_CONTINUE_SCAN",
                "EOB_ACT_END_OF_FILE",
                "EOB_ACT_LAST_MATCH",
                "FLEX_SCANNER",
                "REJECT",
                "YYFARGS0",
                "YYFARGS1",
                "YYFARGS2",
                "YYFARGS3",
                "YYLMAX",
                "YYSTATE",
                "YY_AT_BOL",
                "YY_BREAK",
                "YY_BUFFER_EOF_PENDING",
                "YY_BUFFER_NEW",
                "YY_BUFFER_NORMAL",
                "YY_BUF_SIZE",
                "M4_YY_CALL_LAST_ARG",
                "M4_YY_CALL_ONLY_ARG",
                "YY_CURRENT_BUFFER",
                "YY_DECL",
                "M4_YY_DECL_LAST_ARG",
                "M4_YY_DEF_LAST_ARG",
                "M4_YY_DEF_ONLY_ARG",
                "YY_DO_BEFORE_ACTION",
                "YY_END_OF_BUFFER",
                "YY_END_OF_BUFFER_CHAR",
                "YY_EXIT_FAILURE",
                "YY_EXTRA_TYPE",
                "YY_FATAL_ERROR",
                "YY_FLEX_DEFINED_ECHO",
                "YY_FLEX_LEX_COMPAT",
                "YY_FLEX_MAJOR_VERSION",
                "YY_FLEX_MINOR_VERSION",
                "YY_FLEX_SUBMINOR_VERSION",
                "YY_FLUSH_BUFFER",
                "YY_G",
                "YY_INPUT",
                "YY_INTERACTIVE",
                "YY_INT_ALIGNED",
                "YY_LAST_ARG",
                "YY_LESS_LINENO",
                "YY_LEX_ARGS",
                "YY_LEX_DECLARATION",
                "YY_LEX_PROTO",
                "YY_MAIN",
                "YY_MORE_ADJ",
                "YY_NEED_STRLEN",
                "YY_NEW_FILE",
                "YY_NULL",
                "YY_NUM_RULES",
                "YY_ONLY_ARG",
                "YY_PARAMS",
                "YY_PROTO",
                "M4_YY_PROTO_LAST_ARG",
                "M4_YY_PROTO_ONLY_ARG void",
                "YY_READ_BUF_SIZE",
                "YY_REENTRANT",
                "YY_RESTORE_YY_MORE_OFFSET",
                "YY_RULE_SETUP",
                "YY_SC_TO_UI",
                "YY_SKIP_YYWRAP",
                "YY_START",
                "YY_START_STACK_INCR",
                "YY_STATE_EOF",
                "YY_STDINIT",
                "YY_TRAILING_HEAD_MASK",
                "YY_TRAILING_MASK",
                "YY_USER_ACTION",
                "YY_USE_CONST",
                "YY_USE_PROTOS",
                "unput",
                "yyTABLES_NAME",
                "yy_create_buffer",
                "yy_delete_buffer",
                "yy_flex_debug",
                "yy_flush_buffer",
                "yy_init_buffer",
                "yy_load_buffer_state",
                "yy_new_buffer",
                "yy_scan_buffer",
                "yy_scan_bytes",
                "yy_scan_string",
                "yy_set_bol",
                "yy_set_interactive",
                "yy_switch_to_buffer",
				"yypush_buffer_state",
				"yypop_buffer_state",
				"yyensure_buffer_stack",
                "yyalloc",
                "const",
                "yyextra",
                "yyfree",
                "yyget_debug",
                "yyget_extra",
                "yyget_in",
                "yyget_leng",
                "yyget_column",
                "yyget_lineno",
                "yyget_lloc",
                "yyget_lval",
                "yyget_out",
                "yyget_text",
                "yyin",
                "yyleng",
                "yyless",
                "yylex",
                "yylex_destroy",
                "yylex_init",
                "yylex_init_extra",
                "yylineno",
                "yylloc",
                "yylval",
                "yymore",
                "yyout",
                "yyrealloc",
                "yyrestart",
                "yyset_debug",
                "yyset_extra",
                "yyset_in",
                "yyset_column",
                "yyset_lineno",
                "yyset_lloc",
                "yyset_lval",
                "yyset_out",
                "yytables_destroy",
                "yytables_fload",
                "yyterminate",
                "yytext",
                "yytext_ptr",
                "yywrap",

                /* must be null-terminated */
                NULL};


                for (i=0; undef_list[i] != NULL; i++)
                    fprintf (header_out, "#undef %s\n", undef_list[i]);
        }

	/* undef any of the auto-generated symbols. */
	for (i = 0; i < defs_buf.nelts; i++) {

		/* don't undef start conditions */
		if (sclookup (((char **) defs_buf.elts)[i]) > 0)
			continue;
		fprintf (header_out, "#undef %s\n",
			 ((char **) defs_buf.elts)[i]);
	}

	fprintf (header_out,
		 "#endif /* !YY_HEADER_NO_UNDEFS */\n");
	fprintf (header_out, "\n");
	fprintf (header_out, "#undef %sIN_HEADER\n", prefix);
	fprintf (header_out, "#endif /* %sHEADER_H */\n", prefix);

	if (ferror (header_out))
		lerr (_("error creating header file %s"),
			headerfilename);
	fflush (header_out);
	fclose (header_out);
#endif
}

static const char *cpp_yy_int_aligned(void)
{
	return long_align ? "long int" : "short int";
}

static void cpp_comment(const char *txt)
{
	char buf[MAXLINE];
	bool eol;

	strncpy(buf, txt, MAXLINE-1);
	eol = buf[strlen(buf)-1] == '\n';

	if (eol)
		buf[strlen(buf)-1] = '\0';
	out_str("/* [[%s]] */", buf);
	if (eol)
		outc ('\n');
}

static void cpp_ntod(size_t num_full_table_rows)
// Generate nxt table for ntod
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    long_align ? "flex_int32_t" : "flex_int16_t");

	/* Unless -Ca, declare it "short" because it's a real
	 * long-shot that that won't be large enough.
	 */
	if (gentables)
		out_str_dec
			("static const %s yy_nxt[][%d] =\n    {\n",
			 long_align ? "flex_int32_t" : "flex_int16_t",
			 num_full_table_rows);
	else {
		out_dec ("#undef YY_NXT_LOLEN\n#define YY_NXT_LOLEN (%d)\n", num_full_table_rows);
		out_str ("static const %s *yy_nxt =0;\n",
			 long_align ? "flex_int32_t" : "flex_int16_t");
	}
	/* It would be no good trying to return an allocation size here,
	 * as it's not known before table generation is finished.
	 */
}

static size_t cpp_geneoltbl(size_t sz)
// Generate end-of-line-transitions - only used when yylineno tracking is on
{
	outn ("/* Table of booleans, true if rule could match eol. */");
	out_str_dec (cpp_get_int32_decl (), "yy_rule_can_match_eol", sz);
	return sizeof(int32_t) * sz;
}

static void cpp_mkctbl (size_t sz)
// Make full-speed compressed transition table
{
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_TRANSITION, (void**)&yy_transition, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
}

static size_t cpp_gen_yy_trans(size_t sz)
// Table of verify for transition and offset to next state. (sic)
{
	if (gentables)
		out_dec ("static const struct yy_trans_info yy_transition[%d] =\n    {\n", sz);
	else
		outn ("static const struct yy_trans_info *yy_transition = 0;");
	return sz * sizeof(struct yy_trans_info);
}

static size_t cpp_start_state_list(size_t sz)
// Start initializer for table of pointers to start state
{
	/* Table of pointers to start states. */
	if (gentables)
		out_dec ("static const struct yy_trans_info *yy_start_state_list[%d] =\n", sz);
	else
		outn ("static const struct yy_trans_info **yy_start_state_list =0;");
	return sz * sizeof(struct yy_trans_info *);
}

static void cpp_mkftbl(void)
// Make full table
{
	// FIXME: why are there two places this is emitted, here and in cpp_gentabs_accept()?
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    long_align ? "flex_int32_t" : "flex_int16_t");
}

static size_t cpp_genftbl(size_t sz)
{
	out_str_dec (long_align ? cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_accept", sz);
	return sz * (long_align ? sizeof(int32_t) : sizeof(int16_t));
}


static size_t cpp_gentabs_acclist(size_t sz)
// Generate accept list initializer
{
	out_str_dec (long_align ? cpp_get_int32_decl () :
		     cpp_get_int16_decl (), "yy_acclist", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCLIST, (void**)&yy_acclist, sizeof(%s)},\n",
		    long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_accept(size_t sz)
// Generate accept table initializer
{
	out_str_dec (long_align ? cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_accept", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_ACCEPT, (void**)&yy_accept, sizeof(%s)},\n",
		    long_align ? "flex_int32_t" : "flex_int16_t");
	return sz * (long_align ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_meta(size_t sz)
// Generate yy_meta table initializer
{
	out_str_dec (cpp_get_yy_char_decl (), "yy_meta", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_META, (void**)&yy_meta, sizeof(%s)},\n",
		    "YY_CHAR");
	return sz * sizeof(YY_CHAR);
}

static size_t cpp_gentabs_yy_base(size_t sz)
// Generate yy_meta base initializer
{
	out_str_dec ((tblend >= INT16_MAX || long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_base", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_BASE, (void**)&yy_base, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || long_align) ? sizeof(int32_t) : sizeof(int16_t)); 
}

static size_t cpp_gentabs_yy_def(size_t sz)
// Generate yy_def initializer
{
	out_str_dec ((sz >= INT16_MAX || long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (),
		     "yy_def", sz);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_DEF, (void**)&yy_def, sizeof(%s)},\n",
		    (sz >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
	return sz * ((sz >= INT16_MAX || long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_nxt(size_t tblafter)
// Generate yy_nxt initializer
{
	/* Begin generating yy_nxt */
	out_str_dec ((tblafter >= INT16_MAX || long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_nxt",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NXT, (void**)&yy_nxt, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_gentabs_yy_chk(size_t tblafter)
// Generate yy_chk initializer
{
	out_str_dec ((tblafter >= INT16_MAX || long_align) ?
		     cpp_get_int32_decl () : cpp_get_int16_decl (), "yy_chk",
		     tblafter);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_CHK, (void**)&yy_chk, sizeof(%s)},\n",
		    (tblafter >= INT16_MAX
		     || long_align) ? "flex_int32_t" : "flex_int16_t");
	return tblafter * ((tblafter >= INT16_MAX || long_align) ? sizeof(int32_t) : sizeof(int16_t));
}

static size_t cpp_nultrans(int fullspd, size_t afterdfa)
// Generate nulltrans initializer
{
	// Making this a backend method may be overzealous.
	// How many other languages have to sprcial-case NUL
	// because it's a string terminator?
	out_str_dec (cpp_get_state_decl (), "yy_NUL_trans", afterdfa);
	buf_prints (&yydmap_buf,
		    "\t{YYTD_ID_NUL_TRANS, (void**)&yy_NUL_trans, sizeof(%s)},\n",
		    (fullspd) ? "struct yy_trans_info*" : "flex_int32_t");
	return afterdfa * (fullspd ? sizeof(struct yy_trans_info *) : sizeof(int32_t));
}

static size_t cpp_debug_header(size_t sz)
{
	out_str_dec (long_align ? cpp_get_int32_decl () :
		     cpp_get_int16_decl (), "yy_rule_linenum",
		     sz);
	return sz * (long_align ? sizeof(int32_t) : sizeof(int16_t));
}


static size_t cpp_genecs(size_t size)
{
	out_str_dec (cpp_get_yy_char_decl (), "yy_ec", csize);
	return sizeof(YY_CHAR) * size; 
}

static const char *cpp_trans_offset_type(int total_table_size)
{
	return (total_table_size >= INT16_MAX || long_align) ?
			"flex_int32_t" : "flex_int16_t";
}

const char *cpp_skel[] = {
#include "cpp-skel.h"
    0,
};

/* This backend is only accessed through this method table */
struct flex_backend_t cpp_backend = {
	.suffix = cpp_suffix,
	.prolog = cpp_prolog,
	.skel = cpp_skel,
	.epilog = cpp_epilog,
	.yy_int_aligned = cpp_yy_int_aligned,
	.trace_fmt = "#line %d \"%s\"\n",
	.int_define_fmt = "#define %s %d\n",
	.string_define_fmt = "#define %s %s\n",
	.table_opener = "    {",
	.table_continuation = "    },\n",
	.table_closer = "    };\n",
	.dyad = " {%4d,%4d },",
	.comment = cpp_comment,
	.ntod = cpp_ntod,
	.geneoltbl = cpp_geneoltbl,
	.mkctbl = cpp_mkctbl,
	.gen_yy_trans = cpp_gen_yy_trans,
	.start_state_list = cpp_start_state_list,
	.state_entry_fmt = "    &yy_transition[%d],\n",
	.mkftbl = cpp_mkftbl,
	.genftbl = cpp_genftbl,
	.gentabs_acclist = cpp_gentabs_acclist,
	.gentabs_accept = cpp_gentabs_accept,
	.gentabs_yy_meta = cpp_gentabs_yy_meta,
	.gentabs_yy_base = cpp_gentabs_yy_base,
	.gentabs_yy_def = cpp_gentabs_yy_def,
	.gentabs_yy_nxt = cpp_gentabs_yy_nxt,
	.gentabs_yy_chk = cpp_gentabs_yy_chk,
	.genecs = cpp_genecs,
	.nultrans = cpp_nultrans,
	.trans_offset_type = cpp_trans_offset_type,
	.debug_header = cpp_debug_header,
	.caseprefix = "case ",
	.fallthrough = NULL,
	.endcase = "yyterminate();",
	.c_like = 1,
};
