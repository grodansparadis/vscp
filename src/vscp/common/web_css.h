// web_css.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


#if !defined(WEBCSS__INCLUDED_)
#define WEBCSS__INCLUDED_


#define WEB_STYLE_START "<style type=\"text/css\">"
#define WEB_STYLE_END	"</style>"

#define WEB_COMMON_CSS "body{background-color:#f0f0f0;padding:0;margin:0;"\
"background:#fff;color:#000;font-family:arial,verdana,sans-serif;"\
"font-size:85%}#content{width:700px;margin-left:auto;margin-right:auto;"\
"border:0 solid;clear:left}#content h4{background:#fff;padding-left:0;"\
"padding-top:0;padding-bottom:0;margin-top:5px;margin-bottom:5px;"\
"font-size:110%;color:#a04040}#content h5{background:#fff;padding-left:0;"\
"padding-top:0;padding-bottom:0;margin-top:5px;margin-bottom:5px;"\
"font-size:110%;color:#c00}#header{margin:0;padding-left:0;width:500px;"\
"color:#c00;background:#fff}* html #header{margin:-60px 5px 5px 5px;"\
"padding-left:5px;width:475px;color:#c00;background:#fff}#body{border-top:1px "\
"solid silver}#body a{color:#c00;background:#fff;text-decoration:none}#footer"\
"{text-align:right;font-size:75%;color:silver;background:#fff;border-top:1px "\
"solid silver}#footer a{color:silver;background:#fff;text-decoration:none}"\
"#links{float:left;width:150px;margin:5px}#links a{color:#c00;background:#fff;"\
"text-decoration:none;font-size:120%}#links p{color:silver;background:#fff;"\
"font-size:85%}#main{margin:5px 5px 5px 160px;padding:15px;border-left:1px "\
"solid silver}#main h2{background:#fff;padding-left:22px;font-size:110%;color:"\
"#c00}#optiontext{color:#888;font-weight:normal}#small{padding-left:0;font-size:"\
"70%;color:#202020}#small b{font-weight:700;font-size:115%;color:#202070}#tdcenter"\
"{float:center;text-align:center}#tdguid{font-family:Courier New,Courier,monospace}"\
"blockquote{border:1px solid #c00;padding:10px}code{white-space:pre}table{width:100%;"\
"border:1px solid #d0d0ff;border-spacing:0}th,td{text-align:left;vertical-align:"\
"middle;border:1px solid #d0d0ff;border-collapse:collapse;padding:.3em;caption-side:"\
"bottom}th{background:#eee}table.invisable{width:100%;border:0 solid #d0d0ff;"\
"border-spacing:0}table.invisable tr{width:100%;border:0 solid #d0d0ff;"\
"border-spacing:0}table.invisable td{text-align:left;vertical-align:middle;"\
"border:0 solid #00f;border-collapse:collapse;padding:.3em;caption-side:bottom}"\
"th.invisable{background:#eee}#nav,#nav ul{float:left;width:100%;list-style:none;"\
"line-height:1;background:whitesmoke;font-weight:bold;padding:0;border:solid "\
"#c0c0c0;border-width:1px 0;margin:0 0 2em 0}#nav a{display:block;width:10em;"\
"width:6em;color:#000;text-decoration:none;padding:.25em 2em}#nav a.daddy{"\
"background:url(rightarrow2.gif) center right no-repeat}#nav li{float:"\
"left;padding:0;width:10em}#nav li ul{position:absolute;left:-999em;height:"\
"auto;width:14.4em;width:13.9em;font-weight:normal;border-width:.25em;margin:0}"\
"#nav li li{padding-right:1em;width:13em}#nav li ul a{width:13em;width:9em}#nav "\
"li ul ul{margin:-1.75em 0 0 14em}#nav li:hover ul ul,#nav li:hover ul ul ul,#nav "\
"li.sfhover ul ul,#nav li.sfhover ul ul ul{left:-999em}#nav li:hover ul,#nav li "\
"li:hover ul,#nav li li li:hover ul,#nav li.sfhover ul,#nav li li.sfhover ul,#nav "\
"li li li.sfhover ul{left:auto}#nav li:hover,#nav "\
"li.sfhover{background:#c0c0c0}#tbalign td{vertical-align:top}"

/*
body {
    background-color:#f0f0f0;
    padding:0;margin:0;
    background:#fff;
    color:#000;
    font-family:arial,verdana,sans-serif;
    font-size:85%
 }
 
#content {
    width:700px;
    margin-left:auto;
    margin-right:auto;
    border:0 solid;
    clear:left
}
 
#content h4 {
    background:#fff;
    padding-left:0;
    padding-top:0;
    padding-bottom:0;
    margin-top:5px;
    margin-bottom:5px;
    font-size:110%;
    color:#a04040
}
 
#content h5 {
    background:#fff;
    padding-left:0;
    padding-top:0;
    padding-bottom:0margin-top:5px;
    margin-bottom:5px;
    font-size:110%;
    color:#c00
}

#header {
    margin:0;
    padding-left:0;
    width:500px;
    color:#c00;
    background:#fff
} 

html #header{
    margin:-60px 5px 5px 5px;
    padding-left:5px;
    width:475px;
    color:#c00;
    background:#fff
} 

#body { 
    border-top:1px solid silver
}

#body a { 
    color:#c00;
    background:#fff;
    text-decoration:none
}

#footer {
    text-align:right;
    font-size:75%;
    color:silver;
    background:#fff;
    border-top:1px solid silver
}

#footer a { 
    color:silver;
    background:#fff;
    text-decoration:none
}
  
#links {
    float:left;
    width:150px;
    margin:5px
}
 
#links a {
    color:#c00;
    background:#fff;
    text-decoration:none;
    font-size:120%
}
  
#links p {
    color:silver;
    background:#fff;
    font-size:85%
}
  
#main { 
    margin:5px 5px 5px 160px;
    padding:15px;
    border-left:1px solid silver
}

#main h2 {
    background:#fff;
    padding-left:22px;
    font-size:110%;
    color: #c00
}

#optiontext {
    color:#888;
    font-weight:normal
}
 
#small {
    padding-left:0;
    font-size: 70%;
    color:#202020
} 
  
#small b
{
    font-weight:700;
    font-size:115%;
    color:#202070
}
  
#tdcenter {
    float:center;
    text-align:center
}
 
#tdguid {
    font-family:Courier New,Courier,monospace
}
 
blockquote {
    border:1px solid #c00;
    padding:10px
}
 
code {
    white-space:pre
}
 
table {
    width:100%;
    border:1px solid #d0d0ff;b
    order-spacing:0
 }
  
 th,td {
    text-align:left;
    vertical-align: middle;border:1px solid #d0d0ff;
    border-collapse:collapse;
    padding:.3em;
    caption-side: bottom
 }
 
 th { 
    background:#eee
 }
 
 table.invisable { 
    width:100%;
    border:0 solid #d0d0ff;
    border-spacing:0
 }
 
 table.invisable tr {
    width:100%;
    border:0 solid #d0d0ff;
    border-spacing:0
 } 
  
 table.invisable td {
    text-align:left;
    vertical-align:middle;
    border:0 solid #00f;
    border-collapse:collapse;
    padding:.3em;
    caption-side:bottom
 }
 
th.invisable {
    background:#eee
 } 
  
 #nav,#nav ul {
    float:left;
    width:100%;
    list-style:none;
    line-height:1;
    background:whitesmoke;
    font-weight:bold;
    padding:0;
    border:solid #c0c0c0;
    border-width:1px 0;
    margin:0 0 2em 0
 }
 
 #nav a { 
    display:block;
    width:10em;"
    width:6em;
    color:#000;
    text-decoration:none;
    padding:.25em 2em
 }
 
 #nav a.daddy {
    background:url(rightarrow2.gif) center right no-repeat
 }
  
 #nav li { 
    float: left;
    padding:0;
    width:10em
 }
 
 #nav li ul {
    position:absolute;
    left:-999em;
    height: auto;
    width:14.4em;
    width:13.9em;
    font-weight:normal;
    border-width:.25em;
    margin:0
}
 
#nav li li {
    padding-right:1em;
    width:13em
}
  
#nav li ul a {
    width:13em;
    width:9em 
 }
  
#nav li ul ul { 
    margin:-1.75em 0 0 14em
 }
  
 #nav li:hover ul ul,
 #nav li:hover ul ul ul,
 #nav li.sfhover ul ul,
 #nav li.sfhover ul ul ul {
    left:-999em
 }
  
 #nav li:hover ul,
 #nav li li:hover ul,
 #nav li li li:hover ul,
 #nav li.sfhover ul,
 #nav li li.sfhover ul,
 #nav li li li.sfhover ul { 
    left:auto
 }
  
 #nav li:hover,
 #nav li.sfhover { 
    background:#c0c0c0
 }
  
 #tbalign td {
    vertical-align:top
 } 
 * 
 */

#endif