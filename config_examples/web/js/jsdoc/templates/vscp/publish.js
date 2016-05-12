//
// JSDoc template to generate dokuwiki markup especially for VSCP wiki
//
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2015 Paradise of the Frog/Grodans Paradis AB
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
'use strict';

function graft(parentNode, childNodes, parentLongname, parentName) {
    childNodes
    .filter(function (element) {
        return (element.memberof === parentLongname);
    })
    .forEach(function (element, index) {
        var i,
            len;

        if (element.kind === 'namespace') {
            if (!parentNode.namespaces) {
                parentNode.namespaces = [];
            }

            var thisNamespace = {
                'name': element.name,
                'description': element.description || '',
                'access': element.access || '',
                'virtual': !!element.virtual
            };

            parentNode.namespaces.push(thisNamespace);

            graft(thisNamespace, childNodes, element.longname, element.name);
        }
        else if (element.kind === 'mixin') {
            if (!parentNode.mixins) {
                parentNode.mixins = [];
            }

            var thisMixin = {
                'name': element.name,
                'description': element.description || '',
                'access': element.access || '',
                'virtual': !!element.virtual
            };

            parentNode.mixins.push(thisMixin);

            graft(thisMixin, childNodes, element.longname, element.name);
        }
        else if (element.kind === 'function') {
            if (!parentNode.functions) {
                parentNode.functions = [];
            }

            var thisFunction = {
                'name': element.name,
                'access': element.access || '',
                'virtual': !!element.virtual,
                'description': element.description || '',
                'parameters': [ ],
                'examples': []
            };

            parentNode.functions.push(thisFunction);

            if (element.returns) {
                thisFunction.returns = {
                    'type': element.returns[0].type? (element.returns[0].type.names.length === 1? element.returns[0].type.names[0] : element.returns[0].type.names) : '',
                    'description': element.returns[0].description || ''
                };
            }

            if (element.examples) {
                for (i = 0, len = element.examples.length; i < len; i++) {
                    thisFunction.examples.push(element.examples[i]);
                }
            }

            if (element.params) {
                for (i = 0, len = element.params.length; i < len; i++) {
                    thisFunction.parameters.push({
                        'name': element.params[i].name,
                        'type': element.params[i].type? (element.params[i].type.names.length === 1? element.params[i].type.names[0] : element.params[i].type.names) : '',
                        'description': element.params[i].description || '',
                        'default': element.params[i].defaultvalue || '',
                        'optional': typeof element.params[i].optional === 'boolean'? element.params[i].optional : '',
                        'nullable': typeof element.params[i].nullable === 'boolean'? element.params[i].nullable : ''
                    });
                }
            }
        }
        else if (element.kind === 'member') {
            if (!parentNode.properties) {
                parentNode.properties = [];
            }
            parentNode.properties.push({
                'name': element.name,
                'access': element.access || '',
                'virtual': !!element.virtual,
                'description': element.description || '',
                'type': element.type? (element.type.length === 1? element.type[0] : element.type) : ''
            });
        }

        else if (element.kind === 'event') {
            if (!parentNode.events) {
                parentNode.events = [];
            }

            var thisEvent = {
                'name': element.name,
                'access': element.access || '',
                'virtual': !!element.virtual,
                'description': element.description || '',
                'parameters': [],
                'examples': []
            };

            parentNode.events.push(thisEvent);

            if (element.returns) {
                thisEvent.returns = {
                    'type': element.returns.type? (element.returns.type.names.length === 1? element.returns.type.names[0] : element.returns.type.names) : '',
                    'description': element.returns.description || ''
                };
            }

            if (element.examples) {
                for (i = 0, len = element.examples.length; i < len; i++) {
                    thisEvent.examples.push(element.examples[i]);
                }
            }

            if (element.params) {
                for (i = 0, len = element.params.length; i < len; i++) {
                    thisEvent.parameters.push({
                        'name': element.params[i].name,
                        'type': element.params[i].type? (element.params[i].type.names.length === 1? element.params[i].type.names[0] : element.params[i].type.names) : '',
                        'description': element.params[i].description || '',
                        'default': element.params[i].defaultvalue || '',
                        'optional': typeof element.params[i].optional === 'boolean'? element.params[i].optional : '',
                        'nullable': typeof element.params[i].nullable === 'boolean'? element.params[i].nullable : ''
                    });
                }
            }
        }
        else if (element.kind === 'class') {
            if (!parentNode.classes) {
                parentNode.classes = [];
            }

            var thisClass = {
                'name': element.name,
                'description': element.classdesc || '',
                'extends': element.augments || [],
                'access': element.access || '',
                'virtual': !!element.virtual,
                'fires': element.fires || '',
                'constructor': {
                    'name': element.name,
                    'description': element.description || '',
                    'parameters': [
                    ],
                    'examples': []
                }
            };

            parentNode.classes.push(thisClass);

            if (element.examples) {
                for (i = 0, len = element.examples.length; i < len; i++) {
                    thisClass.constructor.examples.push(element.examples[i]);
                }
            }

            if (element.params) {
                for (i = 0, len = element.params.length; i < len; i++) {
                    thisClass.constructor.parameters.push({
                        'name': element.params[i].name,
                        'type': element.params[i].type? (element.params[i].type.names.length === 1? element.params[i].type.names[0] : element.params[i].type.names) : '',
                        'description': element.params[i].description || '',
                        'default': element.params[i].defaultvalue || '',
                        'optional': typeof element.params[i].optional === 'boolean'? element.params[i].optional : '',
                        'nullable': typeof element.params[i].nullable === 'boolean'? element.params[i].nullable : ''
                    });
                }
            }

            graft(thisClass, childNodes, element.longname, element.name);
       }
    });
}

function heading( name, level ) {

    var index   = 0;
    var result  = '';
    var max     = 8;
    
    for(index = 0; index < ( max - level ); ++index ) {
        result += '=';
    }
    
    result += ' ' + name + ' ';

    for(index = 0; index < ( max - level ); ++index ) {
        result += '=';
    }
 
    return result;
}

function dumpParameter( obj ) {

    var optional = ' ';
    
    if ( true === obj.optional ) {
        optional = 'optional';
    }

    console.log( '|' + obj.name + '  |' + obj.type + '  |' + optional + '  |' + obj.description + '  |' );
}

function dumpFunction( obj, depth ) {

    var index = 0;
    
    console.log( heading( obj.name + '()', depth ) );
    console.log( obj.description );
    
    if ( ( 'undefined' !== typeof obj.parameters ) &&
         ( 0 < obj.parameters.length ) ) {
        console.log( '^Name^Type^ ^Description^' );
        for( index = 0; index < obj.parameters.length; ++index ) {
            dumpParameter( obj.parameters[ index ] );
        }
    }
}

function dumpProperty( obj ) {

    console.log( '|' + obj.name + '  |' + obj.type.names[0] + '  |' + obj.description + '  |' );
}

function dumpClass( obj, depth ) {

    var index = 0;
    
    console.log( heading( obj.constructor.name + '()', depth ) );
    console.log( obj.constructor.description );
    
    if ( ( 'undefined' !== typeof obj.constructor.parameters ) &&
         ( 0 < obj.constructor.parameters.length ) ) {
         
        console.log( heading( 'Constructor', depth + 1 ) );
        console.log( '^Name^Type^ ^Description^' );
        for( index = 0; index < obj.constructor.parameters.length; ++index ) {
            dumpParameter( obj.constructor.parameters[ index ] );
        }
    }
    
    if ( ( 'undefined' !== typeof obj.properties ) &&
         ( 0 < obj.properties.length ) ) {
    
        console.log( heading( 'Properties', depth + 1 ) );
        console.log( '^Name^Type^Description^' );
        for( index = 0; index < obj.properties.length; ++index ) {
            dumpProperty( obj.properties[ index ] );
        }
    }
}

function dumpNamespace( obj, depth, prefix ) {

    var index = 0;
    
    console.log( heading( prefix + obj.name, depth ) );
    console.log( obj.description );
        
    if ( 'undefined' !== typeof obj.namespaces ) {    
        for( index = 0; index < obj.namespaces.length; ++index ) {
            dumpNamespace( obj.namespaces[ index ], depth + 1, prefix + obj.name + '.' );
        }
    }
    
    if ( 'undefined' !== typeof obj.classes ) {
        console.log( heading( 'Classes', depth + 1 ) );    
        for( index = 0; index < obj.classes.length; ++index ) {
            dumpClass( obj.classes[ index ], depth + 2 );
        }
    }
    
    if ( 'undefined' !== typeof obj.functions ) {
        console.log( heading( 'Functions', depth + 1 ) );    
        for( index = 0; index < obj.functions.length; ++index ) {
            dumpFunction( obj.functions[ index ], depth + 2 );
        }
    }
}

function dump( obj ) {

    var index = 0;
    
    if ( 'undefined' !== typeof obj.namespaces ) {
        for( index = 0; index < obj.namespaces.length; ++index ) {
            dumpNamespace( obj.namespaces[ index ], 1, '' );
        }
    }

}

/**
    @param {TAFFY} taffyData See <http://taffydb.com/>.
    @param {object} opts
    @param {Tutorial} tutorials
 */
exports.publish = function(taffyData, opts, tutorials) {

    var root = {};
    var docs;

    // Remove undocumented data
    taffyData({undocumented: true}).remove();
    
    // Get array of Doclet objects
    docs = taffyData().get();

    graft(root, docs);

    if (opts.destination === 'console') {
        dump( root );
    }
    else {
        console.log('This template only supports output to the console. Use the option "-d console" when you run JSDoc.');
    }
};
