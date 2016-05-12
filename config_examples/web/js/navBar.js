// Navigation bar helper functions used for a single source bootstrap menu.
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2016 Grodans Paradis AB (Paradise of the Frog)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Alternative licenses for VSCP & Friends may be arranged by contacting
// Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

/*jshint bitwise: false */

/** Create the root namespace and making sure we're not overwriting it
 * @namespace navBarMenu
 */
var navBarMenu = navBarMenu || {};

/* ---------------------------------------------------------------------- */

/**
 * Show a menu in bootstrap style.
 *
 * @param {object} menu  Menu structure
 */
navBarMenu.show = function( id, menu ) {

    var index       = 0;
    var navigation  = "";
    
    navigation = '<ul class="nav navbar-nav">';
    
    for(index = 0; index < menu.length; ++index ) {
    
        if ( "undefined" !== typeof menu[ index ].dropDown ) {
            navigation += navBarMenu._dropDown( menu[ index ] );
        }
        else {
            navigation += '<li><a href="' + menu[ index ].url + '">' + menu[ index ].title + '</a></li>';
        }
    }
    
    $( "#" + id ).html( navigation );

};

/**
 * Create a drop-down menu and return it.
 * @private
 *
 * @param {object} menu  Menu structure
 *
 * @return {string} HTML drop-down menu part
 */
navBarMenu._dropDown = function( menu ) {
    
    var index       = 0;
    var navigation  = "";
    
    navigation += '<li class="dropdown">';
    navigation += '<a class="dropdown-toggle" data-toggle="dropdown" href="' + menu.url + '">' + menu.title + '';
    navigation += '<span class="caret"></span></a>';
    navigation += '<ul class="dropdown-menu">';
                        
    for(index = 0; index < menu.dropDown.length; ++index ) {
    
        if ( "undefined" !== typeof menu.dropDown[ index ].dropDown ) {
            navigation += navBarMenu._dropDown( menu.dropDown[ index ] );
        }
        else {
            navigation += '<li><a href="' + menu.dropDown[ index ].url + '">' + menu.dropDown[ index ].title + '</a></li>';
        }
    }
    
    navigation += '</ul>';
    navigation += '</li>';
    
    return navigation;
};
