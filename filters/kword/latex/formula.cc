/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#include <stdlib.h>		/* for atoi function */
#include <kdebug.h>		/* for kdDebug() stream */
#include <qstack.h>		/* for getFormula() */
#include "formula.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Formula::Formula()
{
	_left              = 0;
	_right             = 0;
	_top               = 0;
	_bottom            = 0;
	_runaround         = TA_NONE;
	_runaroundGap      = 0;
	_autoCreate        = TC_EXTEND;
	_newFrameBehaviour = TF_RECONNECT;

}

/*******************************************/
/* analyse                                 */
/*******************************************/
void Formula::analyse(const Markup * balise_initiale)
{
	Token* savedToken = 0;
	Markup* balise    = 0;

	/* MARKUP TYPE : FRAMESET INFO = TEXTE, ENTETE CONNUE */
	
	/* Parameters Analyse */
	Element::analyse(balise_initiale);

	kdDebug() << "FRAME ANALYSE (Formula)" << endl;

	/* Chlidren markups Analyse */
	savedToken = enterTokenChild(balise_initiale);
	while((balise = getNextMarkup()) != 0)
	{

		if(strcmp(balise->token.zText, "FRAME")== 0)
		{
			analyseParamFrame(balise);
		}
		else if(strcmp(balise->token.zText, "FORMULA")== 0)
		{
			Token* p = balise->pContent;
			getFormula(p, 0);
			kdDebug() << _formula << endl;
		}
		
	}
	kdDebug() << "END OF A FRAME" << endl;
}

/*******************************************/
/* getFormula                              */
/*******************************************/
/* Get back the xml markup tree.           */
/*******************************************/
void Formula::getFormula(Token* p, int indent)
{
	Markup* pM = 0;
	Arg*  pArg = 0;

	while( p )
	{
		switch( p->eType )
		{
			case TT_Word:
				_formula = _formula + QString(p->zText) + " ";
				//printf("%*s\"%s\"\n", indent, "", p->zText);
				break;
			case TT_Space:
				_formula = _formula + p->zText;
				//printf("%*s\"%s\"\n", indent, "", p->zText);
				break;
			case TT_EOL:
				_formula = _formula + "\n";
				//printf("%*s\n", indent, "");
				break;
			case TT_Markup:
				_formula = _formula + "<" + p->zText;
				//printf("%*s<%s", indent, "", p->zText); // The tag name
				pM = (Markup*)p;
				for(pArg = pM->pArg; pArg; pArg=pArg->pNext)
				{ // The attributes
					_formula = _formula + " " + pArg->zName + "=\"" + pArg->zValue + "\"";
					//printf(" %s=\"%s\"", pArg->zName, pArg->zValue);
				}
				if(pM->pContent == 0)
					_formula = _formula + "/>";
				else
				{
					_formula = _formula + ">";
					//printf(">\n");
					getFormula(pM->pContent, indent+3); // The child elements
					_formula = _formula + "</" + p->zText + ">";
				}
				break;
			default:
				kdError() << "Can't happen" << endl;
				break;
		}
		p = p->pNext;
	}
}

/*******************************************/
/* analyseParamFrame                       */
/*******************************************/
void Formula::analyseParamFrame(const Markup *balise)
{
	/*<FRAME left="28" top="42" right="566" bottom="798" runaround="1" />*/
	Arg *arg = 0;

	for(arg= balise->pArg; arg; arg= arg->pNext)
	{
		kdDebug() << "PARAM " << arg->zName << endl;
		if(strcmp(arg->zName, "LEFT")== 0)
		{
			_left = atoi(arg->zValue);
		}
		else if(strcmp(arg->zName, "TOP")== 0)
		{
			_top = atoi(arg->zValue);
		}
		else if(strcmp(arg->zName, "RIGHT")== 0)
		{
			_right = atoi(arg->zValue);
		}
		else if(strcmp(arg->zName, "BOTTOM")== 0)
		{
			_bottom = atoi(arg->zValue);
		}
		else if(strcmp(arg->zName, "RUNAROUND")== 0)
		{
			setRunAround(atoi(arg->zValue));
		}
		else if(strcmp(arg->zName, "RUNAROUNDGAP")== 0)
		{
			setAroundGap(atoi(arg->zValue));
		}
		else if(strcmp(arg->zName, "AUTOCREATENEWFRAME")== 0)
		{
			setAutoCreate(atoi(arg->zValue));
		}
		else if(strcmp(arg->zName, "NEWFRAMEBEHAVIOUR")== 0)
		{
			setNewFrame(atoi(arg->zValue));
		}
		else if(strcmp(arg->zName, "SHEETSIDE")== 0)
		{
			setSheetSide(atoi(arg->zValue));
		}
	}
}

/*******************************************/
/* generate                                */
/*******************************************/
void Formula::generate(QTextStream &out)
{
	kdDebug() << "FORMULA GENERATION" << endl;
	out << _formula << endl;
}

