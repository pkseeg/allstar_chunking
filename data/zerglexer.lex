%option noyywrap
%option prefix="zerg"

%{

/*
  Copyright (C) 2002  Apuã Paquola - Instituto de Química -
                    Universidade de São Paulo - Brasil

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

  
#include <string.h>
#include "zerg.h"


#define YY_READ_BUF_SIZE 1048576
#define INTERNAL_BUFFER_LENGTH 1048576

#define DESCONTA _zerg_desconto=number_to_move;

static int _zerg_desconto=0;
static char _zerg_internal_buffer[INTERNAL_BUFFER_LENGTH];
static char* _zerg_description_score;
static char* _zerg_description_evalue;
static int _zerg_tail_of_rep_len=0;

//                           0                   10                  20                  30                  40                  50                  60               
static int _zerg_ignore[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static char* _zerg_value;

/* _zerg_blast_type  0:blastn 1:blastp 2:blastx 3:tblastn 4:tblastx */
static int _zerg_blast_type=0;

#define DBEGIN(SC) printf("[%s]\n",yytext); BEGIN(SC);

#define RET(SC,VALUE,CODE) BEGIN(SC);\
 if(!_zerg_ignore[CODE]) {_zerg_value=VALUE; return CODE;}


static void begin_tail_of_report( size_t __sc ) // the argument is here to make it clearer what start condition we switch to, and to make it show up better in searches
{
    _zerg_tail_of_rep_len = 0; _zerg_internal_buffer[0] = 0;
    //
    BEGIN(__sc);
}


static void _zerg_strip_double_spaces()
{
  char *p1, *p2;
  int state;
  p1=yytext;
  p2=_zerg_internal_buffer;
  state=1;
  while(*p1 && p2-_zerg_internal_buffer < INTERNAL_BUFFER_LENGTH-1)
  {
    if(*p1==' ' || *p1=='\t' || *p1=='\n')
    {
      if(state==0)
      {
	*(p2++)=' ';
	state=1;
      }
    }
    else
    {
      *(p2++)=*p1;
      state=0;
    }
    p1++;
  }
  if(state==1 && p2>_zerg_internal_buffer) p2--;
  *p2='\0';
}


static void _zerg_strip_commas()
{
  char *p1, *p2;
  p1=yytext;
  p2=_zerg_internal_buffer;
  while(*p1 && p2-_zerg_internal_buffer < INTERNAL_BUFFER_LENGTH-1)
  { 
    if(*p1!=',') *(p2++)=*p1;
    p1++;
  }
  *p2='\0';
}

static void _zerg_fix_evalue()
{
  char *p1, *p2;
  p1=yytext;
  p2=_zerg_internal_buffer;
  if(*p1=='e')
    *(p2++)='1';
  while(*p1 && p2-_zerg_internal_buffer < INTERNAL_BUFFER_LENGTH-2)
    *(p2++)=*(p1++);
  *p2='\0';
}


/*  cat a.lex | perl -ne 'print "%x $1\n" if /^<(\S+?)>/' | sort | uniq */
   
%}

%x s_alignment_length
%x s_alignment_length_2
%x s_blast_version
%x s_database
%x s_description_annotation
%x s_description_evalue
%x s_description_score
%x s_descriptions
%x s_evalue
%x s_evalue_2
%x s_hsp_method
%x s_gaps
%x s_gaps_2
%x s_hsp
%x s_identities
%x s_identities_2
%x s_percent_identities
%x s_percent_identities_2
%x s_pre_end
%x s_pre_hits
%x s_pre_hits_2
%x s_round
%x s_round_seq_new
%x s_query_1
%x s_query_2
%x s_query_3
%x s_query_4
%x s_query_5
%x s_query_6
%x s_query_end
%x s_query_end_2
%x s_query_orientation
%x s_query_orientation_2
%x s_query_start
%x s_query_start_2
%x s_query_ali
%x s_query_ali_2
%x s_subject_ali
%x s_subject_ali_2
%x s_reference
%x s_score
%x s_score_2
%x s_score_parentheses
%x s_score_parentheses_2
%x s_searching
%x s_subject_annotation
%x s_subject_annotation_2
%x s_subject_end
%x s_subject_end_2
%x s_subject_length
%x s_subject_length_2
%x s_subject_name
%x s_subject_orientation
%x s_subject_orientation_2
%x s_subject_start
%x s_subject_start_2
%x s_tail_of_report
%x s_positives
%x s_positives_2
%x s_positives_3
%x s_frame
%x s_frame_2
%x s_frame_3
%x s_frame_4
%x s_frame_5
%x s_gaps_3
%x s_pre_subject_name

%%

<INITIAL>T?BLAST.+         {
  
  if(!strncmp(yytext,"BLASTN",6)) _zerg_blast_type=0;
  else if(!strncmp(yytext,"BLASTP",6)) _zerg_blast_type=1;
  else if(!strncmp(yytext,"BLASTX",6)) _zerg_blast_type=2;
  else if(!strncmp(yytext,"TBLASTN",7)) _zerg_blast_type=3;
  else if(!strncmp(yytext,"TBLASTX",7)) _zerg_blast_type=4;

  RET(s_blast_version,yytext,BLAST_VERSION);
  }


<s_blast_version>\n+                  BEGIN(s_reference);
<s_reference,s_query_1>Reference(.+\n)+\n+      RET(s_query_1,yytext,REFERENCE);
<s_query_1>"Query= "      BEGIN(s_query_2);
<s_query_2>[^ \t\n]+        RET(s_query_3,yytext,QUERY_NAME);

<s_query_3>.*\n(.+\n)*\n?/"         ("  {
  /* you may want to add this variable trailing context for safety:
     [0-9,]+" letters)" */
  if(!_zerg_ignore[QUERY_ANNOTATION])
    _zerg_strip_double_spaces();
  RET(s_query_4,_zerg_internal_buffer,QUERY_ANNOTATION);
  }
  

<s_query_4>"         ("   BEGIN(s_query_5);


<s_query_5>[0-9,]+    {
  if(!_zerg_ignore[QUERY_LENGTH])
    _zerg_strip_commas();
  RET(s_query_6,_zerg_internal_buffer,QUERY_LENGTH);
                       }

<s_query_6>.+\n\n      BEGIN(s_database);
<s_database>Database:(.+\n)+\n RET(s_searching,yytext,DATABASE);
<s_searching,s_hsp>Searching"."*done\n+  RET(s_pre_hits,yytext,SEARCHING);
<s_pre_hits>.+"No hits found".+\n\n  RET(s_hsp,yytext,NOHITS);
<s_pre_hits>^"Results from round"" "* BEGIN(s_round);
<s_round>[[:digit:]]+	 RET(s_pre_hits,yytext,ROUND_NUMBER);
<s_pre_hits>[ \t]+Score[[:space:]]+E.*\n.+\n    BEGIN(s_pre_hits_2);
<s_pre_hits_2>^\n      BEGIN(s_descriptions);
<s_pre_hits_2>^"Sequences used in model and found again:"\n     RET(s_pre_hits_2,yytext,ROUND_SEQ_FOUND)

<s_description_annotation>.+\n {
  
  char *evalue, *score, *annotation;
  int evalue_length, score_length, annotation_length, i;
  char *pp, *pp0;
  
  if(!_zerg_ignore[DESCRIPTION_ANNOTATION] ||
     !_zerg_ignore[DESCRIPTION_SCORE] ||
     !_zerg_ignore[DESCRIPTION_EVALUE])
    {
      pp=yytext+yyleng-1;
      
      while(pp>=yytext && (*pp==' ' || *pp=='\t' || *pp=='\n' || *pp=='\0'))
	pp--;

      pp0=pp;
      while(pp>=yytext && !(*pp==' ' || *pp=='\t' || *pp=='\n' || *pp=='\0'))
	pp--;
      evalue_length=pp0-pp;
      evalue=pp+1;
      
      while(pp>=yytext && (*pp==' ' || *pp=='\t' || *pp=='\n' || *pp=='\0'))
	pp--;

      pp0=pp;
      while(pp>=yytext && !(*pp==' ' || *pp=='\t' || *pp=='\n' || *pp=='\0'))
	pp--;
      score_length=pp0-pp;
      score=pp+1;

      while(pp>=yytext && (*pp==' ' || *pp=='\t' || *pp=='\n' || *pp=='\0'))
	pp--;
      
      annotation=yytext;
      while(annotation<=pp && (*annotation==' ' || *annotation=='\t' ||
			       *annotation=='\n' || *annotation=='\0'))
	annotation++;
      
      annotation_length=pp-annotation+1;
      
      if(annotation_length+score_length+evalue_length+4 > INTERNAL_BUFFER_LENGTH)
	{
    _zerg_internal_buffer[INTERNAL_BUFFER_LENGTH-1] = 0;
	  fprintf(stderr, "zerg buffer overflow: '%s'\n", _zerg_internal_buffer);
	  return BUFFER_OVERFLOW;
	}

      pp=_zerg_internal_buffer;
      for(i=0; i<annotation_length; i++)
	*(pp++)=annotation[i];
      *(pp++)='\0';

      _zerg_description_score=pp;
      for(i=0; i<score_length; i++)
	*(pp++)=score[i];
      *(pp++)='\0';
  
      _zerg_description_evalue=pp;
      if(evalue[0]=='e')
	*(pp++)='1';
      for(i=0; i<evalue_length; i++)
	*(pp++)=evalue[i];
      *(pp++)='\0';
    }

  unput('~');
  unput('~');
  RET(s_description_score,_zerg_internal_buffer,DESCRIPTION_ANNOTATION);
  }

<s_description_score>"~"        RET(s_description_evalue,_zerg_description_score,DESCRIPTION_SCORE);
<s_description_evalue>"~"       RET(s_descriptions,_zerg_description_evalue,DESCRIPTION_EVALUE);

<s_pre_hits_2,s_descriptions>\n"Sequences not found previously or not previously below threshold:"\n\n     RET(s_descriptions,yytext,ROUND_SEQ_NEW);
<s_descriptions>[^ \t\n>]+      RET(s_description_annotation,yytext,DESCRIPTION_HITNAME);
<s_descriptions>\n              BEGIN(s_pre_hits);

<s_descriptions,s_pre_hits>\n"CONVERGED!"\n   RET(s_pre_hits,yytext,CONVERGED);
<s_descriptions,s_pre_hits>>                  BEGIN(s_subject_name);

<s_hsp>>                  BEGIN(s_subject_name);

<s_pre_subject_name>"~" RET(s_subject_name,_zerg_internal_buffer+32,SUBJECT_END);


  

<s_subject_name>[^ \t\n]+            RET(s_subject_annotation,yytext,SUBJECT_NAME);
<s_subject_annotation>.*\n(.+\n)*/"          Length = " {
  if(!_zerg_ignore[SUBJECT_ANNOTATION])
    _zerg_strip_double_spaces();
  RET(s_subject_annotation_2,_zerg_internal_buffer,SUBJECT_ANNOTATION);
 }
<s_subject_annotation_2>"          Length = "  BEGIN(s_subject_length);

<s_subject_length>[0-9,]+     {
  if(!_zerg_ignore[SUBJECT_LENGTH])
    _zerg_strip_commas();
  RET(s_subject_length_2,_zerg_internal_buffer,SUBJECT_LENGTH);
                              }

<s_subject_length_2>\n+         BEGIN(s_hsp);

<s_hsp>" Score ="" "+           BEGIN(s_score);

<s_score>[^ \t\n]+              RET(s_score_2,yytext,SCORE_BITS);
<s_score_2>" bits ("            BEGIN(s_score_parentheses);
<s_score_parentheses>[0-9]+            RET(s_score_parentheses_2,yytext,SCORE);
<s_score_parentheses_2>"), Expect"(\([0-9]+\))?" ="" "+  BEGIN(s_evalue);
<s_evalue>[^ \t\n,]+             {
     _zerg_fix_evalue();
     RET(s_evalue_2,_zerg_internal_buffer,EVALUE);
                                }

<s_evalue_2>",   Method:"[ ]*	BEGIN(s_hsp_method);
<s_evalue_2>\n" Identities = "  BEGIN(s_identities);
<s_hsp_method>[^\n]+		RET(s_evalue_2,yytext,HSP_METHOD);
<s_identities>[0-9]+            RET(s_identities_2,yytext,IDENTITIES);
<s_identities_2>"/"             BEGIN(s_alignment_length);
<s_alignment_length>[0-9]+      RET(s_alignment_length_2,yytext,ALIGNMENT_LENGTH);
<s_alignment_length_2>" ("      BEGIN(s_percent_identities);
<s_percent_identities>[0-9]+    RET(s_percent_identities_2,yytext,PERCENT_IDENTITIES);
<s_percent_identities_2>"%), Gaps = " BEGIN(s_gaps);
<s_gaps>[0-9]+                  RET(s_gaps_2,yytext,GAPS);
<s_gaps_2>"/"[0-9]+" ("[0-9]+"%)"\n BEGIN(s_gaps_3);
<s_gaps_3>" Strand = " BEGIN(s_query_orientation);
<s_gaps_3>" Frame = " BEGIN(s_frame);

<s_percent_identities_2>"%)"\n  {
   _zerg_internal_buffer[0]='0';
   _zerg_internal_buffer[1]='\0';
   RET(s_gaps_3,_zerg_internal_buffer,GAPS);
                                }

<s_percent_identities_2>"%), Positives = " BEGIN(s_positives);
<s_positives>[0-9]+             RET(s_positives_2,yytext,POSITIVES);
<s_positives_2>"/"[0-9]+" ("    BEGIN(s_positives_3);
<s_positives_3>[0-9]+           RET(s_percent_identities_2,yytext,PERCENT_POSITIVES);

<s_frame>[+-][0-9]+      {
  if(_zerg_blast_type==2) /* blastx */
  {
    RET(s_frame_4,yytext,QUERY_FRAME);
  }
  else if(_zerg_blast_type==3) /* tblastn */
  {
    RET(s_frame_4,yytext,SUBJECT_FRAME);
  }
  else if(_zerg_blast_type==4) /* tblastx */
  {
    RET(s_frame_2,yytext,QUERY_FRAME);
  }
}

<s_frame_2>" / "         BEGIN(s_frame_3);
<s_frame_3>[+-][0-9]+    RET(s_frame_4,yytext,SUBJECT_FRAME);
<s_frame_4>\n            BEGIN(s_frame_5);

<s_query_orientation>[^ \t\n]+  RET(s_query_orientation_2,yytext,QUERY_ORIENTATION);
<s_query_orientation_2>" / "    BEGIN(s_subject_orientation);
<s_subject_orientation>[^ \t\n]+ RET(s_subject_orientation_2,yytext,SUBJECT_ORIENTATION);
<s_gaps_3,s_subject_orientation_2,s_frame_5>\n+    BEGIN(s_hsp);

<s_hsp>(" "*\n)?"Query:"" "+       BEGIN(s_query_start);

<s_query_start>[0-9]+    RET(s_query_start_2,yytext,QUERY_START);
<s_query_start_2>[ ]+           BEGIN(s_query_ali);
<s_query_ali>[[:alpha:]-]+      RET(s_query_ali_2,yytext,QUERY_ALI);
<s_query_ali_2>[ ]+      BEGIN(s_query_end);
<s_query_end>[0-9]+      RET(s_query_end_2,yytext,QUERY_END);

%{
/* handle this:
Query: 0   -------                                                      

Sbjct: 897 TPGAYGG                                                      903 

   handle this:
[...]
Query: 0                                                               
                                                                       
Sbjct: 28                                                               28
[...]
*/
%}
<s_query_ali,s_query_end,s_query_end_2>\n.+\n"Sbjct:"" "+ BEGIN(s_subject_start);

<s_subject_start>[0-9]+         RET(s_subject_start_2,yytext,SUBJECT_START);
<s_subject_start_2>[ ]+         BEGIN(s_subject_ali);
<s_subject_ali>[[:alpha:]-]+    RET(s_subject_ali_2,yytext,SUBJECT_ALI);
<s_subject_ali_2>[ ]+   BEGIN(s_subject_end);
%{
/* handle this:
[...]
Query: 0                                                               
                                                                       
Sbjct: 28                                                               28
[...]
*/
%}
<s_subject_ali,s_subject_end>[0-9]+           RET(s_subject_end_2,yytext,SUBJECT_END);
<s_subject_end_2>\n+            BEGIN(s_hsp);


<s_pre_hits,s_pre_end>[ \t\n]

   
<s_pre_hits,s_pre_end>Database: {
    yyless(0);
    begin_tail_of_report(s_tail_of_report);
  }

<s_tail_of_report>T?BLAST.         {
    yyless(0); 
    RET(s_pre_end,_zerg_internal_buffer,TAIL_OF_REPORT);
  }

<s_pre_hits,s_pre_end,s_hsp>T?BLAST.         {
       yyless(0); 
       RET(INITIAL,"",END_OF_REPORT);
 }

<s_hsp>"  Database:"  {
    yyless(0);
    begin_tail_of_report(s_tail_of_report);
  }

<s_tail_of_report>.|\n {
    if( _zerg_tail_of_rep_len < INTERNAL_BUFFER_LENGTH-1 )
    {
      _zerg_internal_buffer[_zerg_tail_of_rep_len++] = *yytext;
      _zerg_internal_buffer[_zerg_tail_of_rep_len] = 0;
    }
    else 
      fprintf(stderr, "zerg buffer overflow, can not store '%c'\n", *yytext );
    //BEGIN(s_tail_of_report); // it already is int this start cond
  }

<*>.|\n               RET(INITIAL,yytext,UNMATCHED);

<s_tail_of_report><<EOF>>   {
    RET(INITIAL,_zerg_internal_buffer,TAIL_OF_REPORT);
  }

<INITIAL><<EOF>> {
    _zerg_value=""; return 0;
  }

%%


void zerg_open_file(char* filename)
{
  FILE * fh;

  fh = fopen(filename,"r");

  if( !fh ) zergrestart( stdin );
  else zergrestart( fh );
  BEGIN(INITIAL);
}

void zerg_read_stream(FILE* __stream)
{
  zergrestart( __stream );
  BEGIN(INITIAL);
}

void zerg_close_file()
{
  fclose(zergin);
}


void zerg_ignore(int code)
{
  _zerg_ignore[code]=1;
}

void zerg_ignore_all()
{
  int i;
  for(i=BLAST_VERSION; i<AFTER_LAST_TOKEN; i++)
    _zerg_ignore[i]=1;
}

void zerg_unignore(int code)
{
  _zerg_ignore[code]=0;
}

void zerg_unignore_all()
{
  int i;
  for(i=BLAST_VERSION; i<AFTER_LAST_TOKEN; i++)
    _zerg_ignore[i]=0;
}

int zerg_get_token(int* code, char **value)
{
  *code = zerglex();
  *value = _zerg_value;
  return *code;
}

int zerg_get_token_offset()
{
  int pos;
  pos=ftell(YY_CURRENT_BUFFER->yy_input_file);
  if(pos==-1)
    return -1;
  else
    return pos - YY_CURRENT_BUFFER->yy_n_chars + yytext -YY_CURRENT_BUFFER->yy_ch_buf - _zerg_desconto;
}

// vim:et:ts=2:
