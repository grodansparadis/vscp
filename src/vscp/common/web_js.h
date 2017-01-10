// web_js.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 
// Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


#if !defined(WEBJS__INCLUDED_)
#define WEBJS__INCLUDED_


//#define WEB_COMMON_JS "<script type=\"text/javascript\">sfHover=function(){var sfEls=document.getElementById(\"nav\").getElementsByTagName(\"LI\");for(var i=0;i<sfEls.length;i++){sfEls[i].onmouseover=function(){this.className+=\" sfhover\";} sfEls[i].onmouseout=function(){this.className=this.className.replace(new RegExp(\" sfhover\\b\"),\"\");}}} if(window.attachEvent){window.attachEvent(\"onload\",sfHover);} function ChangeColor(tableRow,highLight){alert('---');if(highLight){tableRow.style.backgroundColor='#dcfac9';} else{tableRow.style.backgroundColor='white';}} function DoNav(theUrl){document.location.href=theUrl;}</script>"

#define WEB_COMMON_JS "<script type=\"text/javascript\">sfHover=function(){var b=document.getElementById(\"nav\").getElementsByTagName(\"LI\");for(var a=0;a<b.length;a++){b[a].onmouseover=function(){this.className+=\" sfhover\"};b[a].onmouseout=function(){this.className=this.className.replace(new RegExp(\" sfhover\\\\b\"),\"\")}}};if(window.attachEvent){window.attachEvent(\"onload\",sfHover)}function ChangeColor(a,b){if(b){a.style.backgroundColor=\"#dcfac9\"}else{a.style.backgroundColor=\"white\"}}function DoNav(a){document.location.href=a};</script>"


#endif